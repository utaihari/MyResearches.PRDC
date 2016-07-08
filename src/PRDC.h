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
#include "opencv2/opencv.hpp"
#include "Dictionary.h"

namespace prdc {

/*
 *
 */
class PRDC {
public:
	PRDC(const std::vector<std::string>& base_dics_path);
	PRDC(const std::string train_file);
	virtual ~PRDC();

	/**
	 * @brief 学習データからベクトルを作成し、学習を行います
	 * @param training_data
	 * @param data_class
	 * @return 最後まで終わればtrue
	 */
	bool train(const std::vector<std::string>& training_data_paths,
			const std::vector<float>& data_class);

	float find_nearest(const std::string file_name, int k) const;

	bool save(const std::string filename);

private:
	std::vector<std::string> base_dics_names;
	std::vector<prdc_lzw::Dictionary> base_dics;
	std::vector<float> classes;
	std::vector<std::vector<float>> vectors;
	CvKNearest knn;
	cv::Mat training_mat;
	cv::Mat classes_mat;
	std::vector<float> make_compless_vector(const std::string text) const;

	void file_read(std::string path, std::string& output) const;
};

} /* namespace prdc */

#endif /* SRC_PRDC_H_ */
