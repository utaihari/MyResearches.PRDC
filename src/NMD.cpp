/*
 * NMD.cpp
 *
 *  Created on: 2016/08/03
 *      Author: uchinosub
 */

#include "NMD.h"
#include "util.h"
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <boost/filesystem.hpp>

using std::string;
using std::vector;
using std::cout;
using std::endl;
using namespace prdc_util;
namespace fs = boost::filesystem;
namespace image_retrieval {

NMD::NMD() :
		classes(), data_classes() {
	// TODO 自動生成されたコンストラクター・スタブ

}

NMD::~NMD() {
	// TODO Auto-generated destructor stub
}

std::vector<std::pair<float, std::string> > NMD::FindNearest(
		std::string file_path, int k, int flag) {
	string input;
	std::vector<std::pair<float, std::string> > output(k); //pair<class,file_path>
	std::vector<std::pair<double, int>> nmd(histgram_paths.size());

	//検索対象データのヒストグラム作成 ここから
	//検索対象データの辞書作成 ここから
	prdc_util::FilePathToString(file_path, input);
	prdc_lzw::Dictionary input_dic(input);
	prdc_util::MakeHistgram(input_dic);
	auto& input_histgram = input_dic.histgram;
	//検索対象データのヒストグラム作成 ここまで

	if (flag & ORIGINAL_NMD) {
		HistgramZeroToOne(input_dic.histgram);
		//全データとのNMDを測る ここから
#pragma omp parallel for
		for (int i = 0; i < (int) histgram_paths.size(); ++i) {
			std::vector<std::pair<std::string, double>> histgram;
			LoadHistgram(histgram_paths.at(i), histgram);
			nmd.at(i).first = NormalizedMultisetDistance(histgram,
					input_histgram);
			nmd.at(i).second = i; //何番目の配列か記録しておく
		}
	} else if (flag & WEIGHTING_NMD) {
		HistgramZeroToOne(input_dic.histgram);
		//全データとのNMDを測る ここから
#pragma omp parallel for
		for (int i = 0; i < (int) histgram_paths.size(); ++i) {
			std::vector<std::pair<std::string, double>> histgram;
			LoadHistgram(histgram_paths.at(i), histgram);
			nmd.at(i).first = NormalizedMultisetDistanceWeighted(histgram,
					input_histgram);
			nmd.at(i).second = i; //何番目の配列か記録しておく
		}
	} else if (flag & NDD) {
		//全データとのNDDを測る ここから
#pragma omp parallel for
		for (int i = 0; i < (int) histgram_paths.size(); ++i) {
			std::vector<std::pair<std::string, double>> histgram;
			LoadHistgram(histgram_paths.at(i), histgram);
			nmd.at(i).first = NormalizedDictionaryDistance(histgram,
					input_histgram);
			nmd.at(i).second = i; //何番目の配列か記録しておく
//			cout << "nmd.at(i).first " << nmd.at(i).first << endl;
		}
	}

//NMDの値でソート
	std::sort(nmd.begin(), nmd.end(),
			[](std::pair<double, int> a,std::pair<double, int> b)->int {return (a.first < b.first);});

//出力の作成
	for (int i = 0; i < k; ++i) {
		output.at(i).first = data_classes.at(nmd.at(i).second);
		output.at(i).second = histgram_paths.at(nmd.at(i).second);
	}

	return output;
}

int NMD::SetCodebook(std::string folder_path) {
	std::vector<std::string> data_paths;
	prdc_util::GetEachFilePathsAndClasses(folder_path, data_paths, data_classes,
			classes);
	int debug = 1;
//それぞれのデータのヒストグラム作成
#pragma omp parallel for
	for (int i = 0; i < (int) data_paths.size(); ++i) {
		if (hasHistgramMade(data_paths.at(i))) {
			cout << GetFileName(data_paths.at(i)) << " 作成済みのため省略します" << endl;
			continue;
		}
		string text;
		prdc_util::FilePathToString(data_paths.at(i), text);
		prdc_lzw::Dictionary dic(text);
		prdc_util::MakeHistgram(dic);

		HistgramZeroToOne(dic.histgram);

		SaveHistgram(GetFileName(data_paths.at(i)),
				classes.at((int) data_classes.at(i)), dic.histgram);

		if (debug % 10 == 0) {
			cout << debug << "番目 作成完了" << endl;
		}
		debug++;
	}
	prdc_util::GetEachFilePathsAndClasses(database_path, histgram_paths,
			data_classes, classes, ".histgram"); //データベースの更新

	return 0;
}

NMD::NMD(std::string database_filepath) :
		database_path(database_filepath) {
	prdc_util::GetEachFilePathsAndClasses(database_filepath, histgram_paths,
			data_classes, classes, ".histgram");
}

void NMD::Save(std::string filename) {
	cout << "saving" << endl;
	vector<string> text;
//HistgramToString(data_histgrams.at(0), text);
	cout << text.at(0) << endl;
}

void NMD::HistgramToString(
		const std::vector<std::pair<std::string, double> >& histgram,
		std::vector<std::string>& output) {
	output.resize(histgram.size());

	for (int i = 0; i < (int) histgram.size(); ++i) {
		string& o_str = output.at(i);
		o_str = to_string(histgram.at(i).first.length());
		o_str += " " + std::to_string(histgram.at(i).second) + " ";
		vector<int> str_int;
		StringToInt(histgram.at(i).first, str_int);
		for (auto i : str_int) {
			o_str += std::to_string(i) + " ";
		}

	}
}

void NMD::StringToInt(const std::string& s, std::vector<int>& output) {
	for (std::string::const_iterator it = s.begin(); it != s.end(); it++) {
		int pre_lead = *it;	//文字列から一文字取り出す
		//cout << "char" << pre_lead << endl;
		output.push_back(pre_lead);
	}
}

double NMD::NormalizedMultisetDistance(
		const std::vector<std::pair<std::string, double> >& histgramA,
		const std::vector<std::pair<std::string, double> >& histgramB) const {

	auto Aiter = histgramA.begin();
	auto Biter = histgramB.begin();
	double Asize = 0;
	double Bsize = 0;

	bool Afinished = false;
	bool Bfinished = false;

	double H = 0;

	while (!(Afinished && Bfinished)) {
		//Aのデータ番号とBのデータ番号が同じだったら頻度が大きい方を足す
		if (Aiter->first == Biter->first) {
			//大きい方の頻度を足す

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
				if (Biter == histgramB.end() - 1) {
					Bfinished = true;
				} else {
					Biter++;
				}
			}
			if (!Afinished) {
				Asize += Aiter->second;
				H += Aplus;
				if (Aiter == histgramA.end() - 1) {
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
				if (Aiter == histgramA.end() - 1) {
					Afinished = true;
				} else {
					Aiter++;
				}
			} else {
				Bsize += Biter->second;
				H += Biter->second;
				if (Biter == histgramB.end() - 1) {
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
				if (Biter == histgramB.end() - 1) {
					Bfinished = true;
				} else {
					Biter++;
				}
			} else {
				Asize += Aiter->second;
				H += Aiter->second;
				if (Aiter == histgramA.end() - 1) {
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

double NMD::NormalizedMultisetDistanceWeighted(
		const std::vector<std::pair<std::string, double> >& histgramA,
		const std::vector<std::pair<std::string, double> >& histgramB) const {
	auto Aiter = histgramA.begin();
	auto Biter = histgramB.begin();
	double Asize = 0;
	double Bsize = 0;

	bool Afinished = false;
	bool Bfinished = false;

	double H = 0;

	while (!(Afinished && Bfinished)) {
		//Aの文字列とBの文字列が同じだったら頻度が大きい方を足す
		if (Aiter->first == Biter->first) {
			//大きい方の頻度を足す

			int Aplus, Bplus;
			double weight = NMD_Weight((int) Aiter->first.length());
			if (Aiter->second > Biter->second) {
				Aplus = Aiter->second * weight;
				Bplus = 0;
			} else {
				Aplus = 0;
				Bplus = Biter->second * weight;
			}
			if (!Bfinished) {
				Bsize += Biter->second * weight;
				H += Bplus;
				if (Biter == histgramB.end() - 1) {
					Bfinished = true;
				} else {
					Biter++;
				}
			}
			if (!Afinished) {
				Asize += Aiter->second * weight;
				H += Aplus;
				if (Aiter == histgramA.end() - 1) {
					Afinished = true;
				} else {
					Aiter++;
				}
			}
		} //Bのデータ番号のほうが小さければ、Bを進める
		else if (Aiter->first > Biter->first) {
			if (Bfinished) {
				double weight = NMD_Weight((int) Aiter->first.length());
				//Bが終わっていたらAを進める
				Asize += Aiter->second * weight;
				H += Aiter->second * weight;
				if (Aiter == histgramA.end() - 1) {
					Afinished = true;
				} else {
					Aiter++;
				}
			} else {
				double weight = NMD_Weight((int) Biter->first.length());
				Bsize += Biter->second * weight;
				H += Biter->second * weight;
				if (Biter == histgramB.end() - 1) {
					Bfinished = true;
				} else {
					Biter++;
				}
			}
		} //Aのデータ番号のほうが小さければ、Aを進める
		else {
			if (Afinished) {
				double weight = NMD_Weight((int) Biter->first.length());
				Bsize += Biter->second * weight;
				H += Biter->second * weight;
				if (Biter == histgramB.end() - 1) {
					Bfinished = true;
				} else {
					Biter++;
				}
			} else {
				double weight = NMD_Weight((int) Aiter->first.length());
				Asize += Aiter->second * weight;
				H += Aiter->second * weight;
				if (Aiter == histgramA.end() - 1) {
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

double NMD::NormalizedDictionaryDistance(
		const std::vector<std::pair<std::string, double>>& A,
		const std::vector<std::pair<std::string, double>>& B) const {

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

	double ndd = (double) ((double) (H - min_dicsize) / (double) max_dicsize);
	return ndd;
}

void NMD::SaveHistgram(std::string file_name, std::string file_class,
		std::vector<std::pair<std::string, double>>& histgram) {
	string save_path = database_path + "/" + file_class;

	struct stat st;
	if (stat(save_path.c_str(), &st) != 0) {
		mkdir(save_path.c_str(), 0775);
	}

	save_path += "/" + file_name + ".histgram";

	FILE* fp;
	fp = std::fopen(save_path.c_str(), "wb");

	int histgram_size = (int) histgram.size();
	int char_count = 0;

	//１文字目にファイル全体のchar数 size: int * 1
	for (int i = 0; i < histgram_size; ++i) {
		char_count += histgram.at(i).first.size() + 1;	//+1は\0の分

	}
	if (fwrite(&char_count, sizeof(int), 1, fp) != 1) {
		cout << "書き込みエラー" << endl;
		exit(1);
	}

	//２文字目はヒストグラムの個数 size: int * 1
	if (fwrite(&histgram_size, sizeof(int), 1, fp) != 1) {
		cout << "書き込みエラー" << endl;
		exit(1);
	}

	//３つ目のブロックは頻度の配列 size: int * histgram_size
	for (int i = 0; i < histgram_size; ++i) {
		int multiple = (int) histgram.at(i).second;
		if (fwrite(&multiple, sizeof(int), 1, fp) != 1) {
			cout << "書き込みエラー" << endl;
			exit(1);
		}
	}
	//４つ目のブロックは文字数の配列 size: int * histgram_size
	//テキスト長さ
	for (int i = 0; i < histgram_size; ++i) {
		int str_length = (int) histgram.at(i).first.size() + 1;
		if (fwrite(&str_length, sizeof(int), 1, fp) != 1) {
			cout << "書き込みエラー" << endl;
			exit(1);
		}
	}

	//５つ目のブロックは文字列 size: char * char_count
	for (int i = 0; i < histgram_size; ++i) {
		char* c;
		c = new char[histgram.at(i).first.size() + 1];

		int p = 0;
		for (auto& s : histgram.at(i).first) {
			c[p] = s;
			++p;
		}
		c[histgram.at(i).first.size()] = '\0';

		if (fwrite(c, sizeof(char), histgram.at(i).first.size() + 1, fp)
				!= histgram.at(i).first.size() + 1) {
			cout << "書き込みエラー" << endl;
			exit(1);
		}

		delete[] c;
	}

	std::fclose(fp);

}
int NMD::LoadHistgram(std::string file_path,
		std::vector<std::pair<std::string, double> >& histgram) {

	FILE *fp; //高速化のため低レベルな関数を用いる

	if ((fp = std::fopen(file_path.c_str(), "rb")) == NULL) { /* ファイルのオープン */
		printf("file open error!!\n");
		return 1;
	}
	int pre_read[2];
//1文字目はファイル全体のchar数
//2文字目はヒストグラムの個数
	if (fread(pre_read, sizeof(int), 2, fp) != 2) {
		cout << "読み込みエラー" << endl;
		exit(1);
	}

	int* file_contents_int;
	file_contents_int = new int[pre_read[1] * 2];
	char* file_contents_char;
	file_contents_char = new char[pre_read[0]];

	if ((int)fread(file_contents_int, sizeof(int), pre_read[1] * 2, fp)
			!= pre_read[1] * 2) {
		cout << "読み込みエラー" << endl;
		exit(1);
	}
	if ((int)fread(file_contents_char, sizeof(char), pre_read[0], fp)
			!= pre_read[0]) {
		cout << "読み込みエラー" << endl;
		exit(1);
	}
	int& histgram_size = pre_read[1];
	histgram.resize(histgram_size);

	for (int i = 0; i < histgram_size; ++i) {
		histgram.at(i).second = file_contents_int[i];
	}
	int char_pointer = 0;
	for (int i = 0; i < histgram_size; ++i) {
		histgram.at(i).first = &file_contents_char[char_pointer];
		char_pointer += file_contents_int[i + histgram_size];
	}

	delete[] file_contents_int;
	delete[] file_contents_char;

	std::fclose(fp);

	return 0;
}

bool NMD::hasHistgramMade(std::string file_path) {
	string file_name = GetFileName(file_path);
	string file_class = GetFileClassName(file_path);
	fs::path database(database_path);
	string histgram_path = database.string() + "/" + file_class + "/"
			+ file_name + ".histgram";
	cout << histgram_path << endl;
	return fs::exists(fs::path(histgram_path));
}

} /* namespace image_retrieval */
