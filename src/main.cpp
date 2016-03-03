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
#include "Dictionary.h"
#include "util.h"

using namespace std;
int main() {
	const string dataset_folder =
			"/home/uchinosub/git/MyResearches.ImageToText/output/corel/";
	vector<string> filename;
	vector<string> file_contents;
	vector<prdc_lzw::Dictionary*> dics;

	const int max_dic = 0;

	filename.push_back(dataset_folder + "354.txt");
	filename.push_back(dataset_folder + "377.txt");
	filename.push_back(dataset_folder + "1.txt");

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

	double h;
	h = HistgramIntersection(dics.at(0)->histgram, dics.at(1)->histgram);
	cout << "1-2 :" << h << endl;
	h = HistgramIntersection(dics.at(0)->histgram, dics.at(2)->histgram);
	cout << "1-3 :" << h << endl;

	for (auto dic : dics) {
		delete dic;
	}

//	string A = "aaaaaiiiiiaiueaiueaiue";
//
//	prdc_lzw::Dictionary dicA;
//
//	//dicA.max_dicsize = 259;
//
//	dicA.Compress(A,prdc_lzw::ARROW_EDIT_DICTIONARY);
//	cout << A << endl;
//	for (auto s : dicA.compressed) {
//		cout << s << ", ";
//	}
//	cout << endl;
//
//	cout << "Binding Array size: " << dicA.binding.size() << endl;
//
//	for (unsigned int i = 256; i < dicA.binding.size(); ++i) {
//		cout << i << ":" << dicA.binding.at(i) << endl;
//	}
//	vector<pair<string,int>> histgram;
//	histgram = dicA.MakeHistgram();
//
//	for (unsigned int i = 0; i < histgram.size(); ++i) {
//		cout << histgram.at(i).first << ":" << histgram.at(i).second << endl;
//	}

	return 0;
}
