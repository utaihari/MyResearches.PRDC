/*
 * NMD.cpp
 *
 *  Created on: 2016/08/03
 *      Author: uchinosub
 */

#include "NMD.h"
#include "util.h"

using std::string;
using std::vector;
using std::cout;
using std::endl;
namespace image_retrieval {

NMD::NMD() :
		classes(), data_paths(), data_classes(), data_histgrams() {
	// TODO 自動生成されたコンストラクター・スタブ

}

NMD::~NMD() {
	// TODO Auto-generated destructor stub
}

std::vector<std::pair<float, std::string> > NMD::FindNearest(
		std::string file_path, int k, int flag) {
	string input;

	std::vector<std::pair<float, std::string> > output(k); //pair<class,file_path>
	std::vector<std::pair<double, int>> nmd(data_histgrams.size());

	//検索対象データのヒストグラム作成 ここから
	prdc_util::FilePathToString(file_path, input);
	prdc_lzw::Dictionary input_dic(input);
	prdc_util::MakeHistgram(input_dic);
	HistgramZeroToOne(input_dic.histgram);
	auto& input_histgram = input_dic.histgram;
	//検索対象データのヒストグラム作成 ここまで

	if (flag & ORIGINAL_NMD) {
		//全データとのNMDを測る ここから
#pragma omp parallel for
		for (int i = 0; i < (int) data_histgrams.size(); ++i) {
			nmd.at(i).first = NormalizedMultisetDistance(data_histgrams.at(i),
					input_histgram);
			nmd.at(i).second = i; //何番目の配列か記録しておく
		}
	} else if (flag & WEIGHTING_NMD) {
		//全データとのNMDを測る ここから
#pragma omp parallel for
		for (int i = 0; i < (int) data_histgrams.size(); ++i) {
			nmd.at(i).first = NormalizedMultisetDistanceWeighted(
					data_histgrams.at(i), input_histgram);
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
		output.at(i).second = data_paths.at(nmd.at(i).second);
	}

	return output;
}

int NMD::SetCodebook(std::string folder_path) {
	prdc_util::GetEachFilePathsAndClasses(folder_path, data_paths, data_classes,
			classes);

	data_histgrams.resize(data_paths.size());

//それぞれのデータのヒストグラム作成
#pragma omp parallel for
	for (int i = 0; i < (int) data_paths.size(); ++i) {
		string text;
		prdc_util::FilePathToString(data_paths.at(i), text);
		prdc_lzw::Dictionary dic(text);
		prdc_util::MakeHistgram(dic);

		HistgramZeroToOne(dic.histgram);

		data_histgrams.at(i) = std::vector<std::pair<std::string, double>>(
				dic.histgram);

		if (i % 10 == 0) {
			cout << "i: " << i << endl;
		}
	}

	return 0;
}

NMD::NMD(std::string database_filepath) {
}

NMD::NMD(std::vector<std::string>& datas, std::vector<float>& d_classes) {
	SetCodebook(datas, d_classes);

}

int NMD::SetCodebook(std::vector<std::string>& datas,
		std::vector<float>& d_classes) {
	copy(datas.begin(), datas.end(), back_inserter(data_paths));
	copy(d_classes.begin(), d_classes.end(), back_inserter(data_classes));

	data_histgrams.resize(data_paths.size());

	int debug = 1;
//それぞれのデータのヒストグラム作成
#pragma omp parallel for
	for (int i = 0; i < (int) data_paths.size(); ++i) {
		string text;
		prdc_util::FilePathToString(data_paths.at(i), text);
		prdc_lzw::Dictionary dic(text);
		prdc_util::MakeHistgram(dic);

		HistgramZeroToOne(dic.histgram);

		data_histgrams.at(i) = std::vector<std::pair<std::string, double>>(
				dic.histgram);

		if (debug % 10 == 0) {
			cout << data_histgrams.size() << "個中 " << debug << "個圧縮完了" << endl;
		}
		debug++;
	}

	return 0;
}

void NMD::Save(std::string filename) {
	cout << "saving" << endl;
	vector<string> text;
	HistgramToString(data_histgrams.at(0), text);
	cout << text.at(0) << endl;
}

void NMD::HistgramToString(
		const std::vector<std::pair<std::string, double> >& histgram,
		std::vector<std::string>& output) {
	output.resize(histgram.size());

	for (int i = 0; i < (int) histgram.size(); ++i) {
		string o_str = output.at(i);
		vector<unsigned int> str_int;
		StringToInt(histgram.at(i).first, str_int);
		o_str = std::to_string(histgram.at(i).second) + ", ";
		for (auto i : str_int) {
			o_str += std::to_string(i) + " ";
		}
	}
}

void NMD::StringToInt(const std::string& s, std::vector<unsigned int>& output) {
	cout << "s: " << s << endl;
	for (std::string::const_iterator it = s.begin(); it != s.end(); it++) {
		unsigned int pre_lead = *it;	//文字列から一文字取り出す
		//cout << "char" << pre_lead << endl;
		cout << "int " << pre_lead << endl;
		output.push_back(pre_lead);
	}
}

NMD::NMD(std::vector<std::vector<std::pair<std::string, double> > >& datas) :
		data_histgrams(datas) {
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
		//Aのデータ番号とBのデータ番号が同じだったら頻度が大きい方を足す
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

} /* namespace image_retrieval */
