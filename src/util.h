/**
 * @name util.h
 *
 * @date 2015/12/03
 * @author taichi
 */

#ifndef SRC_UTIL_H_
#define SRC_UTIL_H_

#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <iterator>
#include <map>
#include <valarray>
#include "Dictionary.h"
#include "opencv2/opencv.hpp"

namespace prdc_util {

using StringPair = std::pair<std::string,std::string>;

template<typename T>
std::string to_string(T value) {
	std::ostringstream os;
	os << value;
	return os.str();
}

/**
 * @brief 文字列を分ける
 * @param input 入力文字列
 * @param output number_of_partitionsで指定した数に分けられた入力文字列
 * @param number_of_partitions 何等分したいかの数
 */
void SplitString(const std::string& input, std::vector<std::string>& output,
		int number_of_partitions);

double HistgramIntersection(std::vector<std::pair<std::string, double>>& A,
		std::vector<std::pair<std::string, double>>& B);
double NormalizedMultisetDistance(prdc_lzw::Dictionary& dicA,
		prdc_lzw::Dictionary& dicB);
double NormalizedDictionaryDistance(prdc_lzw::Dictionary& dicA,
		prdc_lzw::Dictionary& dicB);
/**
 * @brief 文字列を圧縮し、辞書を抽出する
 * @param uncompressed 圧縮する文字列
 * @param flags 辞書に新たな文字列を追加するなら ARROW_EDIT_DICTIONARY を指定
 */
std::vector<int> Compress(const std::string& uncompressed,
		prdc_lzw::Dictionary& dic, unsigned int flags = 0);
/**
 * @brief 圧縮後の辞書番号のヒストグラムを作成し、辞書番号を元の文字列に置き換え、ソートを行う
 * @note ソートまで行うことに注意
 */
std::vector<std::pair<std::string, double>>& MakeHistgram(
		prdc_lzw::Dictionary& dic);

/**
 * @brief 数値列のヒストグラムを作成し、辞書番号を元の文字列に置き換え、ソートを行う
 * @note ソートまで行うことに注意
 */
std::vector<std::pair<std::string, double>> MakeHistgram(
		std::vector<int> compressed, std::vector<std::string>& bind_data);
std::map<int, double>& MakeHistgramInt(prdc_lzw::Dictionary& dic);
std::map<std::pair<std::string, std::string>, int> MakePairHistgram(
		std::vector<std::pair<std::string, std::string>>& pair);
std::vector<std::pair<std::string, std::string>> MakePair(
		std::vector<std::string> compressed);
std::vector<std::string> ConvertNumtoStr(std::vector<int> compressed,
		std::vector<std::string>& bind_data);
std::vector<std::pair<std::string, std::string>> FindPair(
		std::vector<std::pair<std::string, std::string>>& A,
		std::vector<std::pair<std::string, std::string>>& B);
void GetEachFilePathsAndClasses(std::string folder_path,
		std::vector<std::string>& output_file_contents,
		std::vector<float>& output_file_classes,
		std::vector<std::string>& classes);
void GetEachFilePathsAndClasses(std::string folder_path,
		std::vector<std::string>& output_file_paths,
		std::vector<float>& output_file_classes,
		std::vector<std::string>& classes, std::string file_extension);
void FilePathToString(std::string path, std::string& output);
std::string CurrentTimeString();
int ImagesToString(std::string& dataset_path,
		std::vector<std::string>& image_texts,
		std::vector<std::string>& output_file_paths,
		std::vector<float>& output_file_classes, int QUANTIZED_LEVEL = 5);
void ImageToString(cv::Mat& image, unsigned char* output, const int LEVEL);
std::string GetFileName(std::string file_path);
std::string GetFileClassName(std::string file_path);
std::vector<std::pair<int, std::string>> MakeRanking(
		std::vector<std::vector<int>>& classified_table,
		std::vector<std::string>& data_classes);
std::vector<std::pair<int, std::string>> MakeMistakeRanking(
		std::vector<std::vector<int>>& classified_table,
		std::vector<std::string>& data_classes);

/**
 * @brief 平均と分散を求める
 * @param array
 * @return 平均,分散
 */
template <typename T>
std::pair<double,double> mean_variance(std::vector<T>& array);

class SavingImages {
public:
	SavingImages() {
	}
	void Push(std::string image_name, const cv::Mat& image);
	void Save(std::string output_folder_path = "output",
			std::string folder_name = "", bool add_timestamp = true);
private:
	std::vector<std::string> image_names;
	std::vector<cv::Mat> images;
};
class ComparisonImage {
public:
	ComparisonImage(std::vector<std::string> image_title, cv::Mat origin_image,
			cv::Scalar text_color, int width, int height);
	void Push(std::string text, cv::Scalar text_color);
	cv::Mat image;
	std::string image_title;
private:
	cv::Mat origin_image;
	int push_times;
};
class ChangeDatasetPath {
public:
	ChangeDatasetPath();
	ChangeDatasetPath(const ChangeDatasetPath& right) {
		dataset = right.dataset;
		images = right.images;
	}
	ChangeDatasetPath(std::string dataset_path_str,
			std::string images_path_str);
	std::string ChangePath(std::string& text_path);
private:
	std::string dataset;
	std::string images;
};
template<typename T>
std::string printVector(const std::vector<T> &data, std::string &delimiter =
		", ");
}

template<typename T>
inline std::pair<double, double> prdc_util::mean_variance(
		std::vector<T>& vector) {
	auto mean = std::accumulate(vector.begin(), vector.end(), 0.0)
			/ vector.size();
	auto var = (std::inner_product(vector.begin(), vector.end(), vector.begin(),
			0.0) - mean * mean * vector.size()) / vector.size();
	return std::make_pair(mean, var);
}

#endif /* SRC_UTIL_H_ */
