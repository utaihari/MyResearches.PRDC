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
#include "Dictionary.h"
#include "util.h"

/**
 * @brief データセットからランダムに基底辞書と学習用データを選択し、テストを行います
 * @return 正しく終了で0
 */
int PRDC_TEST(std::string dataset_path, int method_flag, int LOOP,
		int NUMBER_OF_DICS = 10, int NUMBER_OF_TEST_DATA = 30, int k = 5);
/**
 * @brief 指定したデータセットを用いてPRDCで分類を行います
 * @return
 */
int PRDC_TEST(std::string train_data_path, std::string test_data_path,
		std::string base_dics_path, int method_flag, int k = 5);
int PRDC_TEST(std::string train_data_path, std::string test_data_path,
		int NUMBER_OF_DICS, int method_flag, int k = 5);
int PRDC_SAMEDICS_TEST(std::string dataset_path, std::vector<int> method_flag,
		int LOOP, int NUMBER_OF_DICS = 1, int NUMBER_OF_TRAIN = 10, int k = 5,
		bool MULTI_BYTE_CHAR = false);

//DATABASE_PATH: データベースを保存した(する)フォルダ。　空なら自動的に作成されます
int NMD_TEST(std::string TEXT_PATH, std::string DATABASE_PATH,
		int NUMBER_OF_OUTPUT, int METHOD_FLAG);
int NMD_CLASSIFICATION(std::string TEXT_PATH, std::string DATABASE_PATH,
		int NUMBER_OF_OUTPUT, int METHOD_FLAG);
int PRDC_SIMILAR_IMAGE_DISPLAY(std::string input_image_path,
		std::string dataset_path, std::string images_path,
		std::string dics_path, int method_flag, int NUMBER_OF_DICS,
		int NUMBER_OF_OUTPUT, std::string save_file_path = "",
		std::string load_file_path = "", std::string output_folder = "output");
int NMD_Create_PR_Graph(std::string TEXT_PATH, std::string DATABASE_PATH,
		int MAX_OUTPUT, int METHOD_FLAG);
int PRDC_ImageRetrival(int argc, char* argv[]);
int PRDC_PRECISION(std::string dataset_path, std::vector<int> method_flag,
		int LOOP, int NUMBER_OF_DICS, int NUMBER_OF_TEST_DATA,
		int NUMBER_OF_OUTPUT);

int NMD_CLASSIFICATION(std::string TRAIN_DATA_PATH, std::string TEST_DATA_PATH,
		std::string DATABASE_PATH, int METHOD_FLAG, std::string savefile_name,
		int k = 5);
int PRDC_SAMEDICS_TEST(std::string train_data_path, std::string test_data_path,
		std::vector<int> flags, int LOOP, std::string savefile_name,
		int NUMBER_OF_DICS = 1, int k = 5, bool multibyte_char = false);
int ShowAllNMDDistance(std::string query_path, std::string datas_path,
		std::string DATABASE_PATH);

#endif /* SRC_TEST_H_ */
