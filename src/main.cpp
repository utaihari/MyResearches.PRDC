/**
 *  @file main.cpp
 *
 *  @date 2015/11/19
 *  @author uchinosub
 */
#include "MultiByteCharList.h"
#include "util.h"
#include "TEST.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <string.h>
#include <stdio.h>
#include <boost/filesystem.hpp>
#include "Debug.h"
using namespace std;
namespace fs = boost::filesystem;

int main(int argc, char* argv[]) {

	//基底辞書作成用テキストのパス
//	const string DATA_SET_PATH(
//			"/home/uchinosub/git/MyResearches.ImageToText/output/101_ObjectCategories/");
//	const string DATA_SET_PATH(
//			"/home/uchinosub/git/MyResearches.ImageToText/output/Edited-101_ObjectCategories-PRDC/");

//	const string DATA_SET_PATH(
//			"/home/uchinosub/Dataset/dataset_texted/image-orig/");
	const string DATA_SET_PATH(
			"/home/uchinosub/Dataset/URC_TS_Archive_rotate/");
//	const string DATA_SET_PATH(
//			"/home/uchinosub/git/MyResearches.ImageToText/output/Edited-image-orig-PRDC/");
//	const string DATA_SET_PATH(
//			"/home/uchinosub/git/MyResearches.ImageToText/output/30Level_Noised_Image/image-orig/");
//	const string DATA_SET_PATH(
//			"/home/uchinosub/eclipse_folder/eclipse4.4/workspace/DictionaryDistanceProject/dataset/20news-18828-multibyte/");

//	const string DATA_SET_PATH(
//			"/home/uchinosub/eclipse_folder/eclipse4.4/workspace/DictionaryDistanceProject/dataset/murphy/learn/");
//
	bool use_utf8_text = false; //データセットがマルチバイト文字を含む場合はtrue
	int loop = 5;
	int number_of_dics = 2;
	int number_of_train = 80;
	int k = 1;

//	NMD_TEST(DATA_SET_PATH, "./NMD_DATABASE/image-orig", 100,
//			image_retrieval::ORIGINAL_NMD);
//	NMD_TEST(DATA_SET_PATH, "./NMD_DATABASE/image-orig", 100,
//			image_retrieval::WEIGHTING_NMD);

//	string text;
//	vector<string> data_names;
//	data_names.push_back("Coffee");
//	data_names.push_back("FISH");
//	data_names.push_back("LargeKitchenAppliances");
//	data_names.push_back("FaceAll");
//	data_names.push_back("CBF");
//
//	for (auto data_name : data_names) {
//		map<int, int> length_histgram;
//		prdc_util::IncludedFilePaths paths(
//				"/home/uchinosub/Dataset/URC_TS_Archive/" + data_name
//						+ "_TRAIN/", { ".txt" });
//
//		for (auto path : paths.included_file_paths) {
//			prdc_util::FilePathToString(path, text);
//
//			prdc_lzw::Dictionary dic(text);
//
//			for (auto c : dic.contents) {
//				length_histgram[c.length()]++;
//			}
//		}
//		cout << data_name << endl;
//
//		for (auto c : length_histgram) {
//			cout << c.first << ", "
//					<< (double) c.second
//							/ (double) paths.included_file_paths.size() << endl;
//		}
//	}

//	string path = "/home/uchinosub/Dataset/URC_TS_Archive15/Car_TRAIN/2/45.txt";
//	string text;
//	prdc_util::FilePathToString(path, text);
//
//	prdc_lzw::Dictionary dic(text);
//
////	for (auto c : dic.contents) {
////		cout << c << endl;
////	}
//	auto histgram = prdc_util::MakeHistgram(dic);
//	cout << path << endl;
////	cout << "encoded: " << endl;
////	for (auto c : dic.compressed.encoded) {
////		cout << dic.contents[c] << flush;
////	}
////
////	cout << endl;
//
//	for (auto c : histgram) {
//		cout << c.first << " ... " << c.second << endl;
//	}
//	string nmd_database = "/home/uchinosub/Dataset/NMD_DATABASE/";
//	string data_name = "Car";
//
//	ShowAllNMDDistance(
//			"/home/uchinosub/Dataset/URC_TS_Archive125/Car_TEST/2/41.txt",
//			DATA_SET_PATH + data_name + "_TRAIN", nmd_database + data_name);

	prdc_util::IncludedFilePaths classes(
			"/home/uchinosub/ownCloud/UCR_TS_Archive_2015", { "" });
	int i = 0;
	for (auto data_name : classes.classes) {
		i++;
		if (i < 69) {
			continue;
		}
		data_name = "rotate_" + data_name;

		string nmd_database = "/home/uchinosub/Dataset/NMD_DATABASE/";
//	string data_name = "ECG200";
#ifdef DEBUG_OUTPUT
//		cout << data_name << endl;
#endif
		cout << data_name << ", " << flush;
		PRDC_SAMEDICS_TEST(DATA_SET_PATH + data_name + "_TRAIN",
				DATA_SET_PATH + data_name + "_TEST", { prdc::PRDC_BASE,
						prdc::RECOMPRESSION }, loop, data_name, number_of_dics,
				k, use_utf8_text);
//		cout << ", " << flush;
		NMD_CLASSIFICATION(DATA_SET_PATH + data_name + "_TRAIN",
				DATA_SET_PATH + data_name + "_TEST", nmd_database + data_name,
				image_retrieval::ORIGINAL_NMD, data_name, k);
		cout << ", " << flush;
		NMD_CLASSIFICATION(DATA_SET_PATH + data_name + "_TRAIN",
				DATA_SET_PATH + data_name + "_TEST", nmd_database + data_name,
				image_retrieval::WEIGHTING_NMD, data_name, k);
		cout << endl;
	}

}

