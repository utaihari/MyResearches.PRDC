/*
 * TEST.h
 *
 *  Created on: 2016/07/19
 *      Author: uchinosub
 */

#ifndef SRC_TEST_H_
#define SRC_TEST_H_

#include <string>
#include "PRDC.h"
#include "NMD.h"

int PRDC_TEST(std::string dataset_path, int method_flag, int LOOP,
		int NUMBER_OF_DICS = 10, int NUMBER_OF_TEST_DATA = 30, int k = 5);
int PRDC_SAMEDICS_TEST(std::string dataset_path, int method_flag, int LOOP,
		int NUMBER_OF_DICS = 10, int NUMBER_OF_TEST_DATA = 30, int k = 5,
		bool MULTI_BYTE_CHAR = false);
int NMD_TEST(std::string TEXT_PATH,std::string DATABASE_PATH, int NUMBER_OF_OUTPUT, int METHOD_FLAG);
int NMD_CLASSIFICATION(std::string TEXT_PATH, std::string DATABASE_PATH,
		int NUMBER_OF_OUTPUT, int METHOD_FLAG);
int PRDC_SIMILAR_IMAGE_DISPLAY(std::string input_image_path,
		std::string dataset_path, std::string images_path, int method_flag,
		int NUMBER_OF_DICS, int NUMBER_OF_OUTPUT, std::string save_file_path =
				"", std::string load_file_path = "", std::string output_folder =
				"output");
int PRDC_ImageRetrival(int argc, char* argv[]);

#endif /* SRC_TEST_H_ */
