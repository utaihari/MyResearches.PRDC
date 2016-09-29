/*
 * PRDC.cpp
 *
 *  Created on: 2016/07/07
 *      Author: uchinosub
 */

#include "PRDC.h"
#include "util.h"
#include <iostream>
#include <fstream>
#include <algorithm>    // std::copy
#include <iterator>     // std::back_inserter
#include <math.h> //log
//#include <omp.h>

using std::cout;
using std::endl;

namespace prdc {
int ofs_count = 0;

std::vector<std::vector<std::vector<float>>>PRDC::PRDC_LAST_LEARNING(METHOD_ARRAY.size());
const PRDC::MakeVecPtr PRDC::make_vector_functions[] = {
		&PRDC::make_compless_vector, &PRDC::make_pair_multiple_vector,
		&PRDC::make_pair_multiple_log_vector, &PRDC::make_recompression_vector,
		&PRDC::make_self_compression_vector,
		&PRDC::make_pair_and_selfcompression_vector };

bool PRDC::train(std::vector<std::string>& training_data_paths,
		std::vector<float>& data_class, bool encoded) {

	if (training_data_paths.size() != data_class.size()) {
		std::cout << "エラー：データとクラスの数が違います." << std::endl;
		return false;
	}

	//念の為data_classをコピー
	copy(data_class.begin(), data_class.end(), back_inserter(classes));

	vectors.resize(training_data_paths.size());

	//次の圧縮に今回の結果を用いる場合の準備（同じ学習画像で違う手法を試す場合）
	for (int i = 0; i < (int) METHOD_ARRAY.size(); ++i) {
		if ((ready_for_next | prdc_flag) & METHOD_ARRAY.at(i)) {
			PRDC_LAST_LEARNING.at(i).resize(training_data_paths.size());
		}
	}

	int debug = 1;
	//それぞれのテキストに対しての処理
#pragma omp parallel for
	for (int i = 0; i < (int) training_data_paths.size(); ++i) {
		std::string text;
		if (encoded) {
			text = training_data_paths.at(i);
		} else {
			prdc_util::FilePathToString(training_data_paths.at(i), text);
		}
		std::vector<float> vec; //PRDCに使用するベクトル

		//ベクトル作成ここから
		vec = make_vector(text, i);

		//ベクトル作成ここまで

		if (debug % 10 == 0) {
			std::cout << training_data_paths.size() << "個中" << debug << "個完了"
					<< std::endl;
		}
		//全てのベクトルが完成した後mat型に変換します（ここでは纏めておくだけ）
		vectors.at(i) = std::vector<float>(vec);

		debug++;
	}

	//データをMat型に変換する

	//Mat(全てのデータ数,次元数,CV_32FC1);
	training_mat.create(training_data_paths.size(), vectors.at(0).size(),
	CV_32FC1);
	classes_mat.create(1, data_class.size(), CV_32FC1);

	for (int i = 0; i < (int) training_data_paths.size(); ++i) {
		for (int j = 0; j < (int) vectors.at(0).size(); ++j) {
			training_mat.at<float>(i, j) = vectors[i][j];
		}
		classes_mat.at<float>(0, i) = classes.at(i);
	}
	knn.train(training_mat, classes_mat);

	return true;
}

PRDC::PRDC(std::vector<std::string>& bases, int flag, int READY_FOR_NEXT,
		bool USE_LAST_DATA, bool MULTI_BYTE_CHAR, bool encoded) :
		base_dics_names(bases), prdc_flag(flag), ready_for_next(READY_FOR_NEXT), use_last_data(
				USE_LAST_DATA), multi_byte_char(MULTI_BYTE_CHAR), encoded(
				encoded) {

	if (use_last_data == false) {
		//METHOD_ARRAYと同じ順番にするように気をつける
		for (int i = 0; i < (int) METHOD_ARRAY.size(); ++i) {
			if ((ready_for_next | prdc_flag) & METHOD_ARRAY.at(i)) {
				PRDC_LAST_LEARNING.at(i).clear();
				PRDC_LAST_LEARNING.at(i).shrink_to_fit();
			}
		}
	}

	for (auto b : bases) {
		std::string text;
		if (encoded) {
			text = b;
		} else {
			file_read(b, text);
		}
		base_dics.push_back(prdc_lzw::Dictionary(text, multi_byte_char));
	}
}

PRDC::PRDC(std::vector<std::string>& bases,
		std::vector<std::string>& bases_path, int flag, int READY_FOR_NEXT,
		bool USE_LAST_DATA, bool MULTI_BYTE_CHAR, bool encoded) :
		base_dics_names(bases_path), prdc_flag(flag), ready_for_next(
				READY_FOR_NEXT), use_last_data(USE_LAST_DATA), multi_byte_char(
				MULTI_BYTE_CHAR), encoded(encoded) {

	if (use_last_data == false) {
		//METHOD_ARRAYと同じ順番にするように気をつける
		for (int i = 0; i < (int) METHOD_ARRAY.size(); ++i) {
			if ((ready_for_next | prdc_flag) & METHOD_ARRAY.at(i)) {
				PRDC_LAST_LEARNING.at(i).clear();
				PRDC_LAST_LEARNING.at(i).shrink_to_fit();
			}
		}
	}

	for (auto b : bases) {
		std::string text;
		if (encoded) {
			text = b;
		} else {
			file_read(b, text);
		}
		base_dics.push_back(prdc_lzw::Dictionary(text, multi_byte_char));
	}
}

PRDC::~PRDC() {
// TODO Auto-generated destructor stub
}

float PRDC::find_nearest(std::string file_name, int k) {
	std::string text;

	file_read(file_name, text);

	//ベクトル作成ここから
	std::vector<float> vec = make_vector(text); //PRDCに使用するベクトル
	//ベクトル作成ここまで

	cv::Mat sample(1, vec.size(), CV_32FC1);
	for (int i = 0; i < (int) vec.size(); ++i) {
		sample.at<float>(i) = vec[i];
	}
	return knn.find_nearest(sample, k);
}

PRDC::PRDC(std::string train_file) :
		base_dics_names(), prdc_flag(), ready_for_next(), use_last_data(), multi_byte_char(
				false), encoded(false) {
	cv::FileStorage cvfs(train_file, CV_STORAGE_READ);
	cvfs["vectors"] >> training_mat;
	cvfs["classes"] >> classes_mat;
	cvfs["base_dics"] >> base_dics_names;
	cvfs["prdc_flag"] >> prdc_flag;
	cvfs["use_multibyte"] >> multi_byte_char;
	cvfs.release();

	for (auto b : base_dics_names) {
		std::string text;
		if (encoded) {
			text = b;
		} else {
			file_read(b, text);
		}
		base_dics.push_back(prdc_lzw::Dictionary(text, multi_byte_char));
	}

	vectors.resize(training_mat.rows);
	for(auto& v:vectors){
		v.resize(training_mat.cols);
	}

	for (int i = 0; i < (int) training_mat.rows; ++i) {
		for (int j = 0; j < (int) training_mat.cols; ++j) {
			 vectors[i][j] = training_mat.at<float>(i, j);
		}
//		classes.at(i) = classes_mat.at<float>(0, i);
	}

//	knn.train(training_mat, classes_mat);
}
bool PRDC::save(std::string filename) {
	cv::FileStorage cvfs(filename, CV_STORAGE_WRITE);
	cv::write(cvfs, "vectors", training_mat);
	cv::write(cvfs, "classes", classes_mat);
	cv::write(cvfs, "base_dics", base_dics_names);
	cv::write(cvfs, "prdc_flag", prdc_flag);
	cv::write(cvfs, "use_multibyte", multi_byte_char);
	cvfs.release();
	return true;
}

std::vector<float> PRDC::make_compless_vector(std::string& text,
		std::vector<prdc_lzw::EncodedText>& comp) const {
	std::vector<float> vec(base_dics.size());

	//基底辞書それぞれで圧縮し、圧縮率をベクトルにする
	for (int i = 0; i < (int) base_dics.size(); ++i) {
		auto& compressed = comp.at(i);
		vec.at(i) = ((float) compressed.size() / (float) text.length());
	}
	return vec;
}

std::vector<float> PRDC::make_pair_multiple_vector(std::string& text,
		std::vector<prdc_lzw::EncodedText>& comp) const {

	std::vector<float> vec(base_dics.size() * 2);

	int vec_index = 0;	//圧縮率ベクトルのインデックス

	//基底辞書それぞれで圧縮し、圧縮率をベクトルにする
	for (int i = 0; i < (int) base_dics.size(); ++i) {
		auto& dic = base_dics.at(i);
		auto& compressed = comp.at(i);
		std::vector<std::pair<int, int>> compressed_pair = make_pair(
				compressed);
		std::map<std::pair<int, int>, int> pair_histgram = make_pair_histgram(
				compressed_pair);

		int reduction = 0; //削減文字数
		for (auto pair : pair_histgram) {
			int count = pair.second; //ペアの頻度 (必ず1以上の値になる)
			int pair_string_length; //ペアの合計文字数

			pair_string_length = (int) dic.contents[pair.first.first].size()
					+ (int) dic.contents[pair.first.second].size();
			reduction += (count - 1) * (pair_string_length - 2);
		}

		vec.at(vec_index) = (float) compressed.length() / (float) text.length();
		vec.at(vec_index + 1) = 1.0
				- (float) reduction / (2.0 * (float) text.length());

		vec_index += 2;
	}
	return vec;
}
std::vector<float> PRDC::make_pair_multiple_log_vector(std::string& text,
		std::vector<prdc_lzw::EncodedText>& comp) const {
	std::vector<float> vec(base_dics.size() * 2);

	int vec_index = 0; //圧縮率ベクトルのインデックス

//基底辞書それぞれで圧縮し、圧縮率をベクトルにする
	for (int i = 0; i < (int) base_dics.size(); ++i) {
		auto& dic = base_dics.at(i);
		auto& compressed = comp.at(i);

		std::vector<std::pair<int, int>> compressed_pair = make_pair(
				compressed);
		std::map<std::pair<int, int>, int> pair_histgram = make_pair_histgram(
				compressed_pair);

		double reduction = 0.0; //削減文字数
		for (auto pair : pair_histgram) {
			int count = pair.second; //ペアの頻度
			int pair_string_length; //ペアの合計文字数

			pair_string_length = (int) dic.contents[pair.first.first].length()
					+ (int) dic.contents[pair.first.second].length() - 2;

			reduction += log2(count) * (double) pair_string_length;
		}

		vec.at(vec_index) = (float) compressed.size() / (float) text.length();
		vec.at(vec_index + 1) = 1.0
				- ((float) reduction / (float) text.length());
		vec_index += 2;
	}
	return vec;
}

std::vector<float> PRDC::make_recompression_vector(std::string& text,
		std::vector<prdc_lzw::EncodedText>& comp) const {

	std::vector<float> vec(base_dics.size() * 2);
	int vec_index = 0; //圧縮率ベクトルのインデックス

	//基底辞書それぞれで圧縮し、圧縮率をベクトルにする
	for (int i = 0; i < (int) base_dics.size(); ++i) {
		auto& compressed = comp.at(i);

		//通常の圧縮率ベクトル作成
		vec.at(vec_index) = (float) compressed.size() / (float) text.length();

		//再圧縮した圧縮率ベクトル作成

		prdc_lzw::EncodedText recompress;
		recompress = compress(compressed); //2分木を用いた圧縮(わずかに処理速度早い)
		float comp_size = (float) recompress.size() / (float) compressed.size();

//		prdc_lzw::Dictionary recompress(compressed);//トライ木を用いた圧縮(機能豊富)
//		float comp_size = (float) recompress.compressed.size()
//				/ (float) compressed.size();

		vec.at(vec_index + 1) = comp_size;
		vec_index += 2;
	}
	return vec;
}

std::map<std::pair<int, int>, int> PRDC::make_pair_histgram(
		std::vector<std::pair<int, int>>& pair) const {
	std::map<std::pair<int, int>, int> pair_histgram;

	for (auto p : pair) {
		pair_histgram[p]++;
	}

	return pair_histgram;
}

std::vector<std::pair<int, int>> PRDC::make_pair(
		prdc_lzw::EncodedText& compressed) const {
	std::vector<std::pair<int, int>> output(compressed.size() - 1);

	int prev = 0;
	int next = 0;

	for (int i = 0; i < (int) compressed.encoded.size() - 1; ++i) {
		prev = compressed.encoded.at(i);
		next = compressed.encoded.at(i + 1);
		output.at(i) = std::make_pair(prev, next);
	}
	std::sort(output.begin(), output.end());
	return output;
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

std::vector<float> PRDC::make_vector(std::string& text, int vec_num) {
	std::vector<float> vec; //PRDCに使用するベクトル
	std::vector<prdc_lzw::EncodedText> compressed(base_dics.size());

	if (vec_num >= 0) {
		//結果を記録する場合（連続したベクトルを作成する場合）

		if (use_last_data == false) {
			//前回の結果を使わない場合
			for (int p = 0; p < (int) base_dics.size(); ++p) {
				//それぞれの基底辞書で圧縮を行う
				compressed.at(p) = base_dics.at(p).Compress(text);
			}

			for (int i = 0; i < (int) METHOD_ARRAY.size(); ++i) {
				//それぞれの手法について調べる

				if ((ready_for_next | prdc_flag) & METHOD_ARRAY.at(i)) {
					//もしもその手法を今回（次回）の圧縮に利用するなら

					//その手法を用いて作ったベクトルを保存する
					PRDC_LAST_LEARNING.at(i).at(vec_num) = (this
							->*make_vector_functions[i])(text, compressed);
				}
			}
		}
		for (int i = 0; i < (int) METHOD_ARRAY.size(); ++i) {
			//それぞれの手法について調べる

			if (prdc_flag & METHOD_ARRAY.at(i)) {
				//もしもその手法を今回の圧縮に利用するなら
				vec = PRDC_LAST_LEARNING.at(i).at(vec_num);
			}

		}
	} else {
		//結果を記録しない場合（独立したベクトルを作成する場合）
		for (int p = 0; p < (int) base_dics.size(); ++p) {
			//それぞれの基底辞書で圧縮を行う
			compressed.at(p) = base_dics.at(p).Compress(text);
		}
		for (int flag = 0; flag < (int) METHOD_ARRAY.size(); ++flag) {
			if (prdc_flag & METHOD_ARRAY.at(flag)) {
				vec = (this->*make_vector_functions[flag])(text, compressed);
			}

		}
	}

	return vec;

}

std::vector<float> PRDC::make_self_compression_vector(std::string& text,
		std::vector<prdc_lzw::EncodedText>& comp) const {

	std::vector<float> vec(base_dics.size() + 1);

//基底辞書それぞれで圧縮し、圧縮率をベクトルにする
	for (int i = 0; i < (int) base_dics.size(); ++i) {
		auto& compressed = comp.at(i);
		//通常の圧縮率ベクトル作成
		vec.at(i) = (float) compressed.size() / (float) text.length();
	}

	prdc_lzw::Dictionary self_compress(text);

	vec.at(vec.size() - 1) = (float) self_compress.compressed.length()
			/ (float) text.length();

	return vec;
}

std::vector<float> PRDC::make_pair_and_selfcompression_vector(std::string& text,
		std::vector<prdc_lzw::EncodedText>& compressed) const {
	std::vector<float> vec = make_pair_multiple_vector(text, compressed);

	prdc_lzw::Dictionary self_compress(text);
	vec.push_back(
			(float) self_compress.compressed.length() / (float) text.length());

	return vec;
}

std::vector<int> PRDC::find_near_vector(std::string input_image_path,
		int output_num, bool encoded) {
	std::string text;
	if (encoded) {
		text = input_image_path;
	} else {
		file_read(input_image_path, text);
	}

	//ベクトル作成ここから
	std::vector<float> vec = make_vector(text); //PRDCに使用するベクトル
	//ベクトル作成ここまで

	cv::Mat sample(1, vec.size(), CV_32FC1);
	for (int i = 0; i < (int) vec.size(); ++i) {
		sample.at<float>(i) = vec[i];
	}
	std::vector<float> norms;
	calc_all_vector_norm(vec, norms);
	std::vector<int_float_pair> ifp(norms.size());
	for (int i = 0; i < (int) norms.size(); ++i) {
		ifp.at(i).ID = i;
		ifp.at(i).norm = norms.at(i);
	}
	std::sort(ifp.begin(), ifp.end());

	std::vector<int> output(output_num);
	for (int i = 0; i < output_num; ++i) {
		output.at(i) = ifp.at(i).ID;
	}

	return output;
}

prdc_lzw::EncodedText PRDC::compress(
		prdc_lzw::EncodedText& uncompressed) const {

	prdc_lzw::EncodedText output;
	int dictSize = 0;
	std::map<std::vector<int>, int> dictionary;
	std::vector<int> w;

	for (int it : uncompressed.encoded) {
		int c = it;
		std::vector<int> wc(w);
		wc.push_back(c);
		if (dictionary.count(wc))
			w.push_back(c);
		else {
			output.push_back(dictionary[w]);
			// Add wc to the dictionary.
			dictionary[wc] = dictSize++;
			w.clear();
			w.push_back(c);
		}
	}

// Output the code for w.
	if (!w.empty())
		output.push_back(dictionary[w]);
	return output;
}

} /* namespace histgram_map */
