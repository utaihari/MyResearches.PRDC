/*
 * util.h
 *
 *  Created on: 2015/12/03
 *      Author: taichi
 */

#ifndef SRC_UTIL_H_
#define SRC_UTIL_H_

#include <string>
#include <sstream>
#include <vector>

template<typename T>
std::string to_string(T value) {
	std::ostringstream os;
	os << value;
	return os.str();
}

void SplitString(const std::string s, std::vector<std::string>& output ,int number_of_partitions);
#endif /* SRC_UTIL_H_ */
