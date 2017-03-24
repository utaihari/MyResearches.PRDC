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
#include "Debug.h"

using std::cout;
using std::endl;

namespace prdc {
int ofs_count = 0;

std::vector<std::vector<std::vector<double>>>PRDC::PRDC_LAST_TRAIN_EACH(METHOD_ARRAY.size());
std::vector<std::vector<prdc_lzw::EncodedText>> PRDC::PRDC_LAST_TRAIN(0);

const PRDC::MakeVecPtr PRDC::make_vector_functions[] = {
		&PRDC::make_compless_vector, &PRDC::make_pair_multiple_vector,
		&PRDC::make_pair_multiple_log_vector, &PRDC::make_recompression_vector,
		&PRDC::make_self_compression_vector,
		&PRDC::make_pair_and_selfcompression_vector,
		&PRDC::make_recompression_correct_vector, &PRDC::make_compless_vector };

std::vector<double> PRDC::compress_rate_array;
std::vector<double> PRDC::recompress_rate_array;

bool PRDC::train(std::vector<std::string>& training_data_paths,
		std::vector<double>& data_class, bool encoded) {

	if (training_data_paths.size() != data_class.size()) {
		std::cout << "エラー：データとクラスの数が違います." << std::endl;
		return false;
	}

	//念の為data_classをコピー
	copy(data_class.begin(), data_class.end(), back_inserter(classes));

	vectors.resize(training_data_paths.size());

	//前回作った圧縮率ベクトルを使い回す
	use_last_data_direct = false;
	for (int i = 0; i < (int) METHOD_ARRAY.size(); ++i) {
		if (prdc_flag & METHOD_ARRAY.at(i)) {
			if (PRDC_LAST_TRAIN_EACH.at(i).size()
					!= training_data_paths.size()) {
				PRDC_LAST_TRAIN_EACH.at(i).clear();
				PRDC_LAST_TRAIN_EACH.at(i).shrink_to_fit();
				PRDC_LAST_TRAIN_EACH.at(i).resize(training_data_paths.size());
			} else if (use_last_data) {
				use_last_data_direct = true;
			}
			break;
		}
	}
	if (PRDC_LAST_TRAIN.size() != training_data_paths.size()) {
		use_last_data = false;
		PRDC_LAST_TRAIN.clear();
		PRDC_LAST_TRAIN.shrink_to_fit();
		PRDC_LAST_TRAIN.resize(training_data_paths.size());
	}
#ifdef DEBUG_OUTPUT
	int debug = 1;
#endif
	//それぞれのテキストに対しての処理
#ifdef PARALLEL
#pragma omp parallel for
#endif
	for (int i = 0; i < (int) training_data_paths.size(); ++i) {
		std::string text;
		if (encoded) {
			text = training_data_paths.at(i);
		} else {
			prdc_util::FilePathToString(training_data_paths.at(i), text);
		}
		std::vector<double> vec; //PRDCに使用するベクトル

		//ベクトル作成ここから
		vec = make_vectors(text, i);

		//ベクトル作成ここまで
#ifdef DEBUG_OUTPUT
		if (debug % 10 == 0) {
			std::cout << training_data_paths.size() << "個中" << debug << "個完了"
					<< std::endl;
		}
#endif
		//全てのベクトルが完成した後mat型に変換します（ここでは纏めておくだけ）
		vectors.at(i) = std::vector<double>(vec);
#ifdef DEBUG_OUTPUT
		debug++;
#endif
	}

	//正規化
	if (prdc_flag & RECOMPRESSION_CORRECT) {
		recompress_vecter_normalization(vectors);
	} else if (prdc_flag & PRDC_NORMALIZED) {
		compress_vecter_normalization(vectors);
	} else if (prdc_flag & NORMALIZE) {
		compress_vecter_each_normalization(vectors);
	}

//データをMat型に変換する
	if (prdc_flag & USE_MAHARANOBIS) {
//		cout << "USE_MAHARANOBIS" << endl;
		training_mat.create(training_data_paths.size(), vectors.at(0).size(),
		CV_64FC1);
		classes_mat.create(1, data_class.size(), CV_64FC1);

		for (int i = 0; i < (int) training_data_paths.size(); ++i) {
			for (int j = 0; j < (int) vectors.at(0).size(); ++j) {
				training_mat.at<double>(i, j) = (double) vectors[i][j];
			}
		}
		make_invert_matrix();
	} else {
//Mat(全てのデータ数,次元数,CV_32FC1);
		training_mat.create(training_data_paths.size(), vectors.at(0).size(),
		CV_32FC1);
		classes_mat.create(1, data_class.size(), CV_32FC1);

		for (int i = 0; i < (int) training_data_paths.size(); ++i) {
			for (int j = 0; j < (int) vectors.at(0).size(); ++j) {
				training_mat.at<float>(i, j) = (float) vectors[i][j];
			}
			classes_mat.at<float>(0, i) = (float) classes.at(i);
		}
		knn.train(training_mat, classes_mat);
	}
	return true;
}

PRDC::PRDC(std::vector<std::string>& bases, int flag, bool USE_LAST_DATA,
		bool MULTI_BYTE_CHAR, bool encoded) :
		base_dics_names(bases), prdc_flag(flag), use_last_data(USE_LAST_DATA), use_last_data_direct(
				false), multi_byte_char(MULTI_BYTE_CHAR), encoded(encoded), c_mean(), c_StDev(), rc_mean(), rc_StDev() {

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
		std::vector<std::string>& bases_path, int flag, bool USE_LAST_DATA,
		bool MULTI_BYTE_CHAR, bool encoded) :
		base_dics_names(bases_path), prdc_flag(flag), use_last_data(
				USE_LAST_DATA), use_last_data_direct(false), multi_byte_char(
				MULTI_BYTE_CHAR), encoded(encoded), c_mean(), c_StDev(), rc_mean(), rc_StDev() {

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

	float return_class = 0.0;
	if (prdc_flag & USE_MAHARANOBIS) {
		std::vector<int> response = find_near_vector_mahalanobis(file_name, k);
		std::map<float, int> output;
		for (int q = 0; q < k; ++q) {
			output[classes.at(response.at(q))]++;
		}
		int max = 0;
		for (auto o : output) {
			if (o.second > max) {
				return_class = o.first;
				max = o.second;
			}
		}
	} else {
		std::string text;
		file_read(file_name, text);

//ベクトル作成ここから
		std::vector<double> vec = make_vector(text); //PRDCに使用するベクトル
//ベクトル作成ここまで

		if (prdc_flag & RECOMPRESSION_CORRECT) {
			for (int i = 0; i < (int) vec.size(); i += 2) {
				vec.at(i) = (vec.at(i) - c_mean) / c_StDev;
				vec.at(i + 1) = (vec.at(i + 1) - rc_mean) / rc_StDev;
			}
		} else if (prdc_flag & PRDC_NORMALIZED) {
			for (int i = 0; i < (int) vec.size(); ++i) {
				vec.at(i) = (vec.at(i) - c_mean) / c_StDev;
			}
		} else if (prdc_flag & NORMALIZE) {
			for (int i = 0; i < (int) vec.size(); ++i) {
				vec.at(i) = (vec.at(i) - c_mean_vec.at(i)) / c_StDev_vec.at(i);
			}
		}

		cv::Mat sample(1, vec.size(), CV_32FC1);
		for (int i = 0; i < (int) vec.size(); ++i) {
			sample.at<float>(i) = vec[i];
		}
		return_class = knn.find_nearest(sample, k);
	}

	return return_class;
}

PRDC::PRDC(std::string train_file) :
		base_dics_names(), prdc_flag(), use_last_data(), use_last_data_direct(
				false), multi_byte_char(false), encoded(false), c_mean(), c_StDev(), rc_mean(), rc_StDev() {
	cv::FileStorage cvfs(train_file, CV_STORAGE_READ);
	cvfs["vectors"] >> training_mat;
	cvfs["classes"] >> classes_mat;
	cvfs["base_dics"] >> base_dics_names;
	cvfs["prdc_flag"] >> prdc_flag;
	cvfs["use_multibyte"] >> multi_byte_char;
	cvfs["c_mean"] >> c_mean;
	cvfs["c_StDev"] >> c_StDev;
	cvfs["rc_mean"] >> rc_mean;
	cvfs["rc_StDev"] >> rc_StDev;
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
	classes.resize(training_mat.rows);
	for (auto& v : vectors) {
		v.resize(training_mat.cols);
	}

	for (int i = 0; i < (int) training_mat.rows; ++i) {
		for (int j = 0; j < (int) training_mat.cols; ++j) {
			vectors[i][j] = training_mat.at<float>(i, j);
		}
		classes.at(i) = classes_mat.at<float>(0, i);
	}

	knn.train(training_mat, classes_mat);
}
bool PRDC::save(std::string filename) {
	cv::FileStorage cvfs(filename, CV_STORAGE_WRITE);
	cv::write(cvfs, "vectors", training_mat);
	cv::write(cvfs, "classes", classes_mat);
	cv::write(cvfs, "base_dics", base_dics_names);
	cv::write(cvfs, "prdc_flag", prdc_flag);
	cv::write(cvfs, "use_multibyte", multi_byte_char);
	cv::write(cvfs, "c_mean", c_mean);
	cv::write(cvfs, "c_StDev", c_StDev);
	cv::write(cvfs, "rc_mean", rc_mean);
	cv::write(cvfs, "rc_StDev", rc_StDev);
	cvfs.release();
	return true;
}

std::vector<double> PRDC::make_compless_vector(std::string& text,
		std::vector<prdc_lzw::EncodedText>& comp) const {
	std::vector<double> vec(base_dics.size());

//基底辞書それぞれで圧縮し、圧縮率をベクトルにする
	for (int i = 0; i < (int) base_dics.size(); ++i) {
		auto& compressed = comp.at(i);
		vec.at(i) = ((double) compressed.size() / (double) text.length());
	}
	return vec;
}

std::vector<double> PRDC::make_pair_multiple_vector(std::string& text,
		std::vector<prdc_lzw::EncodedText>& comp) const {

	std::vector<double> vec(base_dics.size() * 2);

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

		vec.at(vec_index) = (double) compressed.length()
				/ (double) text.length();
		vec.at(vec_index + 1) = 1.0
				- (double) reduction / (2.0 * (double) text.length());

		vec_index += 2;
	}
	return vec;
}
std::vector<double> PRDC::make_pair_multiple_log_vector(std::string& text,
		std::vector<prdc_lzw::EncodedText>& comp) const {
	std::vector<double> vec(base_dics.size() * 2);

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

		vec.at(vec_index) = (double) compressed.size() / (double) text.length();
		vec.at(vec_index + 1) = 1.0
				- ((double) reduction / (double) text.length());
		vec_index += 2;
	}
	return vec;
}

std::vector<double> PRDC::make_recompression_vector(std::string& text,
		std::vector<prdc_lzw::EncodedText>& comp) const {

	std::vector<double> vec(base_dics.size() * 2);
	int vec_index = 0; //圧縮率ベクトルのインデックス

//基底辞書それぞれで圧縮し、圧縮率をベクトルにする
	for (int i = 0; i < (int) base_dics.size(); ++i) {
		auto& compressed = comp.at(i);

		//通常の圧縮率ベクトル作成
		vec.at(vec_index) = (double) compressed.size() / (double) text.length();
		//再圧縮した圧縮率ベクトル作成

		prdc_lzw::EncodedText recompress;
		recompress = compress(compressed); //2分木を用いた圧縮(わずかに処理速度早い)
		double comp_size = (double) recompress.size()
				/ (double) compressed.size();

//		prdc_lzw::Dictionary recompress(compressed);//トライ木を用いた圧縮(機能豊富)
//		float comp_size = (float) recompress.compressed.size()
//				/ (float) compressed.size();

		vec.at(vec_index + 1) = comp_size;
		vec_index += 2;
	}
	return vec;
}
std::vector<double> PRDC::make_recompression_correct_vector(std::string& text,
		std::vector<prdc_lzw::EncodedText>& comp) const {

	std::vector<double> vec(base_dics.size() * 2);
	int vec_index = 0; //圧縮率ベクトルのインデックス

//基底辞書それぞれで圧縮し、圧縮率をベクトルにする
	for (int i = 0; i < (int) base_dics.size(); ++i) {
		auto& compressed = comp.at(i);

		//通常の圧縮率ベクトル作成
		vec.at(vec_index) = (double) compressed.size() / (double) text.length();
		//再圧縮した圧縮率ベクトル作成

		prdc_lzw::EncodedText recompress;
		recompress = compress(compressed); //2分木を用いた圧縮(わずかに処理速度早い)
		double comp_size = (double) recompress.size()
				/ (double) compressed.size();

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

std::vector<double> PRDC::make_vectors(std::string& text, int index) {
	std::vector<double> vec; //PRDCに使用するベクトル
	std::vector<prdc_lzw::EncodedText> compressed(base_dics.size());

	if (use_last_data == false) {
		PRDC_LAST_TRAIN.at(index).resize(base_dics.size());
		//前回の結果を使わない(使えない)場合
		for (int p = 0; p < (int) base_dics.size(); ++p) {
			//それぞれの基底辞書で圧縮を行い、次回のため記録する
			PRDC_LAST_TRAIN.at(index).at(p) = base_dics.at(p).Compress(text);

		}
		compressed = PRDC_LAST_TRAIN.at(index);
		for (int i = 0; i < (int) METHOD_ARRAY.size(); ++i) {
			//それぞれの手法について調べ、次回のために記録する
			if (prdc_flag & METHOD_ARRAY.at(i)) {
				PRDC_LAST_TRAIN_EACH.at(i).at(index) = (this
						->*make_vector_functions[i])(text, compressed);
				vec = PRDC_LAST_TRAIN_EACH.at(i).at(index);
				break;
			}
		}
	} else if (use_last_data_direct == true) {
		//前回の圧縮率ベクトルを参照
		for (int i = 0; i < (int) METHOD_ARRAY.size(); ++i) {
			//前回の圧縮率を参照
			//それぞれの手法について調べる
			if (prdc_flag & METHOD_ARRAY.at(i)) {
				vec = PRDC_LAST_TRAIN_EACH.at(i).at(index);
				break;
			}
		}
	} else {
		for (int i = 0; i < (int) METHOD_ARRAY.size(); ++i) {
			//前回の圧縮率を参照
			compressed = PRDC_LAST_TRAIN.at(index);
			//それぞれの手法について調べる
			if (prdc_flag & METHOD_ARRAY.at(i)) {
				PRDC_LAST_TRAIN_EACH.at(i).at(index) = (this
						->*make_vector_functions[i])(text, compressed);
				vec = PRDC_LAST_TRAIN_EACH.at(i).at(index);
			}
		}
	}
	return vec;
}

std::vector<double> PRDC::make_vector(std::string& text) {
	std::vector<double> vec; //PRDCに使用するベクトル
	std::vector<prdc_lzw::EncodedText> compressed(base_dics.size());

	for (int p = 0; p < (int) base_dics.size(); ++p) {
		//それぞれの基底辞書で圧縮を行う
		compressed.at(p) = base_dics.at(p).Compress(text);
	}
	for (int flag = 0; flag < (int) METHOD_ARRAY.size(); ++flag) {
		if (prdc_flag & METHOD_ARRAY.at(flag)) {
			vec = (this->*make_vector_functions[flag])(text, compressed);
			break;
		}
	}
	return vec;
}

std::vector<double> PRDC::make_self_compression_vector(std::string& text,
		std::vector<prdc_lzw::EncodedText>& comp) const {

	std::vector<double> vec(base_dics.size() + 1);

//基底辞書それぞれで圧縮し、圧縮率をベクトルにする
	for (int i = 0; i < (int) base_dics.size(); ++i) {
		auto& compressed = comp.at(i);
		//通常の圧縮率ベクトル作成
		vec.at(i) = (double) compressed.size() / (double) text.length();
	}

	prdc_lzw::Dictionary self_compress(text);

	vec.at(vec.size() - 1) = (double) self_compress.compressed.length()
			/ (double) text.length();

	return vec;
}

std::vector<double> PRDC::make_pair_and_selfcompression_vector(
		std::string& text,
		std::vector<prdc_lzw::EncodedText>& compressed) const {
	std::vector<double> vec = make_pair_multiple_vector(text, compressed);

	prdc_lzw::Dictionary self_compress(text);
	vec.push_back(
			(double) self_compress.compressed.length()
					/ (double) text.length());

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
	std::vector<double> vec = make_vector(text); //PRDCに使用するベクトル
//ベクトル作成ここまで

	if (prdc_flag & RECOMPRESSION_CORRECT) {
		for (int i = 0; i < (int) vec.size(); i += 2) {
			vec.at(i) = (vec.at(i) - c_mean) / c_StDev;
			vec.at(i + 1) = (vec.at(i + 1) - rc_mean) / rc_StDev;
		}
	} else if (prdc_flag & PRDC_NORMALIZED) {
		for (int i = 0; i < (int) vec.size(); ++i) {
			vec.at(i) = (vec.at(i) - c_mean) / c_StDev;
		}
	}

	cv::Mat sample(1, vec.size(), CV_32FC1);
	for (int i = 0; i < (int) vec.size(); ++i) {
		sample.at<float>(i) = vec[i];
	}

	std::vector<double> norms;

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
		std::vector<int> vc{c};
		if(dictionary.count(vc)){
			dictionary[vc] = dictSize++;
		}
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
void PRDC::recompress_vecter_normalization(
		std::vector<std::vector<double>>& vectors) {

	std::vector<double> compress_rate(
			vectors.size() * (vectors.at(0).size() / 2));
	std::vector<double> recompress_rate(
			vectors.size() * (vectors.at(0).size() / 2));

	for (int i = 0; i < (int) vectors.size(); ++i) {
		int vector_index = 0;
		for (int p = 0; p < (int) vectors.at(0).size(); p += 2) {
			compress_rate.at((i * (vectors.at(0).size() / 2)) + vector_index) =
					vectors.at(i).at(p);
			recompress_rate.at((i * (vectors.at(0).size() / 2)) + vector_index) =
					vectors.at(i).at(p + 1);
			vector_index++;
		}
	}

	auto c_var = prdc_util::mean_variance<double>(compress_rate);
	auto rc_var = prdc_util::mean_variance<double>(recompress_rate);

	c_mean = c_var.first;
	c_StDev = std::sqrt(c_var.second);
	rc_mean = rc_var.first;
	rc_StDev = std::sqrt(rc_var.second);

	for (int i = 0; i < (int) vectors.size(); ++i) {
		for (int p = 0; p < (int) vectors.at(0).size(); p += 2) {
			vectors.at(i).at(p) = (vectors.at(i).at(p) - c_mean) / c_StDev;
			vectors.at(i).at(p + 1) = (vectors.at(i).at(p + 1) - rc_mean)
					/ rc_StDev;
		}
	}
}

void PRDC::compress_vecter_normalization(
		std::vector<std::vector<double>>& vectors) {

	std::vector<double> compress_rate(vectors.size() * vectors.at(0).size());

	for (int i = 0; i < (int) vectors.size(); ++i) {
		for (int p = 0; p < (int) vectors.at(0).size(); ++p) {
			compress_rate.at((i * vectors.at(0).size()) + p) = vectors.at(i).at(
					p);

		}
	}

	auto c_var = prdc_util::mean_variance<double>(compress_rate);

	c_mean = c_var.first;
	c_StDev = std::sqrt(c_var.second);

	for (int i = 0; i < (int) vectors.size(); ++i) {
		for (int p = 0; p < (int) vectors.at(0).size(); ++p) {
			vectors.at(i).at(p) = (vectors.at(i).at(p) - c_mean) / c_StDev;
		}
	}
}

void PRDC::compress_vecter_each_normalization(
		std::vector<std::vector<double>>& vectors) {

	std::vector<std::vector<double>> compress_rate(vectors.at(0).size());
	for (int i = 0; i < (int) vectors.at(0).size(); ++i) {
		compress_rate.at(i).resize(vectors.size());
	}
	for (int i = 0; i < (int) vectors.size(); ++i) {
		for (int p = 0; p < (int) vectors.at(0).size(); ++p) {
			compress_rate.at(p).at(i) = vectors.at(i).at(p);

		}
	}

	std::vector<std::pair<double, double>> c_var(vectors.at(0).size());
	c_mean_vec.resize(vectors.at(0).size());
	c_StDev_vec.resize(vectors.at(0).size());

	for (int i = 0; i < (int) vectors.at(0).size(); ++i) {
		c_var.at(i) = prdc_util::mean_variance<double>(compress_rate.at(i));
		c_mean_vec.at(i) = c_var.at(i).first;
		c_StDev_vec.at(i) = std::sqrt(c_var.at(i).second);
	}
	for (int j = 0; j < (int) vectors.size(); ++j) {
		for (int k = 0; k < (int) vectors.at(0).size(); ++k) {
			vectors.at(j).at(k) = (vectors.at(j).at(k) - c_mean_vec.at(k))
					/ c_StDev_vec.at(k);
		}
	}
}

void PRDC::make_invert_matrix() {
	int data_size = (int) vectors.size();
	cv::Mat* data = new cv::Mat[data_size];

	int ch = CV_64F;

	for (int i = 0; i < (int) data_size; ++i) {
		data[i].create(vectors.at(i).size(), 1, ch);
		for (int p = 0; p < (int) vectors.at(i).size(); ++p) {
			data[i].at<double>(p) = vectors.at(i)[p];
		}
	}
	vector_mats.resize(data_size);
	for (int i = 0; i < (int) data_size; ++i) {
		vector_mats.at(i) = cv::Mat(data[i]);
	}

	cv::Mat mean;
	cv::calcCovarMatrix(training_mat, cover_matrix, mean,
	CV_COVAR_NORMAL | CV_COVAR_ROWS | CV_COVAR_SCALE, ch);

	if (prdc_flag & M_EDIT) {
//		for (int x = 0; x < (int) cover_matrix.cols; ++x) {
//			for (int y = 0; y < (int) cover_matrix.rows; ++y) {
//				if (y % 2 == 0) {
//					//圧縮率に対する処理
//					//自分と後ろの数値以外を0にする
//					if (x != y && x != (y + 1)) {
//						cover_matrix.at<double>(y, x) = 0.0;
//					}
//				} else {
//					//再圧縮率に対する処理
//					//自分と前の数値以外を0にする
//					if (x != y && (x + 1) != y) {
//						cover_matrix.at<double>(y, x) = 0.0;
//					}
//				}
//			}
//		}
		for (int x = 0; x < (int) cover_matrix.cols; ++x) {
			for (int y = 0; y < (int) cover_matrix.rows; ++y) {
				if (y % 2 == 0) {
					//圧縮率に対する処理
					//自分と後ろの数値以外を0にする
					if (x == (y + 1)) {
						cover_matrix.at<double>(y, x) = 0.0;
					}
				} else {
					//再圧縮率に対する処理
					//自分と前の数値以外を0にする
					if ((x + 1) == y) {
						cover_matrix.at<double>(y, x) = 0.0;
					}
				}
			}
		}
	}

//	cout << "cover_matrix\n" << cover_matrix << endl;
	cv::invert(cover_matrix, cover_matrix_inv, cv::DECOMP_SVD);
//	cout << "inv_cover_matrix\n" << cover_matrix_inv << endl;
	delete[] data;
}
std::vector<int> PRDC::find_near_vector_mahalanobis(
		std::string input_image_path, int output_num, bool encoded) {
	std::string text;
	if (encoded) {
		text = input_image_path;
	} else {
		file_read(input_image_path, text);
	}

//ベクトル作成ここから
	std::vector<double> vec = make_vector(text); //PRDCに使用するベクトル
//ベクトル作成ここまで

	if (prdc_flag & RECOMPRESSION_CORRECT) {
		for (int i = 0; i < (int) vec.size(); i += 2) {
			vec.at(i) = (vec.at(i) - c_mean) / c_StDev;
			vec.at(i + 1) = (vec.at(i + 1) - rc_mean) / rc_StDev;
		}
	} else if (prdc_flag & PRDC_NORMALIZED) {
		for (int i = 0; i < (int) vec.size(); ++i) {
			vec.at(i) = (vec.at(i) - c_mean) / c_StDev;
		}
	} else if (prdc_flag & NORMALIZE) {
		for (int i = 0; i < (int) vec.size(); ++i) {
			vec.at(i) = (vec.at(i) - c_mean_vec.at(i)) / c_StDev_vec.at(i);
		}
	}

	std::vector<double> norms;
	calc_mahalanobis_of_all_vector(vec, norms);
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

}
/* namespace histgram_map */
