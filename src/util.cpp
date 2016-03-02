/*
 * util.cpp
 *
 *  Created on: 2015/12/03
 *      Author: taichi
 */
#include "util.h"

void SplitString(const std::string s, std::vector<std::string>& output,
		int number_of_partitions) {
	int string_length = s.length();
	int length_of_a_part = (int) (string_length / number_of_partitions);
	int remainder_of_parts = string_length % number_of_partitions;
	int current_index = 0;

	for (int i = 0; i < remainder_of_parts; i++) {
		output.push_back(s.substr(current_index, length_of_a_part + 1));
		current_index += length_of_a_part + 1;
	}

	for (int i = 0; i < number_of_partitions - remainder_of_parts; i++) {
		output.push_back(s.substr(current_index, length_of_a_part));
		current_index += length_of_a_part;
	}
}

