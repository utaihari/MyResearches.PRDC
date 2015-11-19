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
	vector<int> compressed;
	prdc_lzw::Dictionary dic;

	prdc_lzw::compress("TOBEORNOTTOBEORTOBEORNOT", compressed, dic);

	for (int i : compressed) {
		cout << i << ",";
	}
	cout << endl;
	getchar();
	return 0;
}
