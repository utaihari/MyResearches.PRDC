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

#define compStr 0

using namespace std;
int main() {

	vector<string> compressed, recompressed;
	prdc_lzw::Dictionary dic1, dic2;

#if compStr
	vector<string> split;
	const string A = "TOBEORNOTTOBEORTOBEORNOT";
	const string B = "TOUKYOUTOKKYOKYOKAKYOKU";

#else

	vector<string> filename;
	vector<string> file_contents;
	vector < vector < string >> split_string;
	vector<vector<prdc_lzw::Dictionary*>> dics;
	const int split_number = 1;

	filename.push_back("./data/text/buildings-00.txt");
	filename.push_back("./data/text/buildings-01.txt");
	filename.push_back("./data/text/denseresidential-20.txt");

	split_string.resize(filename.size());
	dics.resize(filename.size());

	for (int i = 0; i < (int) filename.size(); i++) {
		ifstream ifs(filename.at(i));
		if (ifs.fail()) {
			cerr << "読み込みエラー" << endl;
		}
		file_contents.push_back(
				string((istreambuf_iterator<char>(ifs)),
						istreambuf_iterator<char>()));

		SplitString(file_contents.at(i), split_string.at(i), split_number);
	}

#endif

#if compStr
	cout << "String A = " << A << endl;
	cout << "String B = " << B << endl;
	cout << endl;
	cout << endl;
#else
	for (int i = 0; i < (int) filename.size(); i++) {
		cout << "Source" << i << " = \"" << filename.at(i) << "\"" << endl;
	}
	cout << endl;
#endif
#if compStr

	prdc_lzw::compress(A, compressed, dic1);
	prdc_lzw::compress_with_outer_dictionary(A, recompressed, dic1);

	cout << "String A compression for extract dic1: size = "
	<< compressed.size() << endl;
	for (auto i : compressed) {
		cout << i << ",";
	}
	cout << endl;
	cout << endl;
	cout << "String A compression with dic1: size = " << recompressed.size()
	<< endl;
	for (auto i : recompressed) {
		cout << i << ",";
	}
	cout << endl;
	cout << endl;
	cout << endl;

	compressed.clear();
	recompressed.clear();

	prdc_lzw::compress(B, compressed, dic2);
	prdc_lzw::compress_with_outer_dictionary(B, recompressed, dic2);
	cout << "String B compression for extract dic2 = " << compressed.size()
	<< endl;
	for (auto i : compressed) {
		cout << i << ",";
	}
	cout << endl;
	cout << endl;

	cout << "String B compression with dic2: size = " << recompressed.size()
	<< endl;
	for (auto i : recompressed) {
		cout << i << ",";
	}
	cout << endl;
	cout << endl;

	recompressed.clear();
	prdc_lzw::compress_with_outer_dictionary(B, recompressed, dic1);

	cout << "String B compression with dic1: size = " << recompressed.size()
	<< endl;
	for (auto i : recompressed) {
		cout << i << ",";
	}
	cout << endl;
#else

	//辞書作成
	for (int i = 0; i < (int) file_contents.size(); i++) {
		for (int p = 0; p < split_number; p++) {
			prdc_lzw::Dictionary* tempDic = new prdc_lzw::Dictionary();
			dics.at(i).push_back(tempDic);
			prdc_lzw::compress(split_string.at(i).at(p), compressed,
					*dics.at(i).at(p));
			cout << "Source" << i << " compression for extract dic" << i
					<< ": size = " << compressed.size() << endl;

			compressed.clear();
		}
	}

	cout << endl;

	//比較
	for (int i = 0; i < (int) split_string.size(); i++) {
		int total = 0;
		for (int p = 0; p < split_number; p++) {
			for (int q = 0; q < split_number; q++) {
				ofstream ofs("output" + to_string(i) + ".txt");

				prdc_lzw::compress_with_outer_dictionary(
						split_string.at(0).at(p), compressed,
						*dics.at(i).at(q));
				cout << "Source0 compression with dic" << i << ": size = "
						<< compressed.size() << endl;
				total += (int) compressed.size();

				//出力
				for (auto s : compressed) {
					ofs << s << endl;
				}

				compressed.clear();
			}
		}
		cout << "total:" << total << endl;
	}
	cout << endl;

	for (auto dic : dics) {
		for (auto d : dic) {
			delete d;
		}
	}
#endif

	getchar();
	return 0;
}
