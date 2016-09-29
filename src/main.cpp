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
using namespace std;

int main(int argc, char* argv[]) {

	 //基底辞書作成用テキストのパス
	 	const string DATA_SET_PATH(
	 			"/home/uchinosub/git/MyResearches.ImageToText/output/101_ObjectCategories/");

//	 const string DATA_SET_PATH(
//	 "/home/uchinosub/git/MyResearches.ImageToText/output/image-orig/");

	 //	const string DATA_SET_PATH(
	 //			"/home/uchinosub/eclipse_folder/eclipse4.4/workspace/DictionaryDistanceProject/dataset/20news-18828-multibyte/");

	 //	const string DATA_SET_PATH(
	 //			"/home/uchinosub/eclipse_folder/eclipse4.4/workspace/DictionaryDistanceProject/dataset/murphy/learn/");
	 //
	 int flags = prdc::RECOMPRESSION;
	 bool use_utf8_text = false; //データセットがマルチバイト文字を含む場合はtrue
	 int loop = 10;
	 int number_of_dics = 20;
	 int number_of_testdata = 100;
	 int k = 5;

	 //PRDC_TEST(DATA_SET_PATH, flags, loop, number_of_dics, number_of_testdata);
//
//	 	PRDC_SAMEDICS_TEST(DATA_SET_PATH, flags, loop, number_of_dics,
//	 			number_of_testdata, k, use_utf8_text);

	 	NMD_TEST(DATA_SET_PATH, 100, image_retrieval::WEIGHTING_NMD);

}
