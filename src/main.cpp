/**
 *  @file main.cpp
 *
 *  @date 2015/11/19
 *  @author uchinosub
 */

#include "TEST.h"
using namespace std;
int main(int argc, char* argv[]) {
	//基底辞書作成用テキストのパス
//	const string DATA_SET_PATH(
//			"/home/uchinosub/git/MyResearches.ImageToText/output/101_ObjectCategories/");

	const string DATA_SET_PATH(
				"/home/uchinosub/git/MyResearches.ImageToText/output/image-orig/");

//	const string DATA_SET_PATH(
//			"/home/uchinosub/eclipse_folder/eclipse4.4/workspace/DictionaryDistanceProject/dataset/20news-18828/");

	int flags = prdc::PAIR_MULTIPLE;
	int loop = 10;
	int number_of_dics = 10;
	int number_of_testdata = 300;

	//PRDC_TEST(DATA_SET_PATH, flags, loop, number_of_dics, number_of_testdata);
	PRDC_SAMEDICS_TEST(DATA_SET_PATH, flags, loop, number_of_dics,
			number_of_testdata);

	return 0;
}
