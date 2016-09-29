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
	NMD(std::string database_filepath);
	NMD(std::vector<std::string>& data_paths, std::vector<float>& class_paths);
	NMD(std::vector<std::vector<std::pair<std::string, double>>>&data_histgrams);
	virtual ~NMD();

	std::map<std::string, float> classes;

	std::vector<std::pair<float, std::string>> FindNearest(
	std::string file_path, int k = 1) {
		return FindNearest(file_path, k,ORIGINAL_NMD);
	}

	std::vector<std::pair<float, std::string>> FindNearest(
	std::string file_path, int k = 1,int flag = ORIGINAL_NMD);
	int SetCodebook(std::string folder_path);
	int SetCodebook(std::vector<std::string>& data_paths,
	std::vector<float>& class_paths);
	void Save(std::string filename);

private:
	std::vector<std::string> data_paths;
	std::vector<float> data_classes;
	std::vector<std::vector<std::pair<std::string, double>>>data_histgrams;
	std::vector<prdc_lzw::Dictionary> data_dictionaries;

	void HistgramToString(const std::vector<std::pair<std::string, double>>&histgram,std::vector<std::string>& output);
	void StringToInt(const std::string& s,std::vector<unsigned int>& output);
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
	double NMD_Weight(const int length)const{
		return log2((double)length);
	}
};

}
	/* namespace image_retrieval */

#endif /* SRC_NMD_H_ */
