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
#include <tuple>
#include "../Debug.h"
using namespace std;
using namespace prdc_util;
using namespace image_retrieval;

int NMD_CLASSIFICATION(std::string TEXT_PATH, std::string DATABASE_PATH,
		int NUMBER_OF_OUTPUT, int METHOD_FLAG) {

	std::vector<std::string> data_paths;
	std::vector<double> data_classes;
	std::vector<std::string> classes;
	std::map<double, int> each_class_size;
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
	map<double, double> accuracy_rate_each_classes; //それぞれのクラスの正解率
	map<double, int> accuracy_count_each_classes; //それぞれのクラスの正解数

	for (double i = 0.0; i < (double) classes.size(); i += 1.0) {
		accuracy_rate_each_classes[i] = 0.0;
		each_class_size[i] = 0.0;
	}

	int debug = 1;
#ifdef PARALLEL
#pragma omp parallel for
#endif
	for (int i = 0; i < (int) data_paths.size(); ++i) {
		each_class_size[data_classes.at(i)]++;

		//データベースの中でクエリと近いものをNUMBER_OF_OUTPUT+1個求める
		auto response = nmd.FindNearest(
				data_paths.at(i), NUMBER_OF_OUTPUT + 1, METHOD_FLAG);

		double response_class = 0.0;
		map<double, int> response_class_count;

		//どのクラスがいくつ選ばれたか投票 (1位は自分自身)
		for (int p = 1; p < NUMBER_OF_OUTPUT + 1; ++p) {
			response_class_count[get<0>(response.at(p))]++;

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
		double num = (double) i;
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
		double num = (double) i;
		sorted[classes.at(i)] = accuracy_rate_each_classes[num];
	}
	for (auto m : sorted) {
		ofs << m.first << " : " << m.second << endl;
	}

	return 0;
}

int NMD_CLASSIFICATION(std::string TRAIN_DATA_PATH, std::string TEST_DATA_PATH,
		std::string DATABASE_PATH, int METHOD_FLAG, string savefile_name,
		int k) {

	string method_name;

	for (int i = 0; i < (int) METHOD_ARRAY.size(); ++i) {
		if (METHOD_FLAG & METHOD_ARRAY[i]) {
			method_name = METHOD_NAME_ARRAY[i];
		}
	}
	std::map<double, int> each_class_size;
	string filename = "LOG/NMD_CLASSIFICATION_LOG/"  + savefile_name + "_"
			+ method_name + "_" + CurrentTimeString() + ".txt";
	std::ofstream ofs(filename);

	//ファイルの読み込み
	prdc_util::IncludedFilePaths train_files(TRAIN_DATA_PATH, { ".txt" });
	prdc_util::IncludedFilePaths test_files(TEST_DATA_PATH, { ".txt" });

	image_retrieval::NMD nmd(DATABASE_PATH);

	nmd.SetCodebook(TRAIN_DATA_PATH);
#ifdef DEBUG_OUTPUT
	cout << "nmd_set_end" << endl;
#endif

	double accuracy_rate = 0; //正解率
	int accuracy_count = 0; //正解数
	map<double, double> accuracy_rate_each_classes; //それぞれのクラスの正解率
	map<double, int> accuracy_count_each_classes; //それぞれのクラスの正解数

	for (double i = 0.0; i < (double) test_files.classes.size(); i += 1.0) {
		accuracy_rate_each_classes[i] = 0.0;
		each_class_size[i] = 0.0;
	}

	int debug = 1;
#ifdef PARALLEL
#pragma omp parallel for
#endif
	for (int i = 0; i < (int) test_files.included_file_paths.size(); ++i) {
		each_class_size[test_files.included_file_classes.at(i)]++;

		//データベースの中でクエリと近いものをNUMBER_OF_OUTPUT個求める
		auto response = nmd.FindNearest(
				test_files.included_file_paths.at(i), k, METHOD_FLAG);

		double response_class = 0.0;
		map<double, int> response_class_count;

		//どのクラスがいくつ選ばれたか投票
		for (int p = 0; p < k; ++p) {
			response_class_count[get<0>(response.at(p))]++;

		}

		//出力の中で最も多いクラスを決める　ここから
		int max = 0;
		for (auto r : response_class_count) {
			if (r.second > max) {
				response_class = r.first;
				max = r.second;
			}
		}
		//出力の中で最も多いクラスを決める　ここまで

		if (test_files.included_file_classes.at(i) == response_class) {
			accuracy_count++;
			accuracy_count_each_classes[test_files.included_file_classes.at(i)]++;
		}
#ifdef DEBUG_OUTPUT
		cout << test_files.included_file_paths.size() << "個中 " << debug << "個目　"
				<< accuracy_count << "個正解, 正解率: "
				<< (double) accuracy_count / (double) debug << endl;
#endif
		debug++;
	}

	accuracy_rate = (double) accuracy_count
			/ (double) test_files.included_file_paths.size();

	for (int i = 0; i < (int) test_files.included_file_classes.size(); ++i) {
		double num = (double) i;
		if (each_class_size[num] == 0)
			continue;
		accuracy_rate_each_classes[num] =
				(double) accuracy_count_each_classes[num]
						/ (double) each_class_size[num];
	}

	ofs << TRAIN_DATA_PATH << endl;
	ofs << method_name << endl;
	ofs << k << " 個出力" << endl;
	ofs << "正解率" << accuracy_rate << endl;

	cout << method_name << ": " << accuracy_rate << flush;

	ofs << "クラスごとの正解率" << endl;
	map<string, double> sorted;
	for (int i = 0; i < (int) test_files.classes.size(); ++i) {
		double num = (double) i;
		sorted[test_files.classes.at(i)] = accuracy_rate_each_classes[num];
	}
	for (auto m : sorted) {
		ofs << m.first << " : " << m.second << endl;
	}

	return 0;
}
