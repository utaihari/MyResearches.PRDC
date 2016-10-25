/*
 * PRDC_SIMILAR_IMAGE_DISPLAY.cpp
 *
 *  Created on: 2016/09/19
 *      Author: uchinosub
 */

#include "../TEST.h"

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
int PRDC_SIMILAR_IMAGE_DISPLAY(string input_image_path, string dataset_path,
		string images_path, int method_flag, int NUMBER_OF_DICS, int output_num,
		string savefile, string loadfile,string output_folder) {

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

	vector<string> image_paths;
	vector<float> image_classes;
	std::vector<std::string> iclasses;

	//データセットの読み込み
	GetEachFilePathsAndClasses(dataset_path, file_paths, file_classes, classes);
	//画像フォルダの読み込み
	GetEachFilePathsAndClasses(images_path, image_paths, image_classes,
			iclasses);

	if (file_paths.size() != image_paths.size()) {
		cerr
				<< "画像フォルダとテキストフォルダの構造が違います(もしくはパスの記述が間違っています).\nもう一度ImageToText.exeを正しく実行してください."
				<< endl;
	}

	Mat input_image = imread(input_image_path);
	unsigned char* input_image_char;
	input_image_char =
			new unsigned char[input_image.rows * input_image.cols + 1];
	ImageToString(input_image, input_image_char, 5);
	string input_image_data = string((char*) input_image_char);
	delete input_image_char;

	std::random_device rnd; //  乱数生成器
	std::mt19937 mt(rnd()); // メルセンヌツイスター 乱数生成器

	//学習用テキストなどをランダムに選ぶためのランダムな数値が入った配列
	vector<int> random_num(NUMBER_OF_DICS);

	for (int i = 0; i < (int) random_num.size(); ++i) {
		random_num.at(i) = i;
	}
	shuffle(random_num.begin(), random_num.end(), mt);

	//学習用テキストの設定
	vector<string> learning_contents;
	vector<string> learning_contents_path;
	vector<float> learning_class;
	for (int i = 0; i < (int) file_paths.size(); ++i) {
		learning_contents.push_back(file_paths.at(i));
		learning_contents_path.push_back(image_paths.at(i));
		learning_class.push_back(file_classes.at(i));
	}

	//基底辞書用テキストの設定
	vector<string> base_dics;

	for (int i = 0; i < NUMBER_OF_DICS; ++i) {
		base_dics.push_back(file_paths.at(random_num.at(i)));

	}

//PRDCここから
//基底辞書の設定
	prdc::PRDC pr;
	if (loadfile == "") {
		pr = prdc::PRDC(base_dics, method_flag, 0, false, false);
		cout << "基底辞書作成完了" << endl;
		pr.train(learning_contents, learning_class);
		cout << "学習完了" << endl;
	} else {
		pr = prdc::PRDC(loadfile);
	}

	cout << "検索開始（多少時間がかかります）" << endl;

	vector<int> r = pr.find_near_vector(input_image_data, output_num, true);
	vector<Mat> output_images;
	ChangeDatasetPath cdp(dataset_path, images_path);
	SavingImages si;

	for (auto num : r) {
//		cout << learning_contents_path.at(num) << endl;
		cout << cdp.ChangePath(learning_contents.at(num)) << endl;
		cout << endl;
		output_images.push_back(
				Mat(imread(cdp.ChangePath(learning_contents.at(num)))));
	}

	imshow("input", input_image);
	cvMoveWindow("input", 500, 1000);
	si.Push("input", input_image);

	for (int i = 0; i < (int) output_images.size(); ++i) {
		imshow(to_string(i), output_images.at(i));
		cvMoveWindow(to_string(i).c_str(), 1000, 1000);
		si.Push(to_string(i), output_images.at(i));
	}
	si.Save(output_folder,flags_name.at(0) + "-");
	if (savefile != "") {
		pr.save(savefile);
	}

	waitKey(0);

	return 0;
}

