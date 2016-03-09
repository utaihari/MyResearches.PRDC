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
#include "Dictionary.h"
#include "util.h"
#define GLIBCXX_FORCE_NEW
//#define test

using namespace std;
using namespace prdc_lzw;
using namespace prdc_util;
int main() {

#ifndef test
	const string dataset_folder =
	"/home/uchinosub/git/MyResearches.ImageToText/output/corel/";
	vector<string> filename;
	vector<string> file_contents;
	vector<prdc_lzw::Dictionary*> dics;

	const int max_dic = 0;

	filename.push_back(dataset_folder + "354.txt");
	filename.push_back(dataset_folder + "377.txt");
	filename.push_back(dataset_folder + "1.txt");

//	filename.push_back(dataset_folder + "432.txt");
//	filename.push_back(dataset_folder + "432.txt");
//	filename.push_back(dataset_folder + "1.txt");

	for (int i = 0; i < (int) filename.size(); ++i) {
		ifstream ifs(filename.at(i));
		if (ifs.fail()) {
			cerr << "読み込みエラー" << endl;
		}
		file_contents.push_back(
				string((istreambuf_iterator<char>(ifs)),
						istreambuf_iterator<char>()));

	}

	for (int i = 0; i < (int) filename.size(); ++i) {
		prdc_lzw::Dictionary* temp = new prdc_lzw::Dictionary;
		dics.push_back(temp);
	}

	for (int i = 0; i < (int) filename.size(); ++i) {
		//fileiの辞書作成
		dics.at(i)->max_dicsize = max_dic;
		dics.at(i)->Compress(file_contents.at(i),
				prdc_lzw::ARROW_EDIT_DICTIONARY);

		//fileiのヒストグラム作成
		dics.at(i)->MakeHistgram();

		ofstream ofs("output/file" + to_string(i) + "histgram.txt");

		for (auto d : dics.at(i)->histgram) {
			ofs << d.first << ":" << d.second << "\n";
		}
		ofs << flush;
	}

	double nmd;
	nmd = NormalizedMultisetDistance(*dics.at(0), *dics.at(1));
	cout << "nmd 1-2 :" << nmd << endl;
	nmd = NormalizedMultisetDistance(*dics.at(0), *dics.at(2));
	cout << "nmd 1-3 :" << nmd << endl;

	double h;
	h = HistgramIntersection(dics.at(0)->histgram, dics.at(1)->histgram);
	cout << "histgram 1-2 :" << h << endl;
	h = HistgramIntersection(dics.at(0)->histgram, dics.at(2)->histgram);
	cout << "histgram 1-3 :" << h << endl;

	for (int i = 0; i < 3; i++) {
		delete dics[i];
	}

#else
	string A = "aaa";
	string B = "bbb";

	prdc_lzw::Dictionary dicA;
	prdc_lzw::Dictionary dicB;

//	dicA.max_dicsize = 0;
//	dicB.max_dicsize = 0;

	dicA.Compress(A, prdc_lzw::ARROW_EDIT_DICTIONARY);
	dicB.Compress(B, prdc_lzw::ARROW_EDIT_DICTIONARY);

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
