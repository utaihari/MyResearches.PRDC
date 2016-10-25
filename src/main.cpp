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
using namespace std;

int main(int argc, char* argv[]) {

	//基底辞書作成用テキストのパス
//	const string DATA_SET_PATH(
//			"/home/uchinosub/git/MyResearches.ImageToText/output/101_ObjectCategories/");
//	const string DATA_SET_PATH(
//			"/home/uchinosub/git/MyResearches.ImageToText/output/Edited-101_ObjectCategories-PRDC/");

//	const string DATA_SET_PATH(
//			"/home/uchinosub/git/MyResearches.ImageToText/output/image-orig/");
	const string DATA_SET_PATH(
			"/home/uchinosub/git/MyResearches.ImageToText/output/Edited-image-orig-PRDC/");
//	const string DATA_SET_PATH(
//			"/home/uchinosub/git/MyResearches.ImageToText/output/30Level_Noised_Image/image-orig/");
//	const string DATA_SET_PATH(
//			"/home/uchinosub/eclipse_folder/eclipse4.4/workspace/DictionaryDistanceProject/dataset/20news-18828-multibyte/");

//	const string DATA_SET_PATH(
//			"/home/uchinosub/eclipse_folder/eclipse4.4/workspace/DictionaryDistanceProject/dataset/murphy/learn/");
//
	int flags = prdc::RECOMPRESSION;
	bool use_utf8_text = false; //データセットがマルチバイト文字を含む場合はtrue
	int loop = 1;
	int number_of_dics = 20;
	int number_of_testdata = 100;
	int k = 5;

//PRDC_TEST(DATA_SET_PATH, flags, loop, number_of_dics, number_of_testdata);
	PRDC_TEST(DATA_SET_PATH + "train/", DATA_SET_PATH + "test/",
			DATA_SET_PATH + "base_dics/", flags);
//
//	PRDC_SAMEDICS_TEST(DATA_SET_PATH, flags, loop, number_of_dics,
//			number_of_testdata, k, use_utf8_text);

//	NMD_CLASSIFICATION(DATA_SET_PATH, "./NMD_DATABASE", 5,
//			image_retrieval::ORIGINAL_NMD);
//	NMD_CLASSIFICATION(DATA_SET_PATH, "./NMD_DATABASE", 5,
//			image_retrieval::WEIGHTING_NMD);
//	NMD_TEST(DATA_SET_PATH, "./NMD_DATABASE", 100,
//			image_retrieval::ORIGINAL_NMD);
//	NMD_TEST(DATA_SET_PATH, "./NMD_DATABASE", 100,
//			image_retrieval::WEIGHTING_NMD);

}
