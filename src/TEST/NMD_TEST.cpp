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

int NMD_TEST(std::string TEXT_PATH, std::string DATABASE_PATH,
		int NUMBER_OF_OUTPUT, int METHOD_FLAG) {

	std::vector<std::string> data_paths;
	std::vector<float> data_classes;
	std::vector<std::string> classes;
	std::map<float, int> each_class_size;
	string filename = "LOG/NMD_LOG/" + CurrentTimeString() + ".txt";
	string table_name = "LOG/NMD_LOG/" + CurrentTimeString() + "-table.txt";
	std::ofstream ofs(filename);
	ofstream table_ofs(table_name);

	prdc_util::GetEachFilePathsAndClasses(TEXT_PATH, data_paths, data_classes,
			classes);

	image_retrieval::NMD nmd(DATABASE_PATH);

	nmd.SetCodebook(TEXT_PATH);

	cout << "nmd_set_end" << endl;

	double accuracy_rate = 0;
	map<float, double> accuracy_rate_each_classes;
	vector<vector<int>> classified_table(classes.size());

	for (float i = 0.0; i < (float) classes.size(); i += 1.0) {
		accuracy_rate_each_classes[i] = 0.0;
		each_class_size[i] = 0.0;
		classified_table.at((int) i) = vector<int>(classes.size(), 0);
	}

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
			classified_table[(int) data_classes.at(i)][(int) response.at(p).first]++;
		}
		double rate = (double) accuracy_count / (double) NUMBER_OF_OUTPUT;
		accuracy_rate += rate;
		accuracy_rate_each_classes[data_classes.at(i)] += rate;

		cout << data_paths.size() << "個中 " << debug << "個目　正解率: " << rate
				<< endl;
		debug++;
	}

	accuracy_rate = (double) accuracy_rate / (double) data_paths.size();

	for (int i = 0; i < (int) classes.size(); ++i) {
		float num = (float) i;
		if (each_class_size[num] == 0)
			continue;
		accuracy_rate_each_classes[num] =
				(double) accuracy_rate_each_classes[num]
						/ (double) each_class_size[num];
	}

	string method_name;

	for (int i = 0; i < (int) METHOD_ARRAY.size(); ++i) {
		if (METHOD_FLAG & METHOD_ARRAY[i]) {
			method_name = METHOD_NAME_ARRAY[i];
		}
	}

	ofs << method_name << endl;
	ofs << TEXT_PATH << endl;
	ofs << NUMBER_OF_OUTPUT << " 個出力" << endl;
	ofs << "平均正解率" << accuracy_rate << endl;

	cout << "平均正解率" << accuracy_rate << endl;

	ofs << "クラスごとの正解率" << endl;
	map<string, double> sorted;
	for (int i = 0; i < (int) classes.size(); ++i) {
		float num = (float) i;
		sorted[classes.at(i)] = accuracy_rate_each_classes[num];
	}
	for (auto m : sorted) {
		ofs << m.first << " : " << m.second << endl;
	}

	//テーブル出力
	table_ofs << method_name << endl;
	table_ofs << TEXT_PATH << endl;
	table_ofs << endl;
	table_ofs << " ";
	for (auto s : classes) {
		table_ofs << s << " ";
	}
	table_ofs << endl;
	for (int i = 0; i < (int) classes.size(); ++i) {
		table_ofs << classes.at(i) << " ";
		for (int p = 0; p < (int) classes.size(); ++p) {
			table_ofs << classified_table[i][p] << " ";
		}
		table_ofs << endl;
	}

	return 0;
}

