/*
 * NMD_TEST.cpp
 *
 *  Created on: 2016/08/03
 *      Author: uchinosub
 */
#include "TEST.h"
#include "util.h"
#include <iostream>
#include <fstream>
#include <map>

using namespace std;
using namespace prdc_util;
using namespace image_retrieval;

int NMD_TEST(string FOLDER_PATH, int NUMBER_OF_OUTPUT, int METHOD_FLAG) {

	std::vector<std::string> data_paths;
	std::vector<float> data_classes;
	std::map<std::string, float> classes;
	std::map<float, int> each_class_size;
	string filename = "NMD_LOG/" + CurrentTimeString() + ".txt";
	std::ofstream ofs(filename);

	prdc_util::GetEachFilePathsAndClasses(FOLDER_PATH, data_paths, data_classes,
			classes);

	image_retrieval::NMD nmd(data_paths, data_classes);
	cout << "nmd_set_end" << endl;

	double accuracy_rate = 0;
	map<float, double> accuracy_rate_each_classes;

	int debug = 1;
#pragma omp parallel for
	for (int i = 0; i < (int) data_paths.size(); ++i) {
		each_class_size[data_classes.at(i)]++;

		std::vector<std::pair<float, std::string>> response = nmd.FindNearest(
				data_paths.at(i), NUMBER_OF_OUTPUT, METHOD_FLAG);

		int accuracy_count = 0;

		for (int p = 0; p < NUMBER_OF_OUTPUT; ++p) {

			if (data_classes.at(i) == response.at(p).first) {
				accuracy_count++;
			}
		}

		double rate = (double) accuracy_count / (double) NUMBER_OF_OUTPUT;
		accuracy_rate += rate;
		accuracy_rate_each_classes[data_classes.at(i)] += rate;

		cout << data_paths.size() << "個中 " << debug << "個目　正解率: " << rate
				<< endl;
		debug++;
	}

	accuracy_rate = (double) accuracy_rate / (double) data_paths.size();

	for (auto c : classes) {
		if (each_class_size[c.second] == 0)
			continue;
		accuracy_rate_each_classes[c.second] =
				(double) accuracy_rate_each_classes[c.second]
						/ (double) each_class_size[c.second];
	}

	string method_name;

	for (int i = 0; i < (int) METHOD_ARRAY.size(); ++i) {
		if (METHOD_FLAG & METHOD_ARRAY[i]) {
			method_name = METHOD_NAME_ARRAY[i];
		}
	}

	ofs << method_name << endl;
	ofs << NUMBER_OF_OUTPUT << " 個出力" << endl;
	ofs << "平均正解率" << accuracy_rate << endl;

	cout << "平均正解率" << accuracy_rate << endl;

	ofs << "クラスごとの平均正解率" << endl;
	for (auto c : classes) {
		ofs << c.first << " : " << accuracy_rate_each_classes[c.second] << endl;

	}

	return 0;
}

