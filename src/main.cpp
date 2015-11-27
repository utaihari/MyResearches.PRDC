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
#include "Lzw.h"
#include "Dictionary.h"

#define compStr 0

using namespace std;
int main() {

	vector<int> compressed, recompressed;
	prdc_lzw::Dictionary dic1, dic2;

#if compStr

	const string A = "TOBEORNOTTOBEORTOBEORNOT";
	const string B = "TOUKYOUTOKKYOKYOKAKYOKU";

#else

	vector<string> filename;
	vector<string> file_contents;
	vector<ifstream> ifs;
	vector<prdc_lzw::Dictionary*> dics;

	filename.push_back("./data/text/buildings-00.txt");
	filename.push_back("./data/text/buildings-10.txt");
	filename.push_back("./data/text/denseresidential-20.txt");
	filename.push_back("./data/text/forest-30.txt");
	filename.push_back("./data/text/intersection-40.txt");
	filename.push_back("./data/text/river-50.txt");

	for (auto file : filename) {
		ifstream ifs(file);
		if (ifs.fail()) {
			cerr << "読み込みエラー" << endl;
		}
		file_contents.push_back(
				string((istreambuf_iterator<char>(ifs)),
						istreambuf_iterator<char>()));
	}

#endif

#if compStr
	cout << "String A = " << A << endl;
	cout << "String B = " << B << endl;
	cout << endl;
	cout << endl;
#else
	for (int i = 0; i < (int) filename.size(); i++) {
		cout << "Source" << i << " = \"" << filename.at(i) <<"\""<< endl;
	}
	cout << endl;
#endif
#if compStr

	prdc_lzw::compress(A, compressed, dic1);
	prdc_lzw::compress_with_outer_dictionary(A, recompressed, dic1);

	cout << "String A compression for extract dic1: size = "
			<< compressed.size() << endl;
	for (int i : compressed) {
		cout << i << ",";
	}
	cout << endl;
	cout << endl;
	cout << "String A compression with dic1: size = " << recompressed.size()
			<< endl;
	for (int i : recompressed) {
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
	for (int i : compressed) {
		cout << i << ",";
	}
	cout << endl;
	cout << endl;

	cout << "String B compression with dic2: size = " << recompressed.size()
			<< endl;
	for (int i : recompressed) {
		cout << i << ",";
	}
	cout << endl;
	cout << endl;

	recompressed.clear();
	prdc_lzw::compress_with_outer_dictionary(B, recompressed, dic1);

	cout << "String B compression with dic1: size = " << recompressed.size()
			<< endl;
	for (int i : recompressed) {
		cout << i << ",";
	}
	cout << endl;
#else

	//辞書作成
	for (int i = 0; i < (int) file_contents.size(); i++) {
		prdc_lzw::Dictionary* tempDic = new prdc_lzw::Dictionary();
		dics.push_back(tempDic);
		prdc_lzw::compress(file_contents.at(i), compressed, *dics.at(i));
		cout << "Source" << i << " compression for extract dic" << i
		<< ": size = " << compressed.size() << endl;

		string filename = "output" + to_string(i) + ".txt";
		ofstream outputfile(filename);
		for (auto c : compressed) {
			outputfile << c << ",";
		}
		outputfile.close();

		compressed.clear();
	}

	cout << endl;
	//比較
	for (int i = 0; i < (int) file_contents.size(); i++) {
		prdc_lzw::compress_with_outer_dictionary(file_contents.at(0),
				compressed, *dics.at(i));
		cout << "Source0 compression with dic" << i << ": size = "
		<< compressed.size() << endl;
		compressed.clear();
	}
	cout << endl;

	for(auto d:dics) {
		delete d;
	}
#endif



	return 0;
}
