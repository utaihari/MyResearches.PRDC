/*
 * PRDC.h
 *
 *  Created on: 2016/07/07
 *      Author: uchinosub
 */

#ifndef SRC_PRDC_H_
#define SRC_PRDC_H_

#include <vector>
#include <string>
#include <map>
#include <valarray>
#include <functional> //std::function
#include "opencv2/opencv.hpp"
#include "Dictionary.h"

/**
 * 新しい圧縮率作成関数の追加方法
 * 1, フラグの追加
 * 2, フラグをMETHOD_ARRAYに追記する
 * 3, 手法の名前をMETHOD_NAME_ARRAYに追記する
 * 4, 自作の圧縮率作成関数をmake_compless_vectorと同じ引数、型で作成する
 * 5, 自作した圧縮率作成関数をPRDC.cppの先頭にあるmake_vector_functionsに追記する
 */

namespace prdc {

/**
 * ここにフラグを追加する。フラグの数値は
 * http://qiita.com/satoshinew/items/566bf91707b5371b62b6
 * などを参考にしてください
 */
const int PRDC_BASE = 0x0001;
const int PAIR_MULTIPLE = 0x0002;
const int PAIR_MULTIPLE_LOG = 0x0004;
const int RECOMPRESSION = 0x0008;
const int SELF_COMPRESSION = 0x0010;
const int PAIR_AND_SELFCOMPRESS = 0x0020;
const int RECOMPRESSION_CORRECT = 0x0040;
const int PRDC_NORMALIZED = 0x0080;
const int USE_MAHARANOBIS = 0x0100;
const int NORMALIZE = 0x0200;
const int M_EDIT = 0x0400;

const std::vector<int> METHOD_ARRAY = { PRDC_BASE, PAIR_MULTIPLE,
		PAIR_MULTIPLE_LOG, RECOMPRESSION, SELF_COMPRESSION,
		PAIR_AND_SELFCOMPRESS, RECOMPRESSION_CORRECT, PRDC_NORMALIZED };
const std::vector<std::string> METHOD_NAME_ARRAY = { "PRDC_BASE",
		"PAIR_MULTIPLE", "PAIR_MULTIPLE_LOG", "RECOMPRESSION",
		"SELF_COMPRESSION", "PAIR_AND_SELFCOMPRESS", "RECOMPRESSION_CORRECT",
		"PRDC_NORMALIZED" };

/*
 *
 */
class PRDC {
public:
	PRDC() :
			base_dics_names(), prdc_flag(), use_last_data(), multi_byte_char(), encoded(
					false), c_mean(), c_StDev(), rc_mean(), rc_StDev() {
	}
	PRDC(std::vector<std::string>& base_dics_path, int method_flag = PRDC_BASE,
			bool USE_LAST_DATA = false, bool MULTI_BYTE_CHAR = false,
			bool encoded = false);
	PRDC(std::vector<std::string>& base_dics_path,
			std::vector<std::string>& base_dics_name, int method_flag =
					PRDC_BASE, bool USE_LAST_DATA = false,
			bool MULTI_BYTE_CHAR = false, bool encoded = false);

	static std::vector<double> compress_rate_array;
	static std::vector<double> recompress_rate_array;

	/**
	 * @brief 以前作成した学習データを読み込みますA
	 * @note base_dicsを変える場合は最初からやり直してください
	 * @param train_file
	 */
	PRDC(std::string train_file);
	virtual ~PRDC();

	/**
	 * @brief 学習データからベクトルを作成し、学習を行います
	 * @param training_data
	 * @param data_class
	 * @return 最後まで終わればtrue
	 */
	bool train(std::vector<std::string>& training_data_paths,
			std::vector<double>& data_class, bool encoded = false);

	/**
	 * @brief file_nameのベクトルに近いk個のベクトル中で最も多いクラスを返します
	 * @param file_name
	 * @param k
	 * @return
	 */
	float find_nearest(std::string file_name, int k);
	;
	/**
	 * @brief 作成した圧縮率ベクトル等を保存します
	 * @param filename
	 * @return
	 */
	bool save(std::string filename);

	/**
	 * @note PRDC_SIMILAR_IMAGE_DISPLAY用関数
	 * @param input_image_path
	 * @param output_num
	 * @return
	 */
	std::vector<int> find_near_vector(std::string input_image_path,
			int output_num, bool encoded = false);

	std::vector<int> find_near_vector_mahalanobis(std::string input_image_path,
			int output_num, bool encoded = false);

	static void crear_last_data() {
		PRDC_LAST_TRAIN.clear();
		PRDC_LAST_TRAIN.shrink_to_fit();
		for (int i = 0; i < (int) METHOD_ARRAY.size(); ++i) {
			PRDC_LAST_TRAIN_EACH.at(i).clear();
			PRDC_LAST_TRAIN_EACH.at(i).shrink_to_fit();
		}
	}
	std::vector<std::string> base_dics_names; ///基底辞書のファイルパス
private:

	std::vector<prdc_lzw::Dictionary> base_dics; ///基底辞書
	std::vector<double> classes;
	std::vector<std::vector<double>> vectors;
	cv::KNearest knn;
	cv::Mat training_mat;
	cv::Mat classes_mat;
	std::vector<cv::Mat> vector_mats;
	cv::Mat cover_matrix;
	cv::Mat cover_matrix_inv;
	int prdc_flag;
//	int ready_for_next;
	bool use_last_data;
	bool use_last_data_direct;
	bool multi_byte_char;
	bool encoded;

	//正規化用
	double c_mean;
	double c_StDev;
	double rc_mean;
	double rc_StDev;

	std::vector<double> c_mean_vec;
	std::vector<double> c_StDev_vec;
	std::vector<double> rc_mean_vec;
	std::vector<double> rc_StDev_vec;

	static std::vector<std::vector<std::vector<double>>>PRDC_LAST_TRAIN_EACH;
	static std::vector<std::vector<prdc_lzw::EncodedText>>PRDC_LAST_TRAIN;

	void recompress_vecter_normalization(std::vector<std::vector<double>>& vectors);
	void compress_vecter_normalization(std::vector<std::vector<double>>& vectors);
	void recompress_vecter_each_normalization(std::vector<std::vector<double>>& vectors);
	void compress_vecter_each_normalization(std::vector<std::vector<double>>& vectors);

	/**
	 * @brief base_dicsで圧縮した圧縮率ベクトルを作成します（PRDC）
	 * @param text 元テキスト
	 * @return 圧縮率ベクトル
	 */
	std::vector<double> make_compless_vector(std::string& text,std::vector<prdc_lzw::EncodedText>& compressed) const;
	std::vector<double> make_pair_multiple_vector(std::string& text,std::vector<prdc_lzw::EncodedText>& compressed) const;
	std::vector<double> make_pair_multiple_log_vector(std::string& text,
			std::vector<prdc_lzw::EncodedText>& compressed) const;
	std::vector<double> make_recompression_vector(std::string& text,std::vector<prdc_lzw::EncodedText>& compressed) const;
	std::vector<double> make_recompression_correct_vector(std::string& text,std::vector<prdc_lzw::EncodedText>& compressed) const;

	std::vector<double> make_self_compression_vector(std::string& text,std::vector<prdc_lzw::EncodedText>& compressed) const;
	std::vector<double> make_pair_and_selfcompression_vector(std::string& text,std::vector<prdc_lzw::EncodedText>& compressed) const;
	//圧縮率ベクトル作成関数（各手法比較用）　ここまで

	/**
	 * @brief 現在使用中の手法をもちいて、圧縮率べクトルを作成します
	 * @param text
	 * @param i
	 * @return
	 */
	std::vector<double> make_vectors(std::string& text, int index);
	std::vector<double> make_vector(std::string& text);

	typedef std::vector<double> (PRDC::*MakeVecPtr)(std::string&,std::vector<prdc_lzw::EncodedText>&) const;
	static const MakeVecPtr make_vector_functions[];

	//細かい処理用
	void file_read(std::string path, std::string& output) const;
	std::vector<std::pair<int, int>> make_pair(
			prdc_lzw::EncodedText& compressed) const;
	std::map<std::pair<int, int>, int> make_pair_histgram(
			std::vector<std::pair<int, int>>& pair) const;
	prdc_lzw::EncodedText compress(prdc_lzw::EncodedText& text) const;

	//以下 PRDC_SIMILAR_IMAGE_DISPLAY用

	struct int_float_pair {
		int ID;
		float norm;

		bool operator<(const int_float_pair& right)const {
			return right.norm > norm;
		}
	};

	void calc_all_vector_norm(std::vector<double>& input_vec,std::vector<double>& output) {
		output.resize(vectors.size());
#ifdef PARALLEL
#pragma omp parallel for
#endif
		for(int i = 0; i<(int)vectors.size();++i) {
			output.at(i) = norm(input_vec,vectors.at(i));
		}
	}
	double norm(std::vector<double>& a,std::vector<double>& b) {
		double d = 0.0;
		for (int i=0; i<(int)a.size(); i++) {
			d += (a[i] - b[i])*(a[i] - b[i]);
		}
		return sqrt(d);
	}

	void make_invert_matrix();
	void calc_mahalanobis_of_all_vector(std::vector<double>& input,std::vector<double>& output) {
		output.resize(vector_mats.size());
		cv::Mat input_mat(input.size(), 1, CV_64F);
		for (int p = 0; p < (int) input.size(); ++p) {
			input_mat.at<double>(p) = input[p];
		}

#ifdef PARALLEL
#pragma omp parallel for
#endif
		for(int i = 0; i<(int)vector_mats.size();++i) {
			output.at(i) = cv::Mahalanobis(input_mat, vector_mats.at(i), cover_matrix_inv);
		}
	}
};

}
/* namespace prdc */

#endif /* SRC_PRDC_H_ */
