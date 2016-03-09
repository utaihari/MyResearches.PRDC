/*
 * util.cpp
 *
 *  Created on: 2015/12/03
 *      Author: taichi
 */
#include "util.h"

#include<iostream>

namespace prdc_util {
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

double HistgramIntersection(std::vector<std::pair<std::string, double>>& A,
		std::vector<std::pair<std::string, double>>& B) {
	auto Aiter = A.begin();
	auto Biter = B.begin();

	bool Afinished = false;
	bool Bfinished = false;

	double H = 0;

	while (!(Afinished && Bfinished)) {
		//Aのデータ番号とBのデータ番号が同じだったら頻度が小さい方を足す
		if (Aiter->first == Biter->first) {
			//小さい方の頻度を足す
			H += Aiter->second > Biter->second ? Aiter->second : Biter->second;
			if (!Bfinished) {
				if (Biter == B.end() - 1) {
					Bfinished = true;
				} else {
					Biter++;
				}
			}
			if (!Afinished) {
				if (Aiter == A.end() - 1) {
					Afinished = true;
				} else {
					Aiter++;
				}
			}
		} //Bのデータ番号のほうが小さければ、Bを進める
		else if (Aiter->first > Biter->first) {
			H += Biter->second;
			if (Bfinished) {
				//Bが終わっていたらAを進める
				if (Aiter == A.end() - 1) {
					Afinished = true;
				} else {
					Aiter++;
				}
			} else {
				if (Biter == B.end() - 1) {
					Bfinished = true;
				} else {
					Biter++;
				}
			}
		} //Aのデータ番号のほうが小さければ、Aを進める
		else {
			H += Aiter->second;
			if (Afinished) {
				if (Biter == B.end() - 1) {
					Bfinished = true;
				} else {
					Biter++;
				}
			} else {
				if (Aiter == A.end() - 1) {
					Afinished = true;
				} else {
					Aiter++;
				}
			}
		}
	}
	return H;
}

double NormalizedMultisetDistance(prdc_lzw::Dictionary& dicA,
		prdc_lzw::Dictionary& dicB) {

	std::vector<std::pair<std::string, double>>& A = dicA.histgram;
	std::vector<std::pair<std::string, double>>& B = dicB.histgram;
	auto Aiter = A.begin();
	auto Biter = B.begin();
	double Asize = 0;
	double Bsize = 0;

	bool Afinished = false;
	bool Bfinished = false;

	double H = 0;

	while (!(Afinished && Bfinished)) {
		//Aのデータ番号とBのデータ番号が同じだったら頻度が大きい方を足す
		if (Aiter->first == Biter->first) {
			//大きい方の頻度を足す
			H += Aiter->second < Biter->second ? Aiter->second : Biter->second;
			if (!Bfinished) {
				Bsize += Biter->second;
				if (Biter == B.end() - 1) {
					Bfinished = true;
				} else {
					Biter++;
				}
			}
			if (!Afinished) {
				Asize += Aiter->second;
				if (Aiter == A.end() - 1) {
					Afinished = true;
				} else {
					Aiter++;
				}
			}
		} //Bのデータ番号のほうが小さければ、Bを進める
		else if (Aiter->first > Biter->first) {
			if (Bfinished) {
				//Bが終わっていたらAを進める
				Asize += Aiter->second;
				H += Aiter->second;
				if (Aiter == A.end() - 1) {
					Afinished = true;
				} else {
					Aiter++;
				}
			} else {
				Bsize += Biter->second;
				H += Biter->second;
				if (Biter == B.end() - 1) {
					Bfinished = true;
				} else {
					Biter++;
				}
			}
		} //Aのデータ番号のほうが小さければ、Aを進める
		else {
			if (Afinished) {
				Bsize += Biter->second;
				H += Biter->second;
				if (Biter == B.end() - 1) {
					Bfinished = true;
				} else {
					Biter++;
				}
			} else {
				Asize += Aiter->second;
				H += Aiter->second;
				if (Aiter == A.end() - 1) {
					Afinished = true;
				} else {
					Aiter++;
				}
			}
		}
	}

	double max_dicsize;
	double min_dicsize;

	if (Asize > Bsize) {
		max_dicsize = Asize;
		min_dicsize = Bsize;
	} else {
		max_dicsize = Bsize;
		min_dicsize = Asize;
	}

	std::cout << "H:" << H << " max:" << max_dicsize << " min:" << min_dicsize
			<< std::endl;

	double nmd = (double) ((double) (H - min_dicsize) / (double) max_dicsize);
	return nmd;
}
}
