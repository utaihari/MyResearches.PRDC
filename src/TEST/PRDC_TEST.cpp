/*
 * PRDC_TEST.cpp
 *
 *  Created on: 2016/07/19
 *      Author: uchinosub
 */

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <functional>
#include <algorithm>//shuffle
#include <random>//shuffle
#include <memory>
#include <chrono>//時間計測

#include <math.h>
#include "opencv2/opencv.hpp"
#include "../Dictionary.h"
#include "../util.h"
#include "../PRDC.h"
#include "../TEST.h"

#ifdef PARALLEL
#include <omp.h>
#endif

namespace fs = boost::filesystem;
using namespace std;
using namespace prdc_lzw;
using namespace prdc_util;
using namespace cv;

int PRDC_TEST(string dataset_path, int method_flag, int LOOP,
		int NUMBER_OF_DICS, int NUMBER_OF_TEST_DATA, int k) {

	string filename = "LOG/PRDC_LOG/" + CurrentTimeString() + ".txt";
	std::ofstream ofs(filename);

	vector<int> flags;

	if (method_flag & prdc::PRDC_BASE)
		flags.push_back(prdc::PRDC_BASE);
	if (method_flag & prdc::PAIR_MULTIPLE)
		flags.push_back(prdc::PAIR_MULTIPLE);
	if (method_flag & prdc::PAIR_MULTIPLE_LOG)
		flags.push_back(prdc::PAIR_MULTIPLE_LOG);
	if (method_flag & prdc::RECOMPRESSION)
		flags.push_back(prdc::RECOMPRESSION);
	if (method_flag & prdc::USE_MAHARANOBIS) {
		for (auto& f : flags) {
			f |= prdc::USE_MAHARANOBIS;
		}
	}

	for (int flag : flags) {

		Dictionary dic;
		vector<string> file_paths;
		vector<double> file_classes;
		std::vector<std::string> classes;

		//ファイルの読み込み
		prdc_util::GetEachFilePathsAndClasses(dataset_path, file_paths,
				file_classes, classes);

		if ((int) file_paths.size()
				< NUMBER_OF_DICS + NUMBER_OF_TEST_DATA + 1) {
			cout << "データセットが少なすぎます" << endl;
			cout
					<< "The number of image in dataset is less than (NUMBER_OF_DICS + NUMBER_OF_TEST_DATA).";
			return -1;
		}

		int NUMBER_OF_LEARNING = (int) file_paths.size()
				- (NUMBER_OF_DICS + NUMBER_OF_TEST_DATA);

		vector<double> accuracy_rate;

		auto start = std::chrono::system_clock::now(); // 計測開始時間
		for (int p = 0; p < LOOP; p++) {

			//学習用テキストなどをランダムに選ぶためのランダムな数値が入った配列
			vector<int> random_num(file_paths.size());
			std::random_device rnd; //  乱数生成器
			std::mt19937 mt(rnd()); // メルセンヌツイスター 乱数生成器
			for (int i = 0; i < (int) random_num.size(); ++i) {
				random_num.at(i) = i;
			}
			shuffle(random_num.begin(), random_num.end(), mt);

			//学習用テキストの設定
			vector<string> learning_contents;
			vector<double> learning_class;
			for (int i = 0; i < NUMBER_OF_LEARNING; ++i) {
				learning_contents.push_back(file_paths.at(random_num.at(i)));
				learning_class.push_back(file_classes.at(random_num.at(i)));
			}

			//テスト用テキストの設定
			vector<string> test_contents;
			vector<double> test_class;
			for (int i = NUMBER_OF_LEARNING;
					i < (int) file_paths.size() - NUMBER_OF_DICS; ++i) {
				test_contents.push_back(file_paths.at(random_num.at(i)));
				test_class.push_back(file_classes.at(random_num.at(i)));
			}

			//基底辞書用テキストの設定
			vector<string> base_dics;
			for (int i = NUMBER_OF_LEARNING + NUMBER_OF_TEST_DATA;
					i < (int) file_paths.size(); ++i) {
				base_dics.push_back(file_paths.at(random_num.at(i)));
			}

//PRDCここから

//基底辞書の設定
			prdc::PRDC pr(base_dics, flag);

//prdc::PRDC pr("test.xml");

			cout << "基底辞書作成完了" << endl;
			pr.train(learning_contents, learning_class);
			cout << "学習完了" << endl;

			pr.save("test.xml");

			int accuracy_count = 0;

#ifdef PARALLEL
#pragma omp parallel for
#endif
			for (int i = 0; i < (int) test_contents.size(); ++i) {

				double r = pr.find_nearest(test_contents.at(i), k);

				if (r == test_class.at(i)) {
					accuracy_count++;
				}
			}

			accuracy_rate.push_back(
					(double) accuracy_count / (double) test_contents.size());
			cout << p + 1 << "回目　正解率：" << accuracy_rate.at(p) << endl;

		}
		cout << "~~~~~~~~~~~~~~~~~~~~終了~~~~~~~~~~~~~~~~~~~" << endl;
		double accuracy_rate_sum = 0.0;

		switch (flag) {
		case prdc::PRDC_BASE:
			cout << "PRDC_BASE" << endl;
			ofs << "PRDC_BASE" << endl;
			break;
		case prdc::PAIR_MULTIPLE:
			cout << "PAIR_MULTIPLE" << endl;
			ofs << "PAIR_MULTIPLE" << endl;
			break;
		case prdc::PAIR_MULTIPLE_LOG:
			cout << "PAIR_MULTIPLE_LOG" << endl;
			ofs << "PAIR_MULTIPLE_LOG" << endl;
			break;
		case prdc::RECOMPRESSION:
			cout << "RECOMPRESSION" << endl;
			ofs << "RECOMPRESSION" << endl;
			break;
		default:
			break;
		}
		cout << "k: " << k << ", loop: " << LOOP << endl;
		cout << "Number of dics: " << NUMBER_OF_DICS << endl;
		cout << "Number of test data: " << NUMBER_OF_TEST_DATA << endl;
		cout << "Number of learning data: " << NUMBER_OF_LEARNING << endl;

		ofs << "k: " << k << "loop: " << LOOP << endl;
		ofs << "Number of dics: " << NUMBER_OF_DICS << endl;
		ofs << "Number of test data: " << NUMBER_OF_TEST_DATA << endl;
		ofs << "Number of learning data: " << NUMBER_OF_LEARNING << endl;

		for (int i = 0; i < (int) accuracy_rate.size(); ++i) {
			accuracy_rate_sum += accuracy_rate.at(i);
			cout << i + 1 << "回目　正解率：" << accuracy_rate.at(i) << endl;
			ofs << i + 1 << "回目　正解率：" << accuracy_rate.at(i) << endl;
		}
		cout << "平均正解率:"
				<< (double) accuracy_rate_sum / (double) accuracy_rate.size()
				<< endl;
		ofs << "平均正解率:"
				<< (double) accuracy_rate_sum / (double) accuracy_rate.size()
				<< endl;

		auto end = std::chrono::system_clock::now();  // 計測終了時間
		auto elapsed = end - start;
		cout << "処理時間:"
				<< chrono::duration_cast<chrono::seconds>(elapsed).count()
				<< "秒" << endl;

		ofs << "処理時間:"
				<< chrono::duration_cast<chrono::seconds>(elapsed).count()
				<< "秒" << endl;
		ofs << endl;

	}

	ofs.close();
	return 0;
}

int PRDC_TEST(std::string train_data_path, std::string test_data_path,
		std::string base_dics_path, int method_flag, int k) {
	string filename = "LOG/PRDC_LOG/" + CurrentTimeString() + ".txt";
	string table_name = "LOG/PRDC_LOG/" + CurrentTimeString() + "-table.txt";
	std::ofstream ofs(filename);
	ofstream table_ofs(table_name);

	vector<int> flags;
	vector<string> flags_name;

	for (int i = 0; i < (int) prdc::METHOD_ARRAY.size(); ++i) {
		if (method_flag & prdc::METHOD_ARRAY.at(i)) {
			flags.push_back(prdc::METHOD_ARRAY.at(i));
			flags_name.push_back(prdc::METHOD_NAME_ARRAY.at(i));
		}
	}

	vector<string> train_data;
	vector<string> test_data;
	vector<string> base_dics_data;

	vector<double> train_classes;
	vector<double> test_classes;
	vector<double> base_dics_classes;

	std::vector<std::string> classes;

	vector<double> accuracys(flags.size());
	vector<map<double, double>> accuracys_for_each_classes(flags.size());
	map<double, double> accuracy_base_for_each_classes;

	//ファイルの読み込み
	GetEachFilePathsAndClasses(train_data_path, train_data, train_classes,
			classes);
	GetEachFilePathsAndClasses(test_data_path, test_data, test_classes,
			classes);
	GetEachFilePathsAndClasses(base_dics_path, base_dics_data,
			base_dics_classes, classes);

	cout << "読み込んだクラス一覧" << endl;
	for (auto c : classes) {
		cout << c << endl;
	}

	vector<vector<vector<int>>> classified_table(flags.size());
	for (auto& d : classified_table) {
		d = vector<vector<int>>(classes.size());
		for (auto& c : d) {
			c = vector<int>(classes.size(), 0);
		}
	}
	vector<vector<int>> classified_table_base(classes.size());
	for (auto& c : classified_table_base) {
		c = vector<int>(classes.size(), 0);
	}

	auto start = std::chrono::system_clock::now(); // 計測開始時間
	map<double, int> each_class_size;

	//PRDCここから

	//基底辞書の設定
	prdc::PRDC pr(base_dics_data, flags.at(0), method_flag, false);
	pr.train(train_data, train_classes);

	cout << "学習完了" << endl;
	cout << "テスト開始（多少時間がかかります）" << endl;

	int accuracy_count = 0;
	map<double, int> accuracy_count_for_each_classes;

#ifdef PARALLEL
#pragma omp parallel for
#endif
	for (int i = 0; i < (int) test_data.size(); ++i) {
		double r = pr.find_nearest(test_data.at(i), k);
		classified_table[0][(int) test_classes.at(i)][(int) r]++;
		each_class_size[test_classes.at(i)]++;
		if (r == test_classes.at(i)) {
			accuracy_count++;
			accuracy_count_for_each_classes[test_classes.at(i)]++;
		}
	}

	accuracys.at(0) = (double) accuracy_count / (double) test_data.size();

	for (int i = 0; i < (int) classes.size(); ++i) {
		double num = (double) i;
		if (each_class_size[num] == 0)
			continue;
		accuracys_for_each_classes.at(0)[num] =
				(double) accuracy_count_for_each_classes[num]
						/ (double) each_class_size[num];
	}
	cout << "手法" << flags_name.at(0) << " 正解率：" << accuracys.at(0) << endl;

	//各種手法比較
	for (int n = 1; n < (int) flags.size(); ++n) {

		prdc::PRDC pr_plus(base_dics_data, flags.at(n), 0, true);
		pr_plus.train(train_data, train_classes);

		cout << "学習完了" << endl;
		cout << "テスト開始（多少時間がかかります）" << endl;

		int accuracy_count = 0;
		map<double, int> accuracy_count_for_each_classes;

#ifdef PARALLEL
#pragma omp parallel for
#endif
		for (int i = 0; i < (int) test_data.size(); ++i) {
			double r = pr_plus.find_nearest(test_data.at(i), k);
			classified_table[n][(int) test_classes.at(i)][(int) r]++;
			if (r == test_classes.at(i)) {
				accuracy_count++;
				accuracy_count_for_each_classes[test_classes.at(i)]++;
			}
		}

		accuracys.at(n) = (double) accuracy_count / (double) test_data.size();

		for (int i = 0; i < (int) classes.size(); ++i) {
			double num = (double) i;
			if (each_class_size[num] == 0)
				continue;
			accuracys_for_each_classes.at(n)[num] =
					(double) accuracy_count_for_each_classes[num]
							/ (double) each_class_size[num];
		}
		cout << "手法" << flags_name.at(n) << " 正解率：" << accuracys.at(n) << endl;

	}
	cout << "~~~~~~~~~~~~~~~~~~~~終了~~~~~~~~~~~~~~~~~~~" << endl;
	vector<double> accuracys_rate_sum(flags.size()); //精度が何％向上したか
	vector<map<double, double>> accuracys_rate_sum_each_classes(flags.size()); //それぞれのクラスで精度が何％向上したか

	map<double, double> accuracy_base_sum_each_classes;

	for (auto& ars : accuracys_rate_sum) {
		ars = 0.0;
	}

	cout << "Number of dics: " << base_dics_data.size() << endl;
	cout << "Number of test data: " << test_data.size() << endl;
	cout << "Number of train data: " << train_data.size() << endl;

	ofs << "Test Dataset: " << test_data_path << endl;
	ofs << "Train Dataset: " << train_data_path << endl;
	ofs << "BaseDic Dataset: " << base_dics_path << endl;

	ofs << "Number of classes: " << classes.size() << endl;
	ofs << "Number of dics: " << base_dics_data.size() << endl;
	ofs << "Number of test data: " << test_data.size() << endl;
	ofs << "Number of learning data: " << train_data.size() << endl << endl;

	ofs << "CLASS LIST" << endl;
	for (int i = 0; i < (int) classes.size(); ++i) {
		double num = (double) i;
		ofs << classes.at(i) << " >> " << num << endl;
	}
	ofs << endl;
	cout << "~~~~~~~~~~~~~~~~~~~~正解率~~~~~~~~~~~~~~~~~~" << endl;
	cout << "| ~回目 |";
	for (auto name : flags_name) {
		cout << " " << name << " |";
	}
	cout << endl;

	ofs << "~~~~~~~~~~~~~~~~~~~~正解率~~~~~~~~~~~~~~~~~~" << endl;
	ofs << "| ~回目 | ";
	for (auto name : flags_name) {
		ofs << " " << name << " |";
	}
	ofs << endl;

	cout << "| " << 1 << "回目 | ";
	ofs << "| " << 1 << "回目 | ";

	for (int n = 0; n < (int) flags.size(); ++n) {
		cout << accuracys.at(n) << " | ";
		ofs << accuracys.at(n) << " | ";
		accuracys_rate_sum.at(n) += accuracys.at(n);
		for (int m = 0; m < (int) classes.size(); ++m) {
			double num = (double) m;
			accuracys_rate_sum_each_classes.at(n)[num] +=
					accuracys_for_each_classes.at(n)[num];
		}

	}
	cout << endl;
	ofs << endl;

	ofs << "\nクラス毎の正解率" << endl;

	for (int n = 0; n < (int) flags.size(); ++n) {
		ofs << flags_name.at(n) << endl;
		for (int i = 0; i < (int) classes.size(); ++i) {
			double num = (double) i;
			ofs << classes.at(i) << " : "
					<< accuracys_rate_sum_each_classes.at(n)[num] << endl;
		}
		ofs << endl;
	}

//	if (method_flag == prdc::RECOMPRESSION) {
//		auto c_var = prdc_util::mean_variance(prdc::PRDC::compress_rate_array);
//		auto rc_var = prdc_util::mean_variance(
//				prdc::PRDC::recompress_rate_array);
//
//		cout << "圧縮率の平均" << c_var.first << endl;
//		cout << "圧縮率の分散" << c_var.second << endl;
//		cout << "圧縮率の標準偏差" << std::sqrt(c_var.second) << endl;
//		cout << "再圧縮率の平均" << rc_var.first << endl;
//		cout << "再圧縮率の分散" << rc_var.second << endl;
//		cout << "再圧縮率の標準偏差" << std::sqrt(rc_var.second) << endl;
//
//		ofs << "圧縮率の平均" << c_var.first << endl;
//		ofs << "圧縮率の分散" << c_var.second << endl;
//		ofs << "圧縮率の標準偏差" << std::sqrt(c_var.second) << endl;
//		ofs << "再圧縮率の平均" << rc_var.first << endl;
//		ofs << "再圧縮率の分散" << rc_var.second << endl;
//		ofs << "再圧縮率の標準偏差" << std::sqrt(rc_var.second) << endl;
//	}

	auto end = std::chrono::system_clock::now();  // 計測終了時間
	auto elapsed = end - start;
	cout << "処理時間:" << chrono::duration_cast<chrono::seconds>(elapsed).count()
			<< "秒" << endl;

	ofs << "\n処理時間:" << chrono::duration_cast<chrono::seconds>(elapsed).count()
			<< "秒" << endl;
	ofs << endl;

	ofs.close();

	//不正解ランキング出力処理　ここから
	const int DISPLAY_SIZE = 30;

	for (int i = 0; i < (int) flags.size(); ++i) {
		table_ofs << flags_name.at(i) << endl;
		table_ofs << endl;

		std::vector<std::pair<int, std::string> > sorted_table;
		sorted_table = MakeMistakeRanking(classified_table[i], classes);
		for (int i = 0; i < DISPLAY_SIZE; ++i) {
			if (i >= (int) sorted_table.size()) {
				break;
			}
			table_ofs << sorted_table.at(i).second << " :"
					<< sorted_table.at(i).first << endl;
		}
		table_ofs << "\n" << endl;
	}
	//不正解ランキング出力処理　ここまで
	return 0;

}

int PRDC_TEST(std::string train_data_path, std::string test_data_path,
		int number_of_dics, int method_flag, int k) {
	string filename = "LOG/PRDC_LOG/" + CurrentTimeString() + ".txt";
	string table_name = "LOG/PRDC_LOG/" + CurrentTimeString() + "-table.txt";
	std::ofstream ofs(filename);
	ofstream table_ofs(table_name);

	vector<int> flags;
	vector<string> flags_name;

	for (int i = 0; i < (int) prdc::METHOD_ARRAY.size(); ++i) {
		if (method_flag & prdc::METHOD_ARRAY.at(i)) {
			flags.push_back(prdc::METHOD_ARRAY.at(i));
			flags_name.push_back(prdc::METHOD_NAME_ARRAY.at(i));
		}
	}

	vector<string> train_data;
	vector<string> test_data;
	vector<string> base_dics;

	vector<double> train_classes;
	vector<double> test_classes;

	std::vector<std::string> classes;

	vector<double> accuracys(flags.size());
	vector<map<double, double>> accuracys_for_each_classes(flags.size());
	map<double, double> accuracy_base_for_each_classes;

	//ファイルの読み込み
	GetEachFilePathsAndClasses(train_data_path, train_data, train_classes,
			classes);
	GetEachFilePathsAndClasses(test_data_path, test_data, test_classes,
			classes);

	cout << "読み込んだクラス一覧" << endl;
	for (auto c : classes) {
		cout << c << endl;
	}
	std::random_device rnd;     // 非決定的な乱数生成器を生成
	std::mt19937 mt(rnd());     //  メルセンヌ・ツイスタの32ビット版、引数は初期シード値
	std::uniform_int_distribution<> rand(0, train_data.size() - 1);
	//基底辞書の選択
	for (int i = 0; i < (int) train_classes.size(); ++i) {
		for (int p = 0; p < number_of_dics; ++p) {
			int random_num = rand(mt);
			base_dics.push_back(train_data.at(random_num));
			prdc_util::remove(train_classes, random_num);
		}
	}

	vector<vector<vector<int>>> classified_table(flags.size());
	for (auto& d : classified_table) {
		d = vector<vector<int>>(classes.size());
		for (auto& c : d) {
			c = vector<int>(classes.size(), 0);
		}
	}
	vector<vector<int>> classified_table_base(classes.size());
	for (auto& c : classified_table_base) {
		c = vector<int>(classes.size(), 0);
	}

	auto start = std::chrono::system_clock::now(); // 計測開始時間
	map<double, int> each_class_size;

	//PRDCここから

	//基底辞書の設定
	prdc::PRDC pr(base_dics, flags.at(0), method_flag, false);
	pr.train(train_data, train_classes);

	cout << "学習完了" << endl;
	cout << "テスト開始（多少時間がかかります）" << endl;

	int accuracy_count = 0;
	map<double, int> accuracy_count_for_each_classes;

#ifdef PARALLEL
#pragma omp parallel for
#endif
	for (int i = 0; i < (int) test_data.size(); ++i) {
		double r = pr.find_nearest(test_data.at(i), k);
		classified_table[0][(int) test_classes.at(i)][(int) r]++;
		each_class_size[test_classes.at(i)]++;
		if (r == test_classes.at(i)) {
			accuracy_count++;
			accuracy_count_for_each_classes[test_classes.at(i)]++;
		}
	}

	accuracys.at(0) = (double) accuracy_count / (double) test_data.size();

	for (int i = 0; i < (int) classes.size(); ++i) {
		double num = (double) i;
		if (each_class_size[num] == 0)
			continue;
		accuracys_for_each_classes.at(0)[num] =
				(double) accuracy_count_for_each_classes[num]
						/ (double) each_class_size[num];
	}
	cout << "手法" << flags_name.at(0) << " 正解率：" << accuracys.at(0) << endl;

	//各種手法比較
	for (int n = 1; n < (int) flags.size(); ++n) {

		prdc::PRDC pr_plus(base_dics, flags.at(n), 0, true);
		pr_plus.train(train_data, train_classes);

		cout << "学習完了" << endl;
		cout << "テスト開始（多少時間がかかります）" << endl;

		int accuracy_count = 0;
		map<double, int> accuracy_count_for_each_classes;

#ifdef PARALLEL
#pragma omp parallel for
#endif
		for (int i = 0; i < (int) test_data.size(); ++i) {
			double r = pr_plus.find_nearest(test_data.at(i), k);
			classified_table[n][(int) test_classes.at(i)][(int) r]++;
			if (r == test_classes.at(i)) {
				accuracy_count++;
				accuracy_count_for_each_classes[test_classes.at(i)]++;
			}
		}

		accuracys.at(n) = (double) accuracy_count / (double) test_data.size();

		for (int i = 0; i < (int) classes.size(); ++i) {
			double num = (double) i;
			if (each_class_size[num] == 0)
				continue;
			accuracys_for_each_classes.at(n)[num] =
					(double) accuracy_count_for_each_classes[num]
							/ (double) each_class_size[num];
		}
		cout << "手法" << flags_name.at(n) << " 正解率：" << accuracys.at(n) << endl;

	}
	cout << "~~~~~~~~~~~~~~~~~~~~終了~~~~~~~~~~~~~~~~~~~" << endl;
	vector<double> accuracys_rate_sum(flags.size()); //精度が何％向上したか
	vector<map<double, double>> accuracys_rate_sum_each_classes(flags.size()); //それぞれのクラスで精度が何％向上したか

	map<double, double> accuracy_base_sum_each_classes;

	for (auto& ars : accuracys_rate_sum) {
		ars = 0.0;
	}

	cout << "Number of test data: " << test_data.size() << endl;
	cout << "Number of train data: " << train_data.size() << endl;

	ofs << "Test Dataset: " << test_data_path << endl;
	ofs << "Train Dataset: " << train_data_path << endl;

	ofs << "Number of classes: " << classes.size() << endl;
	ofs << "Number of test data: " << test_data.size() << endl;
	ofs << "Number of learning data: " << train_data.size() << endl << endl;

	ofs << "CLASS LIST" << endl;
	for (int i = 0; i < (int) classes.size(); ++i) {
		double num = (double) i;
		ofs << classes.at(i) << " >> " << num << endl;
	}
	ofs << endl;
	cout << "~~~~~~~~~~~~~~~~~~~~正解率~~~~~~~~~~~~~~~~~~" << endl;
	cout << "| ~回目 |";
	for (auto name : flags_name) {
		cout << " " << name << " |";
	}
	cout << endl;

	ofs << "~~~~~~~~~~~~~~~~~~~~正解率~~~~~~~~~~~~~~~~~~" << endl;
	ofs << "| ~回目 | ";
	for (auto name : flags_name) {
		ofs << " " << name << " |";
	}
	ofs << endl;

	cout << "| " << 1 << "回目 | ";
	ofs << "| " << 1 << "回目 | ";

	for (int n = 0; n < (int) flags.size(); ++n) {
		cout << accuracys.at(n) << " | ";
		ofs << accuracys.at(n) << " | ";
		accuracys_rate_sum.at(n) += accuracys.at(n);
		for (int m = 0; m < (int) classes.size(); ++m) {
			double num = (double) m;
			accuracys_rate_sum_each_classes.at(n)[num] +=
					accuracys_for_each_classes.at(n)[num];
		}

	}
	cout << endl;
	ofs << endl;

	ofs << "\nクラス毎の正解率" << endl;

	for (int n = 0; n < (int) flags.size(); ++n) {
		ofs << flags_name.at(n) << endl;
		for (int i = 0; i < (int) classes.size(); ++i) {
			double num = (double) i;
			ofs << classes.at(i) << " : "
					<< accuracys_rate_sum_each_classes.at(n)[num] << endl;
		}
		ofs << endl;
	}

	auto end = std::chrono::system_clock::now();  // 計測終了時間
	auto elapsed = end - start;
	cout << "処理時間:" << chrono::duration_cast<chrono::seconds>(elapsed).count()
			<< "秒" << endl;

	ofs << "\n処理時間:" << chrono::duration_cast<chrono::seconds>(elapsed).count()
			<< "秒" << endl;
	ofs << endl;

	ofs.close();

	//不正解ランキング出力処理　ここから
	const int DISPLAY_SIZE = 30;

	for (int i = 0; i < (int) flags.size(); ++i) {
		table_ofs << flags_name.at(i) << endl;
		table_ofs << endl;

		std::vector<std::pair<int, std::string> > sorted_table;
		sorted_table = MakeMistakeRanking(classified_table[i], classes);
		for (int i = 0; i < DISPLAY_SIZE; ++i) {
			if (i >= (int) sorted_table.size()) {
				break;
			}
			table_ofs << sorted_table.at(i).second << " :"
					<< sorted_table.at(i).first << endl;
		}
		table_ofs << "\n" << endl;
	}
	//不正解ランキング出力処理　ここまで
	return 0;

}

int PRDC_PRECISION(string dataset_path, vector<int> flags, int LOOP,
		int NUMBER_OF_DICS, int NUMBER_OF_TRAIN, int NUMBER_OF_OUTPUT) {

	string filename = "LOG/PRDC_LOG/precision-" + CurrentTimeString() + ".txt";
	std::ofstream ofs(filename);

	vector<string> flags_name(flags.size());

	for (int i = 0; i < (int) flags.size(); ++i) {
		flags_name.at(i) = "";
		for (int p = 0; p < (int) prdc::METHOD_ARRAY.size(); ++p) {
			if (flags.at(i) & prdc::METHOD_ARRAY.at(p)) {
				;
				flags_name.at(i) += prdc::METHOD_NAME_ARRAY.at(p);
			}
		}
		if (flags.at(i) & prdc::USE_MAHARANOBIS) {
			flags_name.at(i) += " -USE_MAHARANOBIS";
		}
		if (flags.at(i) & prdc::NORMALIZE) {
			flags_name.at(i) += " -NORMALIZE";
		}
		if (flags.at(i) & prdc::M_EDIT) {
			flags_name.at(i) += " -EDIT";
		}
	}

	vector<vector<double>> accuracys(flags.size());
	vector<vector<map<double, double>>> accuracys_for_each_classes(flags.size());
	vector<double> accuracy_base(LOOP);
	vector<map<double, double>> accuracy_base_for_each_classes(LOOP);

	for (int i = 0; i < (int) flags.size(); ++i) {
		accuracys.at(i).resize(LOOP);
		accuracys_for_each_classes.at(i).resize(LOOP);
	}

	//ファイルの読み込み
	prdc_util::IncludedFilePaths files(dataset_path, { ".txt" });

	cout << "読み込んだクラス一覧" << endl;
	for (auto c : files.classes) {
		cout << c << endl;
	}

	if ((int) files.included_file_paths.size()
			< NUMBER_OF_DICS + NUMBER_OF_TRAIN + 1) {
		cout << "データセットが少なすぎます" << endl;
		cout
				<< "The number of datasets is less than (NUMBER_OF_DICS + NUMBER_OF_TEST_DATA).";
		return -1;
	}

	std::random_device rnd; //  乱数生成器
	std::mt19937 mt(rnd()); // メルセンヌツイスター 乱数生成器

//	vector<vector<vector<int>>> classified_table(flags.size());
//	for (auto& c : classified_table) {
//		c = vector<vector<int>>(files.classes.size());
//		for (auto&d : c) {
//			d = vector<int>(files.classes.size(), 0);
//		}
//	}
	map<double, vector<double>> all_of_accuracy_rate;
	auto start = std::chrono::system_clock::now(); // 計測開始時間
	for (int p = 0; p < LOOP; p++) {
		map<double, int> each_class_size;

		//ファイルの読み込み
		prdc_util::IncludedFilePaths files_in_loop(dataset_path, { ".txt" });

		//基底辞書用テキストの設定
		vector<string> base_dics;
		for (int i = 0; i < (int) files_in_loop.classes.size(); ++i) {

			if ((int) files.each_class_files[i].size() < NUMBER_OF_DICS) {
				cout << "ERROR: NUMBER_OF_DICS の数が各クラスのデータ数を上回っています" << endl;
				return -1;
			}
			vector<string>& random_files = files_in_loop.each_class_files[i];
			shuffle(random_files.begin(), random_files.end(), mt);

			for (int p = 0; p < NUMBER_OF_DICS; ++p) {
				base_dics.push_back(string(random_files.at(0)));
				prdc_util::remove(random_files, 0);
			}
		}

		//学習用テキストの設定
		vector<string> train_contents;
		vector<double> train_class;
		for (int i = 0; i < (int) files_in_loop.classes.size(); ++i) {

			if ((int) files.each_class_files[i].size() < NUMBER_OF_TRAIN) {
				cout << "ERROR: NUMBER_OF_TRAIN の数が各クラスのデータ数を上回っています" << endl;
				return -1;
			}
			vector<string>& random_files = files_in_loop.each_class_files[i];
			shuffle(random_files.begin(), random_files.end(), mt);

			for (int p = 0; p < NUMBER_OF_TRAIN; ++p) {
				train_contents.push_back(string(random_files.at(0)));
				train_class.push_back(i);
				prdc_util::remove(random_files, 0);
			}
		}

		//テスト用テキストの設定
		vector<string> test_contents;
		vector<double> test_class;
		for (int i = 0; i < (int) files_in_loop.classes.size(); ++i) {
			for (auto d : files_in_loop.each_class_files[i]) {
				test_contents.push_back(string(d));
				test_class.push_back(i);
			}
		}

		prdc::PRDC::crear_last_data();
		//PRDCここから
		for (int flag : flags) {
			//基底辞書の設定
			prdc::PRDC pr(base_dics, flag, true);

			//prdc::PRDC pr("test.xml");

			cout << "基底辞書作成完了" << endl;
			pr.train(train_contents, train_class);
//			pr.make_invert_matrix();
			cout << "学習完了" << endl;

			double accuracy_rate = 0;
			int debug = 1;

#ifdef PARALLEL
#pragma omp parallel for
#endif
			for (int i = 0; i < (int) test_contents.size(); ++i) {
				std::vector<int> response;

				if (flag & prdc::USE_MAHARANOBIS) {
					response = pr.find_near_vector_mahalanobis(
							test_contents.at(i), NUMBER_OF_OUTPUT);
				} else {
					response = pr.find_near_vector(test_contents.at(i),
							NUMBER_OF_OUTPUT);
				}

				int accuracy_count = 0;

				for (int q = 0; q < NUMBER_OF_OUTPUT; ++q) {
					if (test_class.at(i) == train_class.at(response.at(q))) {
						accuracy_count++;
					}
				}
				double rate = (double) accuracy_count
						/ (double) NUMBER_OF_OUTPUT;
				accuracy_rate += rate;
				cout << test_contents.size() << "個中 " << debug << "個目　正解率: "
						<< rate << endl;
				debug++;
			}

			all_of_accuracy_rate[flag].push_back(
					(double) accuracy_rate / (double) test_contents.size());
			cout << p + 1 << "回目　平均適合率：" << all_of_accuracy_rate[flag].at(p)
					<< endl;

		}
	}
	cout << "~~~~~~~~~~~~~~~~~~~~終了~~~~~~~~~~~~~~~~~~~" << endl;

	for (int flag : flags) {
		double accuracy_rate_sum = 0.0;

		for (int i = 0; i < (int) prdc::METHOD_ARRAY.size(); ++i) {
			if (flag == prdc::METHOD_ARRAY.at(i)) {
				cout << prdc::METHOD_NAME_ARRAY.at(i) << endl;
				ofs << prdc::METHOD_NAME_ARRAY.at(i) << endl;
			}
		}

		cout << "NUMBER_OF_OUTPUT: " << NUMBER_OF_OUTPUT << ", loop: " << LOOP
				<< endl;
		cout << "Number of dics: " << NUMBER_OF_DICS << endl;
		cout << "Number of learning data: " << NUMBER_OF_TRAIN << endl;

		ofs << "NUMBER_OF_OUTPUT: " << NUMBER_OF_OUTPUT << "loop: " << LOOP
				<< endl;
		ofs << "Number of dics: " << NUMBER_OF_DICS << endl;
		ofs << "Number of learning data: " << NUMBER_OF_TRAIN << endl;

		for (int i = 0; i < (int) all_of_accuracy_rate[flag].size(); ++i) {
			accuracy_rate_sum += all_of_accuracy_rate[flag].at(i);
			cout << i + 1 << "回目　平均適合率：" << all_of_accuracy_rate[flag].at(i)
					<< endl;
			ofs << i + 1 << "回目　平均適合率：" << all_of_accuracy_rate[flag].at(i)
					<< endl;
		}
		cout << "平均正解率:"
				<< (double) accuracy_rate_sum
						/ (double) all_of_accuracy_rate[flag].size() << endl;
		ofs << "平均正解率:"
				<< (double) accuracy_rate_sum
						/ (double) all_of_accuracy_rate[flag].size() << endl;

		auto end = std::chrono::system_clock::now();  // 計測終了時間
		auto elapsed = end - start;
		cout << "処理時間:"
				<< chrono::duration_cast<chrono::seconds>(elapsed).count()
				<< "秒" << endl;

		ofs << "処理時間:"
				<< chrono::duration_cast<chrono::seconds>(elapsed).count()
				<< "秒" << endl;
		ofs << endl;

	}

	ofs.close();
	return 0;
}

