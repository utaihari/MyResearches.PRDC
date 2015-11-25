/**
 *  @file main.cpp
 *
 *  @date 2015/11/19
 *  @author uchinosub
 */
#include <iostream>
#include <vector>
#include <stdio.h>
#include "Lzw.h"
#include "Dictionary.h"

using namespace std;
int main() {
	vector<int> compressed, recompressed;
	prdc_lzw::Dictionary dic1, dic2;

	const string A = "TOBEORNOTTOBEORTOBEORNOT";
	const string B = "TOUKYOUTOKKYOKYOKAKYOKU";

	cout << "String A = " << A << endl;
	cout << "String B = " << B << endl;
	cout << endl;

	prdc_lzw::compress(A, compressed, dic1);
	prdc_lzw::compress_with_outer_dictionary(A, recompressed, dic1);

	cout << "String A compression with dic1" << endl;
	for (int i : compressed) {
		cout << i << ",";
	}
	cout << endl;
	cout << "String A recompression with dic1" << endl;
	for (int i : recompressed) {
		cout << i << ",";
	}
	cout << endl;
	cout << endl;

	compressed.clear();
	recompressed.clear();

	prdc_lzw::compress(B, compressed, dic2);
	prdc_lzw::compress_with_outer_dictionary(B, recompressed, dic2);
	cout << "String B compression with dic2" << endl;
	for (int i : compressed) {
		cout << i << ",";
	}
	cout << endl;
	cout << "String B recompression with dic2" << endl;
	for (int i : recompressed) {
		cout << i << ",";
	}

	cout << endl;
	recompressed.clear();
	prdc_lzw::compress_with_outer_dictionary(B, recompressed, dic1);

	cout << "String B recompression with dic1" << endl;
	for (int i : recompressed) {
		cout << i << ",";
	}

	cout << endl;

	getchar();
	return 0;
}
