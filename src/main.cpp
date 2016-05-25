/**
 *  @file main.cpp
 *
 *  @date 2015/11/19
 *  @author uchinosub
 */
#include <iostream>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <string>
#include <iterator>
#include <sstream>
#include <functional>
#include <memory>
#include <set>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "opencv2/opencv.hpp"
#include "Dictionary.h"
#include "util.h"
#define GLIBCXX_FORCE_NEW
//#define test

using namespace std;
using namespace prdc_lzw;
using namespace prdc_util;
using namespace cv;

int main() {

#ifndef test
//	const string dataset_folder =
//			"/home/uchinosub/git/MyResearches.ImageToText/output/corel/";
//	const string image_folder =
//			"/home/uchinosub/git/MyResearches.ImageToText/dataset/Corel/corel/";

	const string dataset_folder =
			"/home/uchinosub/git/MyResearches.ImageToText/output/";
	const string image_folder = "/home/uchinosub/workspace/CreateImage/output/";
	vector<string> filename;
	vector<string> file_image;
	vector<string> file_contents;
	vector<prdc_lzw::Dictionary*> dics;

	vector<string> data_num = { "16()/0", "16()/1", "2(0-)/0", "2(0-)/5",
			"2(1-)/5" };
	const int ROOT_NUM = 0; //（全てのデータの圧縮に用いる辞書）の作成に用いるデータの番号

	for (int i = 0; i < (int) data_num.size(); ++i) {
		filename.push_back(dataset_folder + data_num[i] + ".txt");
	}

	//ファイル読み込み
	for (int i = 0; i < (int) filename.size(); ++i) {
		ifstream ifs(filename.at(i));
		if (ifs.fail()) {
			cerr << "読み込みエラー" << endl;
		}
		file_contents.push_back(
				string((istreambuf_iterator<char>(ifs)),
						istreambuf_iterator<char>()));

	}

	vector<vector<ofstream*>> ofs;
	vector<vector<string>> compressed;
	vector<vector<pair<string, string>>> str_pair;
	vector<vector<vector<pair<string, string>>> > found_pair;

	compressed.resize(filename.size());
	str_pair.resize(filename.size());
	found_pair.resize(filename.size());
	ofs.resize(filename.size());
	for (int i = 0; i < (int) filename.size(); ++i) {
		found_pair.at(i).resize(filename.size());
		ofs.at(i).resize(filename.size());
	}

	for (int i = 0; i < (int) filename.size(); ++i) {
		for (int p = 0; p < (int) filename.size(); ++p) {
			if (i == p)
				continue;
			ofstream* temp = new ofstream(
					"output/pair" + data_num[i] + "-" + data_num[p] + ".txt");
			ofs[i][p] = temp;
		}
	}

	//辞書作成
	for (int i = 0; i < (int) file_contents.size(); ++i) {
		Dictionary* temp = new Dictionary(file_contents.at(i));
		dics.push_back(temp);
	}

	//画像定義
	vector<Mat> image;
	vector<string> title;
	vector<string> compress_size;
	vector<ComparisonImage*> output_image;
	SavingImages images();

	image.resize(filename.size());
	title.resize(filename.size());
	compress_size.resize(filename.size());
	output_image.resize(filename.size());

	for (int i = 0; i < (int) filename.size(); ++i) {
		compressed[i] = ConvertNumtoStr(
				Compress(file_contents.at(i), *dics[ROOT_NUM]),
				dics.at(ROOT_NUM)->binding);
		str_pair[i] = MakePair(compressed[i]);
		cout << "pair" << i << "length: " << str_pair[i].size() << endl;

		//画像作成
		image.at(i) = cv::imread(image_folder + data_num[i] + ".jpg");

		title.at(i) = "\"" + data_num[i] + ".jpg\"";
		compress_size.at(i) = string("size: ")
				+ to_string(compressed[i].size());

		Scalar text_color;
		if (i == ROOT_NUM) {
			text_color = Scalar(50, 50, 255);

		} else {
			text_color = cv::Scalar(0, 0, 0);
		}

		int height = image.at(i).rows;
		const int STEP = 35;
		if ((STEP * (int) filename.size()) > image.at(i).rows) {
			height = STEP * (int) filename.size();
		}
		//output_image作成
		output_image.at(i) = new ComparisonImage(
				{ title.at(i), compress_size.at(i) }, image.at(i), text_color,
				image.at(i).cols + 350, height);
	}

//画像保存用フォルダ作成
	struct tm *date;
	time_t now = time(NULL);
	date = localtime(&now);

	string today = to_string(date->tm_mon + 1) + "月" + to_string(date->tm_mday)
			+ "日" + to_string(date->tm_hour) + "時" + to_string(date->tm_min)
			+ "分" + to_string(date->tm_sec) + "秒";

	string path = "output/" + today + ":" + to_string(data_num.size());
	struct stat st;
	if (stat(path.c_str(), &st) != 0) {
		mkdir(path.c_str(), 0775);
	}

	for (int i = 0; i < (int) filename.size(); ++i) {

		for (int p = 0; p < (int) filename.size(); ++p) {
			if (i == p)
				continue;

			found_pair[i][p] = FindPair(str_pair[i], str_pair[p]);
			cout << data_num[i] << "-" << data_num[p] << "size: "
					<< found_pair[i][p].size() << endl;
			for (auto s : found_pair[i][p]) {
				*ofs[i][p] << s.first << " , " << s.second << endl;
			}

			//text挿入
			string text = string("->\"") + data_num[p] + ".jpg\": "
					+ to_string(found_pair[i][p].size());

			Scalar text_color(255, 245, 255);

			if (i == ROOT_NUM || p == ROOT_NUM)
				text_color = Scalar(100, 100, 255);

			output_image.at(i)->Push(text, text_color);

		}
		imshow(output_image.at(i)->image_title, output_image.at(i)->image);

		//タイトルから"/"の抜き取り
		for (size_t c = title.at(i).find_first_of("/"); c != string::npos; c =
				title.at(i).find_first_of("/")) {
			title.at(i).erase(c, 1);
		}
		//画像保存
		imwrite(path + "/" + output_image.at(i)->image_title + ".png",
				output_image.at(i)->image);
		waitKey(30);
	}

	for (auto c : dics) {
		delete c;
	}
	for (int i = 0; i < (int) filename.size(); ++i) {
		for (int p = 0; p < (int) filename.size(); ++p) {
			if (i == p)
				continue;
			delete ofs[i][p];
		}
	}
	for (auto images : output_image) {
		delete images;
	}
	cv::waitKey();
#else
	string A = "aaa";
	string B = "bbb";

	prdc_lzw::Dictionary dicA;
	prdc_lzw::Dictionary dicB;

//	dicA.max_dicsize = 0;
//	dicB.max_dicsize = 0;

	dicA.Compress(A, prdc_lzw::DONOT_EDIT_DICTIONARY);
	dicB.Compress(B, prdc_lzw::DONOT_EDIT_DICTIONARY);

	cout << endl;

	double histgram;
	dicA.MakeHistgram();
	dicB.MakeHistgram();

	histgram = HistgramIntersection(dicA.histgram, dicB.histgram);
	cout << "histgram:" << histgram << endl;
	double nmd = NormalizedMultisetDistance(dicA, dicB);
	cout << "nmd:" << nmd << endl;

#endif

	return 0;
}
