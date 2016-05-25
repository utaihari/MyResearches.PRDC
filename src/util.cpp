/*
 * util.cpp
 *
 *  Created on: 2015/12/03
 *      Author: taichi
 */
#include "util.h"
#include <iostream>
#include <set>
#include <algorithm>
#include <time.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
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
			H += Aiter->second < Biter->second ? Aiter->second : Biter->second;
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

	//前準備 NMDでは0回出力された単語を1回出力されたことにする
	//TODO 事前に(このメソッドを呼び出す前に)終わらせておくようにする
	for (int i = 0; i < (int) dicA.histgram.size(); ++i) {
		if (dicA.histgram.at(i).second == 0) {
			dicA.histgram.at(i).second = 1;
		}
	}
	for (int i = 0; i < (int) dicB.histgram.size(); ++i) {
		if (dicB.histgram.at(i).second == 0) {
			dicB.histgram.at(i).second = 1;
		}
	}
	//前準備ここまで

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

			//以下のコメントアウトされた式では、どちらかの配列が先に終了した場合に不具合が起きる
			//H += Aiter->second > Biter->second ? Aiter->second : Biter->second;

			int Aplus, Bplus;
			if (Aiter->second > Biter->second) {
				Aplus = Aiter->second;
				Bplus = 0;
			} else {
				Aplus = 0;
				Bplus = Biter->second;
			}
			if (!Bfinished) {
				Bsize += Biter->second;
				H += Bplus;
				if (Biter == B.end() - 1) {
					Bfinished = true;
				} else {
					Biter++;
				}
			}
			if (!Afinished) {
				Asize += Aiter->second;
				H += Aplus;
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

	//std::cout << "H:" << H << " max:" << max_dicsize << " min:" << min_dicsize << std::endl;

	double nmd = (double) ((double) (H - min_dicsize) / (double) max_dicsize);
	return nmd;
}
double NormalizedDictionaryDistance(prdc_lzw::Dictionary& dicA,
		prdc_lzw::Dictionary& dicB) {

	std::vector<std::pair<std::string, double>>& A = dicA.histgram;
	std::vector<std::pair<std::string, double>>& B = dicB.histgram;
	auto Aiter = A.begin();
	auto Biter = B.begin();
	int Asize = (int) A.size();
	int Bsize = (int) B.size();

	bool Afinished = false;
	bool Bfinished = false;

	int H = 0;

	while (!(Afinished && Bfinished)) {
		//Aのデータ番号とBのデータ番号が同じだったら
		if (Aiter->first == Biter->first) {
			H++;
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
			H++;
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
			H++;
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

	int max_dicsize;
	int min_dicsize;

	if (Asize > Bsize) {
		max_dicsize = Asize;
		min_dicsize = Bsize;
	} else {
		max_dicsize = Bsize;
		min_dicsize = Asize;
	}

	//std::cout << "H:" << H << " max:" << max_dicsize << " min:" << min_dicsize << std::endl;

	double nmd = (double) ((double) (H - min_dicsize) / (double) max_dicsize);
	return nmd;
}
std::vector<int> Compress(const std::string &uncompressed,
		prdc_lzw::Dictionary& dic, unsigned int flags) {

	std::vector<int> compressed;
	prdc_lzw::LzwNode* current_node = dic.get_root(); //初期位置
	unsigned int dicsize = 256;
	unsigned int string_length = 1;
	std::string w; //複数回の圧縮で共通数字を出力するため、元のテキストを保存しておく

	//数値→文字列変換のための配列のサイズ設定
	dic.binding.reserve(dic.max_dicsize + 256);

	for (std::string::const_iterator it = uncompressed.begin();
			it != uncompressed.end(); ++it) {

		char c = *it;	//未圧縮の文字列から一文字取り出す
		std::string wc = w + c;
		prdc_lzw::LzwNode* q = current_node->FindChild(c);

		if (q != NULL) {
			//辞書に文字列が追加されていたら
			current_node = q;	//探索ノードを一つ進める
			w = wc;
			string_length++;
		} else {
			compressed.push_back(current_node->get_data());

			if ((flags & ARROW_EDIT_DICTIONARY)) {
				if (dicsize < dic.max_dicsize
						&& string_length < dic.max_length) {
					dicsize++;
					dic.AddNode(current_node, c);	//current_nodeの下に文字cのノードを作成
					dic.binding.push_back(wc);
				}
			}
			//NOTE:圧縮文字列に0以下、または256以上の文字コードが入っていた場合エラーになる
			current_node = dic.get_root()->FindChild(c);	//最初から検索し直す
			w = std::string(1, c);
			string_length = 1;
		}
	}
	std::vector<std::string>(dic.binding).swap(dic.binding);
	compressed.push_back(current_node->get_data());
	return compressed;
}
std::vector<std::pair<std::string, double>>& MakeHistgram(
		prdc_lzw::Dictionary& dic) {
	std::vector<int> output;
	const unsigned int max_value = dic.binding.size();
	output.resize(max_value, 0);
	dic.histgram.resize(max_value);
	for (auto i : dic.compressed) {
		output[i]++;
	}
	for (unsigned int i = 0; i < max_value; ++i) {
		dic.histgram.at(i) = std::make_pair(dic.binding.at(i),
				(double) (output.at(i)));
	}

	std::sort(dic.histgram.begin(), dic.histgram.end());
	return dic.histgram;
}
std::vector<std::pair<std::string, double>> MakeHistgram(
		std::vector<int> compressed, std::vector<std::string>& bind_data) {
	std::vector<int> output;
	std::vector<std::pair<std::string, double>> histgram;
	const unsigned int max_value = bind_data.size();

	output.resize(max_value, 0);
	histgram.resize(max_value);
	for (auto i : compressed) {
		output[i]++;
	}

	for (unsigned int i = 0; i < max_value; ++i) {
		histgram.at(i) = std::make_pair(bind_data.at(i),
				(double) (output.at(i)));
	}

	std::sort(histgram.begin(), histgram.end());
	return histgram;
}
std::vector<std::pair<std::string, std::string>> MakePair(
		std::vector<std::string> compressed) {
	std::vector<std::pair<std::string, std::string>> output;

	std::string prev = "";
	std::string next = "";

	for (int i = 0; i < (int) compressed.size() - 1; ++i) {
		prev = compressed.at(i);
		next = compressed.at(i + 1);
		output.push_back(std::make_pair(prev, next));
	}
	std::sort(output.begin(), output.end());
	return output;
}

std::vector<std::string> ConvertNumtoStr(std::vector<int> compressed,
		std::vector<std::string>& bind_data) {
	std::vector<std::string> output;

	for (int i : compressed) {
		output.push_back(bind_data.at(i));
	}
	return output;
}
std::vector<std::pair<std::string, std::string>> FindPair(
		std::vector<std::pair<std::string, std::string>>& A,
		std::vector<std::pair<std::string, std::string>>& B) {
	auto Aiter = A.begin();
	auto Biter = B.begin();

	bool Afinished = false;
	bool Bfinished = false;

	std::vector<std::pair<std::string, std::string>> output;

	while (!(Afinished && Bfinished)) {
		//AとBが同じだったら出力
		if (*Aiter == *Biter) {
			//小さい方の頻度を足す
			output.push_back(*Aiter);
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
		} //Bのほうが小さければ、Bを進める
		else if (*Aiter > *Biter) {
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
	return output;
}

void SavingImages::Push(std::string image_name, const cv::Mat& image) {
	//タイトルから"/"の抜き取り
	for (size_t c = image_name.find_first_of("/"); c != std::string::npos; c =
			image_name.find_first_of("/")) {
		image_name.erase(c, 1);
	}
	image_names.push_back(image_name);
	images.push_back(image.clone());
}
void SavingImages::Save() {
	//画像保存用フォルダ作成
	struct tm *date;
	time_t now = time(NULL);
	date = localtime(&now);

	std::string today = to_string(date->tm_mon + 1) + "月"
			+ to_string(date->tm_mday) + "日" + to_string(date->tm_hour) + "時"
			+ to_string(date->tm_min) + "分" + to_string(date->tm_sec) + "秒";

	std::string path = "output/" + today + ":" + std::to_string(images.size());
	struct stat st;
	if (stat(path.c_str(), &st) != 0) {
		mkdir(path.c_str(), 0775);
	}

	for (int i = 0; i < (int) image_names.size(); ++i) {
		//画像保存
		imwrite(path + "/" + image_names.at(i) + ".png", images.at(i));
		cv::waitKey(30);
	}

}

ComparisonImage::ComparisonImage(std::vector<std::string> image_title,
		cv::Mat origin_image, cv::Scalar text_color, int width, int height) :
		image(cv::Mat::zeros(cv::Size(width, height), CV_8UC3)), image_title(image_title.at(0)), origin_image(
				origin_image.clone()), push_times(0) {

	cv::Mat roi(image, cv::Rect(0, 0, origin_image.cols, origin_image.rows));
	this->origin_image.copyTo(roi);

	//title 縁取り部分挿入
	for (int i = 0; i < (int) image_title.size(); ++i) {
		cv::putText(image, image_title.at(i), cv::Point(20, 30 + (i * 40)),
				cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 6);
	}

	//title 挿入
	for (int i = 0; i < (int) image_title.size(); ++i) {
		cv::putText(image, image_title.at(i), cv::Point(20, 30 + (i * 40)),
				cv::FONT_HERSHEY_SIMPLEX, 1.0, text_color, 2);
	}
}

void ComparisonImage::Push(std::string text, cv::Scalar text_color) {
	cv::putText(image, text, cv::Point(image.cols + 20, (push_times * 40) + 30),
			cv::FONT_HERSHEY_SIMPLEX, 0.8, text_color, 2);
	push_times++;
}
}
