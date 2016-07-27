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
#include <functional> //std::function
#include "opencv2/opencv.hpp"
#include "Dictionary.h"

#define prdc_base

namespace prdc {

const int PRDC_BASE = 0x0001;
const int PAIR_MULTIPLE = 0x0002;
const int PAIR_MULTIPLE_LOG = 0x0004;
const int RECOMPRESSION = 0x0008;

const std::vector<int> METHOD_ARRAY = { PRDC_BASE, PAIR_MULTIPLE,
		PAIR_MULTIPLE_LOG, RECOMPRESSION };

/*
 *
 */
class PRDC {
public:
	PRDC(std::vector<std::string>& base_dics_path, int flag = PRDC_BASE,
			int READY_FOR_NEXT = 0, bool USE_LAST_DATA = false);

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
			std::vector<float>& data_class);

	/**
	 * @brief file_nameのベクトルに近いk個のベクトル中で最も多いクラスを返します
	 * @param file_name
	 * @param k
	 * @return
	 */
	float find_nearest(std::string file_name, int k);

	/**
	 * @brief 作成した圧縮率ベクトル等を保存します
	 * @param filename
	 * @return
	 */
	bool save(std::string filename);

private:
	std::vector<std::string> base_dics_names; ///基底辞書のファイルパス
	std::vector<prdc_lzw::Dictionary> base_dics; ///基底辞書
	std::vector<float> classes;
	std::vector<std::vector<float>> vectors;
	CvKNearest knn;
	cv::Mat training_mat;
	cv::Mat classes_mat;
	int prdc_flag;
	int ready_for_next;
	bool use_last_data;

	static std::vector<std::vector<std::vector<float>>>PRDC_LAST_LEARNING;

	/**
	 * @brief base_dicsで圧縮した圧縮率ベクトルを作成します（PRDC）
	 * @param text 元テキスト
	 * @return 圧縮率ベクトル
	 */
	std::vector<float> make_compless_vector(std::string& text,std::vector<std::vector<int>>& compressed) const;
	std::vector<float> make_pair_multiple_vector(std::string& text,std::vector<std::vector<int>>& compressed) const;
	std::vector<float> make_pair_multiple_log_vector(std::string& text,
			std::vector<std::vector<int>>& compressed) const;
	std::vector<float> make_recompression_vector(std::string& text,std::vector<std::vector<int>>& compressed) const;

	std::vector<float> make_vector(std::string& text, int i = -1);

	typedef std::vector<float> (PRDC::*MakeVecPtr)(std::string&,std::vector<std::vector<int>>&) const;
	static const MakeVecPtr make_vector_functions[];

	void file_read(std::string path, std::string& output) const;
	std::vector<std::pair<int, int>> make_pair(
			std::vector<int>& compressed) const;
	std::map<std::pair<int, int>, int> make_pair_histgram(
			std::vector<std::pair<int, int>>& pair) const;
	std::vector<int> compress(std::vector<int>& text) const;

};
}
/* namespace prdc */

#endif /* SRC_PRDC_H_ */
