/*
 * PRDC_SAMEDICS_TEST.cpp
 *
 *  Created on: 2016/07/20
 *      Author: uchinosub
 */

#include "../TEST.h"

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
#include <omp.h>
#include "opencv2/opencv.hpp"
#include "../Dictionary.h"
#include "../util.h"
#include "../PRDC.h"
#include "../Debug.h"

namespace fs = boost::filesystem;
using namespace std;
using namespace prdc_lzw;
using namespace prdc_util;
using namespace cv;

/**
 * @brief PRDCとの比較を行います
 * @param dataset_path
 * @param method_flag
 * @param LOOP
 * @param NUMBER_OF_DICS
 * @param NUMBER_OF_TEST_DATA
 * @param k
 * @return
 */
int PRDC_SAMEDICS_TEST(string dataset_path, vector<int> flags, int LOOP,
		int NUMBER_OF_DICS, int NUMBER_OF_TRAIN, int k, bool multibyte_char) {

	string filename = "LOG/PRDC_SAMEDICS_LOG/" + CurrentTimeString() + ".txt";
	string table_name = "LOG/PRDC_SAMEDICS_LOG/" + CurrentTimeString()
			+ "-table.txt";
	std::ofstream ofs(filename);
	ofstream table_ofs(table_name);

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

//PRDCここから

		prdc::PRDC::crear_last_data();
		//各種手法比較
		for (int n = 0; n < (int) flags.size(); ++n) {
			vector<string> dics(base_dics);
			prdc::PRDC::crear_last_data();
			prdc::PRDC pr_plus(dics, flags.at(n), true, multibyte_char);
			pr_plus.train(train_contents, train_class);

			int accuracy_count = 0;
			map<float, int> accuracy_count_for_each_classes;
			map<float, int> class_size;

			for (int i = 0; i < (int) files_in_loop.classes.size(); ++i) {
				class_size[i] = 0;
			}

#ifdef PARALLEL
#pragma omp parallel for
#endif
			for (int i = 0; i < (int) test_contents.size(); ++i) {
				class_size[test_class.at(i)]++;

				float r = pr_plus.find_nearest(test_contents.at(i), k);

				if (r == test_class.at(i)) {
					accuracy_count++;
					accuracy_count_for_each_classes[test_class.at(i)]++;
				}
			}

			accuracys.at(n).at(p) = (double) accuracy_count
					/ (double) test_contents.size();

			for (int i = 0; i < (int) files_in_loop.classes.size(); ++i) {
				float num = (float) i;
				if (class_size[num] == 0)
					continue;
				accuracys_for_each_classes.at(n).at(p)[num] =
						(double) accuracy_count_for_each_classes[num]
								/ (double) class_size[num];
			}
			cout << p + 1 << "回目　手法" << flags_name.at(n) << " 正解率："
					<< accuracys.at(n).at(p) << endl;
			cout << "number_of_dics " << dics.size() << endl;

		}
	}
	cout << "~~~~~~~~~~~~~~~~~~~~終了~~~~~~~~~~~~~~~~~~~" << endl;
	vector<double> accuracys_rate_sum(flags.size()); //精度が何％向上したか
	vector<map<float, double>> accuracys_rate_sum_each_classes(flags.size()); //それぞれのクラスで精度が何％向上したか

//	double accuracy_base_sum = 0.0; //PRDC(先行研究)の平均正解率
	map<float, double> accuracy_base_sum_each_classes;

	for (auto& ars : accuracys_rate_sum) {
		ars = 0.0;
	}

	cout << "k: " << k << ", loop: " << LOOP << endl;
	cout << "Number of dics: " << NUMBER_OF_DICS << endl;
	cout << "Number of learning data: " << NUMBER_OF_TRAIN << endl;

	ofs << "Dataset: " << dataset_path << endl;
	ofs << "k: " << k << ", loop: " << LOOP << endl;
	ofs << "Number of classes: " << files.classes.size() << endl;
	ofs << "Number of dics: " << NUMBER_OF_DICS << endl;
	ofs << "Number of learning data: " << NUMBER_OF_TRAIN << "\n" << endl;

	ofs << "CLASS LIST" << endl;
	for (int i = 0; i < (int) files.classes.size(); ++i) {
		float num = (float) i;
		ofs << files.classes.at(i) << " >> " << num << endl;
	}
	ofs << endl;
	cout << "~~~~~~~~~~~~~~~~~~~~正解率~~~~~~~~~~~~~~~~~~" << endl;
	cout << "| ~回目 | ";
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

	for (int i = 0; i < LOOP; ++i) {
		cout << "| " << i + 1 << "回目 | ";
		ofs << "| " << i + 1 << "回目 | ";

		for (int n = 0; n < (int) flags.size(); ++n) {
			cout << accuracys.at(n).at(i) << " | ";
			ofs << accuracys.at(n).at(i) << " | ";
			accuracys_rate_sum.at(n) += accuracys.at(n).at(i);
			for (int m = 0; m < (int) files.classes.size(); ++m) {
				float num = (float) m;
				accuracys_rate_sum_each_classes.at(n)[num] +=
						accuracys_for_each_classes.at(n).at(i)[num];
			}

		}
		cout << endl;
		ofs << endl;
	}

	for (int n = 0; n < (int) flags.size(); ++n) {
		cout << flags_name.at(n) << " 平均正解率: "
				<< (double) accuracys_rate_sum.at(n) / (double) LOOP << endl;
		ofs << flags_name.at(n) << " 平均正解率: "
				<< (double) accuracys_rate_sum.at(n) / (double) LOOP << endl;
	}

	ofs << "\nクラス毎の平均正解率" << endl;

	for (int n = 0; n < (int) flags.size(); ++n) {
		ofs << flags_name.at(n) << endl;
		map<string, double> sorted;
		for (int i = 0; i < (int) files.classes.size(); ++i) {
			float num = (float) i;
			sorted[files.classes.at(i)] =
					accuracys_rate_sum_each_classes.at(n)[num] / (double) LOOP;
		}
		for (auto m : sorted) {
			ofs << m.first << " : " << m.second << endl;
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

	return 0;
}

int PRDC_SAMEDICS_TEST(string train_data_path, string test_data_path,
		vector<int> flags, int LOOP, string savefile_name, int NUMBER_OF_DICS,
		int k, bool multibyte_char) {

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

	string flagnames = "";
	for (auto f : flags_name) {
		flagnames += f + "-";
	}

	string filename = "LOG/PRDC_SAMEDICS_LOG/" + savefile_name + "_" + flagnames
			+ "_" + CurrentTimeString() + ".txt";
	std::ofstream ofs(filename);
	string  TF_vector = "LOG/PRDC_SAMEDICS_LOG/" + savefile_name + "_" + flagnames
				+ "_" + CurrentTimeString() + ".csv";
		std::ofstream TF_csv(TF_vector);

	vector<vector<double>> accuracys(flags.size());
	vector<vector<map<double, double>>> accuracys_for_each_classes(flags.size());

	for (int i = 0; i < (int) flags.size(); ++i) {
		accuracys.at(i).resize(LOOP);
		accuracys_for_each_classes.at(i).resize(LOOP);
	}

	//ファイルの読み込み
	prdc_util::IncludedFilePaths train_files(train_data_path, { ".txt" });
	prdc_util::IncludedFilePaths test_files(test_data_path, { ".txt" });

#ifdef DEBUG_OUTPUT
	cout << "読み込んだクラス一覧" << endl;
	for (auto c : train_files.classes) {
		cout << c << endl;
	}
#endif

	if ((int) train_files.included_file_paths.size() < (NUMBER_OF_DICS * (int)train_files.classes.size()) + 1) {
		cout << "データセットが少なすぎます" << endl;
		cout
				<< "The number of datasets is less than (NUMBER_OF_DICS + NUMBER_OF_TEST_DATA).";
		return -1;
	}

	std::random_device rnd; //  乱数生成器
	std::mt19937 mt(rnd()); // メルセンヌツイスター 乱数生成器

	auto start = std::chrono::system_clock::now(); // 計測開始時間
	for (int p = 0; p < LOOP; p++) {
		map<double, int> each_class_size;

		//ファイルの読み込み
		train_files = prdc_util::IncludedFilePaths(train_data_path, { ".txt" });
		test_files = prdc_util::IncludedFilePaths(test_data_path, { ".txt" });

		//基底辞書用テキストの設定
		//辞書用テキストはTRAINデータの各クラスからNUMBER_OF_DICS個ずつランダムに選びます
		vector<string> base_dics;

		for (int i = 0; i < (int) train_files.class_size(); ++i) {
			if((int)train_files.each_class_files[i].size() <= (NUMBER_OF_DICS)){
				cout << "ERROR, " << flush;
				return -1;
			}

			for (int p = 0; p < NUMBER_OF_DICS; ++p) {
				std::uniform_int_distribution<> rand(0,
						train_files.each_class_files[i].size() - 1);
				int random_num = rand(mt);
				base_dics.push_back(
						train_files.each_class_files[i].at(random_num));
				prdc_util::remove(train_files.each_class_files[i], random_num);
			}
		}

		//学習用テキストの設定
		vector<string> train_contents;
		vector<double> train_class;

		for (double i = 0; i < (double) train_files.class_size(); ++i) {
			for (auto s : train_files.each_class_files[i]) {
				train_contents.push_back(s);
				train_class.push_back(i);
			}
		}

		//テスト用テキストの設定
		vector<string> test_contents = test_files.included_file_paths;
		vector<double> test_class = test_files.included_file_classes;

//PRDCここから

		prdc::PRDC::crear_last_data();
		//各種手法比較
		for (int n = 0; n < (int) flags.size(); ++n) {
			vector<string> dics(base_dics);
			prdc::PRDC pr_plus(dics, flags.at(n), true, multibyte_char);
			pr_plus.train(train_contents, train_class);

#ifdef DEBUG_OUTPUT
			cout << "学習完了" << endl;
#endif
			cout << flush;
			int accuracy_count = 0;
			map<float, int> accuracy_count_for_each_classes;
			map<float, int> class_size;

			for (int i = 0; i < (int) test_files.classes.size(); ++i) {
				class_size[i] = 0;
			}

//#ifdef PARALLEL
//#pragma omp parallel for
//#endif
			for (int i = 0; i < (int) test_contents.size(); ++i) {
				class_size[test_class.at(i)]++;
//				int tn = omp_get_thread_num() ;
//				cout << "fn_begin: " << tn << endl;
				float r = pr_plus.find_nearest(test_contents.at(i), k);
//				cout << "fn_end: " << tn << endl;
				if (r == test_class.at(i)) {
					accuracy_count++;
					accuracy_count_for_each_classes[test_class.at(i)]++;
				}
				cout << flush;
			}

			accuracys.at(n).at(p) = (double) accuracy_count
					/ (double) test_contents.size();

			for (int i = 0; i < (int) test_files.classes.size(); ++i) {
				float num = (float) i;
				if (class_size[num] == 0)
					continue;
				accuracys_for_each_classes.at(n).at(p)[num] =
						(double) accuracy_count_for_each_classes[num]
								/ (double) class_size[num];
			}
#ifdef DEBUG_OUTPUT
			cout << p + 1 << "回目　手法" << flags_name.at(n) << " 正解率："
					<< accuracys.at(n).at(p) << endl;
			cout << "number_of_dics " << dics.size() << endl;
#endif

		}
	}
//	cout << "~~~~~~~~~~~~~~~~~~~~終了~~~~~~~~~~~~~~~~~~~" << endl;
	vector<double> accuracys_rate_sum(flags.size()); //精度が何％向上したか
	vector<map<float, double>> accuracys_rate_sum_each_classes(flags.size()); //それぞれのクラスで精度が何％向上したか

	map<float, double> accuracy_base_sum_each_classes;

	for (auto& ars : accuracys_rate_sum) {
		ars = 0.0;
	}

	train_files = prdc_util::IncludedFilePaths(train_data_path, { ".txt" });
	test_files = prdc_util::IncludedFilePaths(test_data_path, { ".txt" });

//	cout << "k: " << k << ", loop: " << LOOP << endl;
//	cout << "Number of dics: " << NUMBER_OF_DICS << endl;
//	cout << "Number of learning data: " << train_files.size() << endl;

	ofs << "TRAIN_DATA: " << train_data_path << endl;
	ofs << "TEST_DATA: " << test_data_path << endl;
	ofs << "k: " << k << ", loop: " << LOOP << endl;
	ofs << "Number of dics: " << NUMBER_OF_DICS << endl;
	ofs << "Number of learning data: " << train_files.size() << "\n" << endl;

	ofs << "CLASS LIST" << endl;
	for (int i = 0; i < (int) test_files.class_size(); ++i) {
		float num = (float) i;
		ofs << test_files.classes.at(i) << " >> " << num << endl;
	}
	ofs << endl;
#ifdef DEBUG_OUTPUT
	cout << "~~~~~~~~~~~~~~~~~~~~正解率~~~~~~~~~~~~~~~~~~" << endl;
	cout << "| ~回目 | ";
	for (auto name : flags_name) {
		cout << " " << name << " |";
	}
	cout << endl;
#endif

	ofs << "~~~~~~~~~~~~~~~~~~~~正解率~~~~~~~~~~~~~~~~~~" << endl;
	ofs << "| ~回目 | ";
	for (auto name : flags_name) {
		ofs << " " << name << " |";
	}
	ofs << endl;

	for (int i = 0; i < LOOP; ++i) {
#ifdef DEBUG_OUTPUT
		cout << "| " << i + 1 << "回目 | ";
#endif
		ofs << "| " << i + 1 << "回目 | ";

		for (int n = 0; n < (int) flags.size(); ++n) {
#ifdef DEBUG_OUTPUT
			cout << accuracys.at(n).at(i) << " | ";
#endif
			ofs << accuracys.at(n).at(i) << " | ";
			accuracys_rate_sum.at(n) += accuracys.at(n).at(i);
			for (int m = 0; m < (int) test_files.classes.size(); ++m) {
				float num = (float) m;
				accuracys_rate_sum_each_classes.at(n)[num] +=
						accuracys_for_each_classes.at(n).at(i)[num];
			}

		}
#ifdef DEBUG_OUTPUT
		cout << endl;
#endif
		ofs << endl;
	}

	for (int n = 0; n < (int) flags.size(); ++n) {
		cout << flags_name.at(n) << ": "
				<< (double) accuracys_rate_sum.at(n) / (double) LOOP <<", " <<  flush;
		ofs << flags_name.at(n) << " 平均正解率: "
				<< (double) accuracys_rate_sum.at(n) / (double) LOOP << endl;
	}

	ofs << "\nクラス毎の平均正解率" << endl;

	for (int n = 0; n < (int) flags.size(); ++n) {
		ofs << flags_name.at(n) << endl;
		map<string, double> sorted;
		for (int i = 0; i < (int) test_files.classes.size(); ++i) {
			float num = (float) i;
			sorted[test_files.classes.at(i)] =
					accuracys_rate_sum_each_classes.at(n)[num] / (double) LOOP;
		}
		for (auto m : sorted) {
			ofs << m.first << " : " << m.second << endl;
		}
		ofs << endl;
	}

	auto end = std::chrono::system_clock::now();  // 計測終了時間
	auto elapsed = end - start;
//	cout << "処理時間:" << chrono::duration_cast<chrono::seconds>(elapsed).count()
//			<< "秒" << endl;

	ofs << "\n処理時間:" << chrono::duration_cast<chrono::seconds>(elapsed).count()
			<< "秒" << endl;
	ofs << endl;

	ofs.close();

	return 0;
}

