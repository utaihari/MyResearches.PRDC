/*
 * NMD_TEST.cpp
 *
 *  Created on: 2016/08/03
 *      Author: uchinosub
 */
#include "../TEST.h"
#include "../util.h"
#include <iostream>
#include <fstream>
#include <map>

using namespace std;
using namespace prdc_util;
using namespace image_retrieval;

int NMD_Create_PR_Graph(std::string TEXT_PATH, std::string DATABASE_PATH,
		int MAX_OUTPUT, int METHOD_FLAG) {

	std::vector<std::string> data_paths;
	std::vector<double> data_classes;
	std::vector<std::string> classes;
	std::map<double, int> each_class_size;

	string method_name;
	for (int i = 0; i < (int) METHOD_ARRAY.size(); ++i) {
		if (METHOD_FLAG & METHOD_ARRAY[i]) {
			method_name = METHOD_NAME_ARRAY[i];
		}
	}

	string filename = "LOG/NMD_LOG/" + method_name + "-" + CurrentTimeString()
			+ "-PR.txt";
	std::ofstream ofs(filename);

	prdc_util::GetEachFilePathsAndClasses(TEXT_PATH, data_paths, data_classes,
			classes);

	image_retrieval::NMD nmd(DATABASE_PATH);

	nmd.SetCodebook(TEXT_PATH);

	cout << "nmd_set_end" << endl;

	for (double i = 0.0; i < (double) classes.size(); i += 1.0) {
		each_class_size[i] = 0.0;
	}
	vector<vector<std::tuple<double, std::string, double>>> response(data_paths.size());

	cout << "類似検索開始" << endl;
#ifdef PARALLEL
#pragma omp parallel for
#endif
	for (int i = 0; i < (int) data_paths.size(); ++i) {
		each_class_size[data_classes.at(i)]++;
		response.at(i) = nmd.FindNearest(data_paths.at(i), MAX_OUTPUT,
				METHOD_FLAG);
	}
	cout << "検索終了" << endl;

	vector<double> precisions(MAX_OUTPUT - 1);
	vector<double> recalls(MAX_OUTPUT - 1);

	for (int output = 1; output < MAX_OUTPUT; ++output) {
		double precision_rate = 0.0;
		double recall_rate = 0.0;
		for (int i = 0; i < (int) data_paths.size(); ++i) {
			//output枚出力した時の平均適合率と平均再現率
			int accuracy_count = 0;
			for (int q = 0; q < output; ++q) {
				if (data_classes.at(i) == get<0>(response.at(i).at(q))) {
					accuracy_count++;
				}
			}
			double precision = (double) accuracy_count / (double) output;
			precision_rate += precision;
			double recall = (double) accuracy_count
					/ (double) each_class_size[data_classes.at(i)];
			recall_rate += recall;
		}
		precisions.at(output - 1) = (double) precision_rate
				/ (double) data_paths.size();
		recalls.at(output - 1) = (double) recall_rate
				/ (double) data_paths.size();
	}

//	ofs << method_name << endl;
//	ofs << TEXT_PATH << endl;
//	ofs << MAX_OUTPUT << " 個出力" << endl;

	cout << "平均適合率、再現率算出終了" << endl;
	for (int i = 0; i < MAX_OUTPUT - 1; ++i) {
		ofs << recalls.at(i) << " " << precisions.at(i) << endl;
	}

	return 0;
}

