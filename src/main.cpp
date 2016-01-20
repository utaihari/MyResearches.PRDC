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

	filename.push_back("./data/text/buildings-33.txt");
	filename.push_back("./data/text/buildings-32.txt");
	filename.push_back("./data/text/denseresidential-01.txt");

	for (int i = 0; i < (int) filename.size(); i++) {
		prdc_lzw::Dictionary* temp = new prdc_lzw::Dictionary;
		dics.push_back(temp);
	}
	compressed.resize(4);
	pair.resize(4);

	for (int i = 0; i < (int) filename.size(); i++) {
		ifstream ifs(filename.at(i));
		if (ifs.fail()) {
			cerr << "読み込みエラー" << endl;
		}
		file_contents.push_back(
				string((istreambuf_iterator<char>(ifs)),
						istreambuf_iterator<char>()));

	}

	for (int i = 0; i < (int) filename.size(); i++) {
		cout << "Source" << i << " = \"" << filename.at(i) << "\"" << endl;
	}
	cout << endl;

	//file0の辞書作成
	CompressWithMakePair(file_contents.at(0), compressed.at(0), *dics.at(0),
			pair.at(0));

	//file0の圧縮後文字列出力
	ofstream ofs0("file0_compress.txt");
	for (auto c : compressed.at(0)) {
		ofs0 << c << endl;
	}
	ofs0.close();
	cout << "file0 size:" << compressed.at(0).size() << endl;
	cout << endl;

	compressed.at(0).clear();

	//file0から作成した辞書でfile1を圧縮
	CompressWithMakePair(file_contents.at(2), compressed.at(0), *dics.at(0),
			pair.at(1), prdc_lzw::ARROW_EDIT_PAIR);

	//file1をfile0で圧縮した時に作成したペアがfile0を再圧縮した時に現れるか調べる
	CompressWithMakePair(file_contents.at(0), compressed.at(1), *dics.at(0),
			pair.at(1));

	ofstream ofs1("file0_recompress_with_dic0_pair2.txt");
	for (auto c : compressed.at(1)) {
		ofs1 << c << endl;
	}
	ofs1.close();

	for (auto dic : dics) {
		delete dic;
	}

	getchar();
	return 0;
}
