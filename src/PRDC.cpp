/*
 * PRDC.cpp
 *
 *  Created on: 2016/07/07
 *      Author: uchinosub
 */

#include "PRDC.h"
#include <iostream>
#include <fstream>
#include <algorithm>    // std::copy
#include <iterator>     // std::back_inserter
//#include <omp.h>

namespace prdc {

bool PRDC::train(const std::vector<std::string>& training_data_paths,
		const std::vector<float>& data_class) {

	if (training_data_paths.size() != data_class.size()) {
		std::cout << "エラー：データとクラスの数が違います." << std::endl;
		return false;
	}

	//念の為data_classをコピー
	copy(data_class.begin(), data_class.end(), back_inserter(classes));

	vectors.resize(training_data_paths.size());

	int debug = 1;
	//それぞれのテキストに対しての処理
#pragma omp parallel for
	for (int i = 0; i < (int) training_data_paths.size(); ++i) {
		std::string text;
		file_read(training_data_paths.at(i), text);

		//現在のテキストの圧縮率ベクトル
		std::vector<float> vec = make_compless_vector(text);

		if (debug % 10 == 0) {
			std::cout << training_data_paths.size() << "個中" << debug << "個完了"
					<< std::endl;
		}
		debug++;
		//全てのベクトルが完成した後mat型に変換します（ここでは纏めておくだけ）
		vectors.at(i) = std::vector<float>(vec);
	}

	//データをMat型に変換する

	//Mat(全てのデータ数,次元数,CV_32FC1);
	training_mat.create(training_data_paths.size(), base_dics.size(), CV_32FC1);
	classes_mat.create(1, data_class.size(), CV_32FC1);

	for (int i = 0; i < (int) training_data_paths.size(); ++i) {
		for (int j = 0; j < (int) base_dics.size(); ++j) {
			training_mat.at<float>(i, j) = vectors[i][j];
		}
		classes_mat.at<float>(0, i) = classes.at(i);
	}
	knn.train(training_mat, classes_mat);

	return true;
}

PRDC::PRDC(const std::vector<std::string>& bases) :
		base_dics_names(bases) {
	for (auto b : bases) {
		std::string text;
		file_read(b, text);
		base_dics.push_back(prdc_lzw::Dictionary(text));
	}
}

PRDC::~PRDC() {
// TODO Auto-generated destructor stub
}

float PRDC::find_nearest(const std::string file_name, int k) const {

	std::string text;
	file_read(file_name, text);

	std::vector<float> vec = make_compless_vector(text);
	cv::Mat sample(1, vec.size(), CV_32FC1);
	for (int i = 0; i < (int) vec.size(); ++i) {
		sample.at<float>(i) = vec[i];
	}
	return knn.find_nearest(sample, k);
}

PRDC::PRDC(const std::string train_file) {
	cv::FileStorage cvfs(train_file, CV_STORAGE_READ);
	cvfs["vectors"] >> training_mat;
	cvfs["classes"] >> classes_mat;
	cvfs["base_dics"] >> base_dics_names;
	cvfs.release();

	for (auto b : base_dics_names) {
		std::string text;
		file_read(b, text);
		base_dics.push_back(prdc_lzw::Dictionary(text));
	}

	knn.train(training_mat, classes_mat);
}

std::vector<float> PRDC::make_compless_vector(const std::string text) const {
	std::vector<float> vec;

	//基底辞書それぞれで圧縮し、圧縮率をベクトルにする
	for (auto dic : base_dics) {
		std::vector<int> compressed = dic.Compress(text);
		vec.push_back((float) compressed.size() / (float) text.length());
	}
	return vec;
}

bool PRDC::save(const std::string filename) {
	cv::FileStorage cvfs(filename, CV_STORAGE_WRITE);
	cv::write(cvfs, "vectors", training_mat);
	cv::write(cvfs, "classes", classes_mat);
	cv::write(cvfs, "base_dics", base_dics_names);
	cvfs.release();
	return true;
}

void PRDC::file_read(std::string path, std::string& output) const {
	std::ifstream ifs(path);
	if (ifs.fail()) {
		std::cerr << "読み込みエラー:" << path << std::endl;
	}
	output = std::string((std::istreambuf_iterator<char>(ifs)),
			std::istreambuf_iterator<char>());
	ifs.close();
}

} /* namespace histgram_map */
