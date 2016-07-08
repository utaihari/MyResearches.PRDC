/**
 *  @file main.cpp
 *
 *  @date 2015/11/19
 *  @author uchinosub
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

int main(int argc, char* argv[]) {

	//基底辞書作成用テキストのパス
	const string DATA_SET_PATH(
			"/home/uchinosub/git/MyResearches.ImageToText/output/image-orig/");

	vector<string> file_paths;
	vector<float> file_classes;
	map<string, float> classes;

	//ファイルの読み込み
	ReadFiles(DATA_SET_PATH, file_paths, file_classes, classes);

	//学習用テキストなどをランダムに選ぶためのランダムな数値が入った配列
	vector<int> random_num(file_paths.size());
	std::random_device rnd;       //  乱数生成器
	std::mt19937 mt(rnd());        // メルセンヌツイスター 乱数生成器
	for (int i = 0; i < (int) random_num.size(); ++i) {
		random_num.at(i) = i;
	}
	shuffle(random_num.begin(), random_num.end(), mt);

	//学習用テキストの設定
	vector<string> learning_contents;
	vector<float> learning_class;
	for (int i = 0; i < 960; ++i) {
		learning_contents.push_back(file_paths.at(random_num.at(i)));
		learning_class.push_back(file_classes.at(random_num.at(i)));
	}

	//テスト用テキストの設定
	vector<string> test_contents;
	vector<float> test_class;
	for (int i = 960; i < 990; ++i) {
		test_contents.push_back(file_paths.at(random_num.at(i)));
		test_class.push_back(file_classes.at(random_num.at(i)));
	}

	//基底辞書用テキストの設定
	vector<string> base_dics;
	for (int i = 990; i < 1000; ++i) {
		base_dics.push_back(file_paths.at(random_num.at(i)));
	}

//PRDCここから

//基底辞書の設定
	prdc::PRDC pr(base_dics);

	//prdc::PRDC pr("test.xml");

	cout << "基底辞書作成完了" << endl;
	pr.train(learning_contents, learning_class);
	cout << "学習完了" << endl;

	pr.save("test.xml");

	double accuracy_rate = 0.0;
	int accuracy_count = 0;

#pragma omp parallel for
	for (int i = 0; i < (int) test_contents.size(); ++i) {
		int k = 5;

		float r = pr.find_nearest(test_contents.at(i), k);

		if (r == test_class.at(i)) {
			accuracy_count++;
		}

		cout << "r:" << r << ", A:" << test_class.at(i) << endl;
	}

	accuracy_rate = (double) accuracy_count / (double) test_contents.size();

	cout << "正解率:" << accuracy_rate << endl;

	return 0;
}
