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
#include "tools.h"
#include "Lzw.h"
#include "Dictionary.h"

using namespace std;
int main() {

	vector<string> filename;
	vector<string> file_contents;
	vector<prdc_lzw::Dictionary*> dics;
	vector<vector<string>> compressed;
	vector<prdc_lzw::LzwPair> pair;
	prdc_lzw::BindingMap bind_data;

	filename.push_back("./data/corel/354.txt");
	filename.push_back("./data/corel/377.txt");
	filename.push_back("./data/corel/1.txt");

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
	compressed.resize(filename.size());

	for (int i = 0; i < (int) filename.size(); ++i) {
		//fileiの辞書作成
		prdc_lzw::CompressBoundData(file_contents.at(i), compressed.at(i),
				*dics.at(i), bind_data, prdc_lzw::ARROW_EDIT_DICTIONARY);

		//file0の圧縮後文字列出力
		ofstream ofs("output/file" + to_string(i) + "compressed.txt");
		for (auto c : compressed.at(i)) {
			ofs << c << "\n";
		}
		ofs.close();
	}
	for (auto dic : dics) {
		delete dic;
	}


//	string A = "aaaaaiiiii";
//	string B = "iiiiiaaaaa";
//
//	prdc_lzw::Dictionary dicA;
//	prdc_lzw::Dictionary dicB;
//
//	vector<string> comp;
//	prdc_lzw::CompressBoundData(A, comp, dicA, bind_data,
//			prdc_lzw::ARROW_EDIT_DICTIONARY);
//	cout << A << endl;
//	for (auto s : comp) {
//		cout << s << ", ";
//	}
//	cout << endl;
//	comp.clear();
//	prdc_lzw::CompressBoundData(B, comp, dicB, bind_data,
//			prdc_lzw::ARROW_EDIT_DICTIONARY);
//	cout << B << endl;
//	for (auto s : comp) {
//		cout << s << ", ";
//	}
//	cout << endl;
//	cout << "Binding" << endl;
//	for (auto m : bind_data) {
//		if (m.second > 255)
//			cout << m.first << "->" << m.second << endl;
//	}
	getchar();
	return 0;
}
