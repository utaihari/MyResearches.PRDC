/*
 * PRDC_ImageRetrival.cpp
 *
 *  Created on: 2016/09/21
 *      Author: uchinosub
 */

#include"../TEST.h"
#include<string>

using namespace std;
int PRDC_ImageRetrival(int argc,char* argv[]){
	string input_image_path = argv[1];
	string dataset_path = argv[2];
	string image_folder_path = argv[3];
	int flag = atoi(argv[4]);
	int flags;
	switch (flag) {
	case 0:
		flags = prdc::PRDC_BASE;
		break;
	case 1:
		flags = prdc::RECOMPRESSION;
		break;
	default:
		flags = prdc::PRDC_BASE;
		break;
	}
	int NUMBER_OF_DICS = atoi(argv[5]);
	int NUMBER_OF_OUTPUT = atoi(argv[6]);
	string save_file = "";
	string load_file = "";

	if (argc > 7) {
		save_file = argv[7];
	}
	if (argc > 8) {
		load_file = argv[8];
	}

	PRDC_SIMILAR_IMAGE_DISPLAY(input_image_path, dataset_path,
			image_folder_path, flags, NUMBER_OF_DICS, NUMBER_OF_OUTPUT,
			save_file, load_file);
	return 0;
}
