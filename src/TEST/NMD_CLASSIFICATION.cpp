/*
 * NMD_CLASSIFICATION.cpp
 *
 *  Created on: 2016/10/09
 *      Author: uchinosub
 */

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

int NMD_CLASSIFICATION(std::string TEXT_PATH, std::string DATABASE_PATH,
		int NUMBER_OF_OUTPUT, int METHOD_FLAG) {

	std::vector<std::string> data_paths;
	std::vector<float> data_classes;
	std::vector<std::string> classes;
	std::map<float, int> each_class_size;
	string filename = "LOG/NMD_CLASSIFICATION_LOG/" + CurrentTimeString()
			+ ".txt";
	std::ofstream ofs(filename);

	prdc_util::GetEachFilePathsAndClasses(TEXT_PATH, data_paths, data_classes,
			classes);

	image_retrieval::NMD nmd(DATABASE_PATH);

	nmd.SetCodebook(TEXT_PATH);

	cout << "nmd_set_end" << endl;

	double accuracy_rate = 0; //正解率
	int accuracy_count = 0; //正解数
	map<float, double> accuracy_rate_each_classes; //それぞれのクラスの正解率
	map<float, int> accuracy_count_each_classes; //それぞれのクラスの正解数

	for (float i = 0.0; i < (float) classes.size(); i += 1.0) {
		accuracy_rate_each_classes[i] = 0.0;
		each_class_size[i] = 0.0;
	}

	int debug = 1;
#pragma omp parallel for
	for (int i = 0; i < (int) data_paths.size(); ++i) {
		each_class_size[data_classes.at(i)]++;

		//データベースの中でクエリと近いものをNUMBER_OF_OUTPUT+1個求める
		std::vector<std::pair<float, std::string>> response = nmd.FindNearest(
				data_paths.at(i), NUMBER_OF_OUTPUT + 1, METHOD_FLAG);

		float response_class = 0.0;
		map<float, int> response_class_count;

		//どのクラスがいくつ選ばれたか投票 (1位は自分自身)
		for (int p = 1; p < NUMBER_OF_OUTPUT + 1; ++p) {
			response_class_count[response.at(p).first]++;

		}

		//出力の中で最も多いクラスを決める　ここから
		int max = 0;
		for (auto r : response_class_count) {
			if (r.second > max) {
				response_class = r.first;
			}
		}
		//出力の中で最も多いクラスを決める　ここまで

		if (data_classes.at(i) == response_class) {
			accuracy_count++;
			accuracy_count_each_classes[data_classes.at(i)]++;
		}

		cout << data_paths.size() << "個中 " << debug << "個目　" << accuracy_count
				<< "個正解, 正解率: " << (double) accuracy_count / (double) debug
				<< endl;
		debug++;
	}

	accuracy_rate = (double) accuracy_count / (double) data_paths.size();

	for (int i = 0; i < (int) classes.size(); ++i) {
		float num = (float) i;
		if (each_class_size[num] == 0)
			continue;
		accuracy_rate_each_classes[num] =
				(double) accuracy_count_each_classes[num]
						/ (double) each_class_size[num];
	}

	string method_name;

	for (int i = 0; i < (int) METHOD_ARRAY.size(); ++i) {
		if (METHOD_FLAG & METHOD_ARRAY[i]) {
			method_name = METHOD_NAME_ARRAY[i];
		}
	}

	ofs << TEXT_PATH << endl;
	ofs << method_name << endl;
	ofs << NUMBER_OF_OUTPUT << " 個出力" << endl;
	ofs << "正解率" << accuracy_rate << endl;

	cout << "正解率" << accuracy_rate << endl;

	ofs << "クラスごとの正解率" << endl;
	map<string, double> sorted;
	for (int i = 0; i < (int) classes.size(); ++i) {
		float num = (float) i;
		sorted[classes.at(i)] = accuracy_rate_each_classes[num];
	}
	for (auto m : sorted) {
		ofs << m.first << " : " << m.second << endl;
	}

	return 0;
}

