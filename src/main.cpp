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
	string filenameA = "./data/text/buildings-00.txt";
	string filenameB = "./data/text/forest-00.txt";
	string filenameC = "./data/text/forest-01.txt";
	ifstream ifsA(filenameA);
	ifstream ifsB(filenameB);
	ifstream ifsC(filenameB);
	if (ifsA.fail() || ifsB.fail() || ifsC.fail()) {
		cerr << "読み込みエラー" << endl;
		return -1;
	}
	const string A((istreambuf_iterator<char>(ifsA)),
			istreambuf_iterator<char>());
	const string B((istreambuf_iterator<char>(ifsB)),
			istreambuf_iterator<char>());
	const string C((istreambuf_iterator<char>(ifsC)),
			istreambuf_iterator<char>());

#endif

#if compStr
	cout << "Source A = " << A << endl;
	cout << "Source B = " << B << endl;
	cout << endl;
#else
	cout << "Source A = " << filenameA << endl;
	cout << "Source B = " << filenameB << endl;
	cout << "Source C = " << filenameC << endl;
	cout << endl;
#endif
#if compStr

	prdc_lzw::compress(A, compressed, dic1);
	prdc_lzw::compress_with_outer_dictionary(A, recompressed, dic1);

	cout << "Source A compression with dic1: size = " << compressed.size()
	<< endl;
	for (int i : compressed) {
		cout << i << ",";
	}

	cout << endl;
	cout << "Source A recompression with dic1: size = " << recompressed.size()
	<< endl;
	for (int i : recompressed) {
		cout << i << ",";
	}
	cout << endl;
	cout << endl;

	compressed.clear();
	recompressed.clear();

	prdc_lzw::compress(B, compressed, dic2);
	prdc_lzw::compress_with_outer_dictionary(B, recompressed, dic2);
	cout << "Source B compression with dic2: size = " << compressed.size()
	<< endl;
	for (int i : compressed) {
		cout << i << ",";
	}
	cout << endl;
	cout << "Source B recompression with dic2: size = " << recompressed.size()
	<< endl;
	for (int i : recompressed) {
		cout << i << ",";
	}
	cout << endl;
	recompressed.clear();
	prdc_lzw::compress_with_outer_dictionary(B, recompressed, dic1);

	cout << "Source B recompression with dic1: size = " << recompressed.size()
	<< endl;
	for (int i : recompressed) {
		cout << i << ",";
	}
	cout << endl;
#else

	prdc_lzw::compress(A, compressed, dic1);

	cout << "Source A compression with dic1: size = " << compressed.size()
			<< endl;
	compressed.clear();

	prdc_lzw::compress(B, compressed, dic2);
	cout << "Source B compression with dic2: size = " << compressed.size()
			<< endl;
	compressed.clear();
	cout << endl;

	prdc_lzw::compress_with_outer_dictionary(C, compressed, dic1);
	cout << "Source C compression with dic1: size = " << compressed.size()
			<< endl;
	compressed.clear();
	prdc_lzw::compress_with_outer_dictionary(B, compressed, dic2);

	cout << "Source C compression with dic2: size = " << compressed.size()
			<< endl;
	cout << endl;
#endif

	return 0;
}
