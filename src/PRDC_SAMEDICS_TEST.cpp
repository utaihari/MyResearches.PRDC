/*
 * PRDC_SAMEDICS_TEST.cpp
 *
 *  Created on: 2016/07/20
 *      Author: uchinosub
 */

#include "TEST.h"

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
#include "Dictionary.h"
#include "util.h"
#include "PRDC.h"

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
int PRDC_SAMEDICS_TEST(string dataset_path, int method_flag, int LOOP,
		int NUMBER_OF_DICS, int NUMBER_OF_TEST_DATA, int k,
		bool multibyte_char) {

	string filename = "LOG/PRDC_SAMEDICS_LOG/" + CurrentTimeString() + ".txt";
	std::ofstream ofs(filename);

	vector<int> flags;
	vector<string> flags_name;

	for (int i = 0; i < (int) prdc::METHOD_ARRAY.size(); ++i) {
		if (method_flag & prdc::METHOD_ARRAY.at(i)) {
			flags.push_back(prdc::METHOD_ARRAY.at(i));
			flags_name.push_back(prdc::METHOD_NAME_ARRAY.at(i));
		}
	}

	vector<string> file_paths;
	vector<float> file_classes;
	std::vector<std::string> classes;

	vector<vector<double>> accuracys(flags.size());
	vector<vector<map<float, double>>> accuracys_for_each_classes(flags.size());
	vector<double> accuracy_base(LOOP);
	vector<map<float, double>> accuracy_base_for_each_classes(LOOP);

	for (int i = 0; i < (int) flags.size(); ++i) {
		accuracys.at(i).resize(LOOP);
		accuracys_for_each_classes.at(i).resize(LOOP);
	}

	//ファイルの読み込み
	GetEachFilePathsAndClasses(dataset_path, file_paths, file_classes, classes);

	cout << "読み込んだクラス一覧" << endl;
	for (auto c : classes) {
		cout << c << endl;
	}

	if ((int) file_paths.size() < NUMBER_OF_DICS + NUMBER_OF_TEST_DATA + 1) {
		cout << "データセットが少なすぎます" << endl;
		cout
				<< "The number of datasets is less than (NUMBER_OF_DICS + NUMBER_OF_TEST_DATA).";
		return -1;
	}

	int NUMBER_OF_LEARNING = (int) file_paths.size()
			- (NUMBER_OF_DICS + NUMBER_OF_TEST_DATA);

	std::random_device rnd; //  乱数生成器
	std::mt19937 mt(rnd()); // メルセンヌツイスター 乱数生成器

	auto start = std::chrono::system_clock::now(); // 計測開始時間
	for (int p = 0; p < LOOP; p++) {
		map<float, int> each_class_size;
		//学習用テキストなどをランダムに選ぶためのランダムな数値が入った配列
		vector<int> random_num(file_paths.size());

		for (int i = 0; i < (int) random_num.size(); ++i) {
			random_num.at(i) = i;
		}
		shuffle(random_num.begin(), random_num.end(), mt);

		//学習用テキストの設定
		vector<string> learning_contents;
		vector<float> learning_class;
		for (int i = 0; i < NUMBER_OF_LEARNING; ++i) {
			learning_contents.push_back(file_paths.at(random_num.at(i)));
			learning_class.push_back(file_classes.at(random_num.at(i)));
		}

		//テスト用テキストの設定
		vector<string> test_contents;
		vector<float> test_class;
		for (int i = NUMBER_OF_LEARNING;
				i < (int) file_paths.size() - NUMBER_OF_DICS; ++i) {
			test_contents.push_back(file_paths.at(random_num.at(i)));
			test_class.push_back(file_classes.at(random_num.at(i)));
			each_class_size[file_classes.at(random_num.at(i))]++;
		}

		//基底辞書用テキストの設定
		vector<string> base_dics;
		for (int i = NUMBER_OF_LEARNING + NUMBER_OF_TEST_DATA;
				i < (int) file_paths.size(); ++i) {
			base_dics.push_back(file_paths.at(random_num.at(i)));
		}

//PRDCここから

//基底辞書の設定
		prdc::PRDC pr(base_dics, prdc::PRDC_BASE, method_flag, false,
				multibyte_char);
		cout << "基底辞書作成完了" << endl;
		pr.train(learning_contents, learning_class);
		cout << "学習完了" << endl;
		cout << "テスト開始（多少時間がかかります）" << endl;

		int accuracy_count = 0;
		map<float, int> accuracy_count_base_for_each_classes;

		for (int i = 0; i < (int) classes.size(); ++i) {
			accuracy_count_base_for_each_classes[(float) i] = 0;
		}

#pragma omp parallel for
		for (int i = 0; i < (int) test_contents.size(); ++i) {

			float r = pr.find_nearest(test_contents.at(i), k);
			if (r == test_class.at(i)) {
				//正解個数の計算
				accuracy_count++;
				accuracy_count_base_for_each_classes[test_class.at(i)]++;
			}
		}

		//正解個数の平均
		accuracy_base.at(p) = (double) accuracy_count
				/ (double) test_contents.size();

		for (int i = 0; i < (int) classes.size(); ++i) {
			float num = (float) i;
			if (each_class_size[num] == 0)
				continue;
			accuracy_base_for_each_classes.at(p)[num] =
					(double) accuracy_count_base_for_each_classes[num]
							/ (double) each_class_size[num];
		}

		cout << p + 1 << "回目　PRDC正解率：" << accuracy_base.at(p) << endl;

		//各種手法比較
		for (int n = 0; n < (int) flags.size(); ++n) {

			prdc::PRDC pr_plus(base_dics, flags.at(n), 0, true, multibyte_char);
			pr_plus.train(learning_contents, learning_class);

			int accuracy_count = 0;
			map<float, int> accuracy_count_for_each_classes;

#pragma omp parallel for
			for (int i = 0; i < (int) test_contents.size(); ++i) {

				float r = pr_plus.find_nearest(test_contents.at(i), k);

				if (r == test_class.at(i)) {
					accuracy_count++;
					accuracy_count_for_each_classes[test_class.at(i)]++;
				}
			}

			accuracys.at(n).at(p) = (double) accuracy_count
					/ (double) test_contents.size();

			for (int i = 0; i < (int) classes.size(); ++i) {
				float num = (float) i;
				if (each_class_size[num] == 0)
					continue;
				accuracys_for_each_classes.at(n).at(p)[num] =
						(double) accuracy_count_for_each_classes[num]
								/ (double) each_class_size[num];
			}
			cout << p + 1 << "回目　手法" << flags_name.at(n) << " 正解率："
					<< accuracys.at(n).at(p) << endl;

		}
	}
	cout << "~~~~~~~~~~~~~~~~~~~~終了~~~~~~~~~~~~~~~~~~~" << endl;
	vector<double> accuracys_rate_sum(flags.size()); //精度が何％向上したか
	vector<map<float, double>> accuracys_rate_sum_each_classes(flags.size()); //それぞれのクラスで精度が何％向上したか

	double accuracy_base_sum = 0.0; //PRDC(先行研究)の平均正解率
	map<float, double> accuracy_base_sum_each_classes;

	for (auto& ars : accuracys_rate_sum) {
		ars = 0.0;
	}

	cout << "k: " << k << ", loop: " << LOOP << endl;
	cout << "Number of dics: " << NUMBER_OF_DICS << endl;
	cout << "Number of test data: " << NUMBER_OF_TEST_DATA << endl;
	cout << "Number of learning data: " << NUMBER_OF_LEARNING << endl;

	ofs << "Dataset: " << dataset_path << endl;
	ofs << "k: " << k << ", loop: " << LOOP << endl;
	ofs << "Number of classes: " << classes.size() << endl;
	ofs << "Number of dics: " << NUMBER_OF_DICS << endl;
	ofs << "Number of test data: " << NUMBER_OF_TEST_DATA << endl;
	ofs << "Number of learning data: " << NUMBER_OF_LEARNING << endl << endl;

	ofs << "CLASS LIST" << endl;
	for (int i = 0; i < (int) classes.size(); ++i) {
		float num = (float) i;
		ofs << classes.at(i) << " >> " << num << endl;
	}
	ofs << endl;
	cout << "~~~~~~~~~~~~~~~~~~~~正解率~~~~~~~~~~~~~~~~~~" << endl;
	cout << "| ~回目 | PRDC |";
	for (auto name : flags_name) {
		cout << " " << name << " |";
	}
	cout << endl;

	ofs << "~~~~~~~~~~~~~~~~~~~~正解率~~~~~~~~~~~~~~~~~~" << endl;
	ofs << "| ~回目 | PRDC | ";
	for (auto name : flags_name) {
		ofs << " " << name << " |";
	}
	ofs << endl;

	for (int i = 0; i < LOOP; ++i) {
		cout << "| " << i + 1 << "回目 | " << accuracy_base.at(i) << " | ";
		ofs << "| " << i + 1 << "回目 | " << accuracy_base.at(i) << " | ";
		accuracy_base_sum += accuracy_base.at(i);

		for (int i = 0; i < (int) classes.size(); ++i) {
			float num = (float) i;
			accuracy_base_sum_each_classes[num] +=
					accuracy_base_for_each_classes.at(i)[num];
		}

		for (int n = 0; n < (int) flags.size(); ++n) {
			cout << accuracys.at(n).at(i) << " | ";
			ofs << accuracys.at(n).at(i) << " | ";

			accuracys_rate_sum.at(n) += accuracys.at(n).at(i);

			for (int m = 0; m < (int) classes.size(); ++m) {
				float num = (float) m;
				accuracys_rate_sum_each_classes.at(n)[num] +=
						accuracys_for_each_classes.at(n).at(m)[num];
			}

		}
		cout << endl;
		ofs << endl;
	}

	cout << "PRDC 平均正解率: " << (double) accuracy_base_sum / (double) LOOP
			<< endl;
	ofs << "PRDC 平均正解率: " << (double) accuracy_base_sum / (double) LOOP << endl;
	for (int n = 0; n < (int) flags.size(); ++n) {
		cout << flags_name.at(n) << " 平均正解率: "
				<< (double) accuracys_rate_sum.at(n) / (double) LOOP << endl;
		ofs << flags_name.at(n) << " 平均正解率: "
				<< (double) accuracys_rate_sum.at(n) / (double) LOOP << endl;
	}

	ofs << "\nクラス毎の平均正解率" << endl;

	ofs << "PRDC" << endl;
	for (int i = 0; i < (int) classes.size(); ++i) {
		float num = (float) i;
		ofs << classes.at(i) << " : "
				<< accuracy_base_sum_each_classes[num] / (double) LOOP << endl;
	}
	ofs << endl;

	for (int n = 0; n < (int) flags.size(); ++n) {
		ofs << flags_name.at(n) << endl;
		for (int i = 0; i < (int) classes.size(); ++i) {
			float num = (float) i;
			ofs << classes.at(i) << " : "
					<< accuracys_rate_sum_each_classes.at(n)[num]
							/ (double) LOOP << endl;
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

