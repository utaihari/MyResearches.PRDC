/*
 * ShowAllNMDDistance.cpp
 *
 *  Created on: 2017/01/13
 *      Author: uchinosub
 */
#include "../TEST.h"
#include <iostream>
#include <string>
#include <tuple>
#include "../util.h"
using namespace std;

int ShowAllNMDDistance(string query_path, string datas_path,
		std::string DATABASE_PATH) {

	image_retrieval::NMD nmd(DATABASE_PATH);
	nmd.SetCodebook(datas_path);

	auto nmd_result = nmd.FindNearest(query_path, -1,
			image_retrieval::ORIGINAL_NMD);
	auto wnmd_result = nmd.FindNearest(query_path, -1,
			image_retrieval::WEIGHTING_NMD);

	cout << "NMD" << endl;
	for (auto r : nmd_result) {
		cout << get<1>(r) << ", " << get<0>(r) << "," << get<2>(r) << "\n";
	}
	cout << endl;

	cout << "WNMD" << endl;
	for (auto r : wnmd_result) {
		cout << get<1>(r) << ", " << get<0>(r) << "," << get<2>(r) << "\n";
	}
	cout << endl;

	return 0;
}

