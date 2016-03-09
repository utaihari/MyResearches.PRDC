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
#include "Dictionary.h"

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
#endif /* SRC_UTIL_H_ */
