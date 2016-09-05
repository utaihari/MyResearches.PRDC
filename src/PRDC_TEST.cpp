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
#include "TEST.h"

namespace fs = boost::filesystem;
using namespace std;
using namespace prdc_lzw;
using namespace prdc_util;
using namespace cv;

int PRDC_TEST(string dataset_path, int method_flag, int LOOP,
		int NUMBER_OF_DICS, int NUMBER_OF_TEST_DATA, int k) {

	string filename = "PRDC_LOG/" + CurrentTimeString() + ".txt";
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

	for (int flag : flags) {

		Dictionary dic;
		vector<string> file_paths;
		vector<float> file_classes;
		map<string, float> classes;

		//ファイルの読み込み
		prdc_util::GetEachFilePathsAndClasses(dataset_path, file_paths, file_classes, classes);

		if ((int) file_paths.size()
				< NUMBER_OF_DICS + NUMBER_OF_TEST_DATA + 1) {
			cout << "データセットが少なすぎます" << endl;
			cout
					<< "The number of datasets is less than (NUMBER_OF_DICS + NUMBER_OF_TEST_DATA).";
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

#pragma omp parallel for
			for (int i = 0; i < (int) test_contents.size(); ++i) {

				float r = pr.find_nearest(test_contents.at(i), k);

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
