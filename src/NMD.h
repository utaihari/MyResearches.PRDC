/*
 * NMD.h
 *
 *  Created on: 2016/08/03
 *      Author: uchinosub
 */

#ifndef SRC_NMD_H_
#define SRC_NMD_H_
#include <string>
#include <vector>
#include <math.h>
#include "Dictionary.h"

namespace image_retrieval {
const int ORIGINAL_NMD = 0x0001;
const int WEIGHTING_NMD = 0x0002;
const int NDD = 0x0004;

const std::vector<int> METHOD_ARRAY = { ORIGINAL_NMD, WEIGHTING_NMD, NDD };
const std::vector<std::string> METHOD_NAME_ARRAY = { "ORIGINAL_NMD",
		"WEIGHTING_NMD", "NDD" };
/**
 * @note NDDは,NMDとの比較用に作成しています。
 * @note NDD単体で動作させたい場合でも、NMD用のデータも作成するため、僅かに低速になります。
 */
class NMD {
public:
	NMD();
	~NMD();
	NMD(std::string database_folderpath);

	std::vector<std::string> classes;

	std::vector<std::pair<float, std::string>> FindNearest(
			std::string file_path, int k = 1) {
		return FindNearest(file_path, k, ORIGINAL_NMD);
	}

	std::vector<std::pair<float, std::string>> FindNearest(
			std::string file_path, int k = 1, int flag = ORIGINAL_NMD);
	int SetCodebook(std::string folder_path);
	void Save(std::string filename);

private:
	std::string database_path;

	std::vector<std::string> histgram_paths;
	std::vector<float> data_classes;

	/**
	 * @note 高速化のため　"テキスト長さ 頻度 テキスト内容"の順に記述しています
	 * @param histgram
	 * @param output
	 */
	void HistgramToString(const std::vector<std::pair<std::string, double>>&histgram,
			std::vector<std::string>& output);
	void StringToInt(const std::string& s,std::vector<int>& output);
	double NormalizedMultisetDistance(
			const std::vector<std::pair<std::string, double>>& histgramA,
			const std::vector<std::pair<std::string, double>>& histgramB) const;
	double NormalizedMultisetDistanceWeighted(
			const std::vector<std::pair<std::string, double>>& histgramA,
			const std::vector<std::pair<std::string, double>>& histgramB) const;
	double NormalizedDictionaryDistance(
			const std::vector<std::pair<std::string, double>>& histgramA,
			const std::vector<std::pair<std::string, double>>& histgramB) const;
	void HistgramZeroToOne(std::vector<std::pair<std::string, double>>& histgram) const {
		for (auto& h : histgram) {
			if (h.second == 0.0) {
				h.second = 1.0;
			}
		}
	}
	double NMD_Weight(const int length)const {
		return sqrt((double)length);
	}
	void SaveHistgram(std::string file_name,std::string file_class,
			std::vector<std::pair<std::string, double>>& histgram);
	int LoadHistgram(std::string file_path,
			std::vector<std::pair<std::string, double>>& histgram);
	bool hasHistgramMade(std::string file_path);
};

}
/* namespace image_retrieval */

#endif /* SRC_NMD_H_ */
