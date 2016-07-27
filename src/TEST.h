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

int PRDC_TEST(std::string dataset_path, int method_flag, int LOOP,
		int NUMBER_OF_DICS = 10, int NUMBER_OF_TEST_DATA = 30, int k = 5);
int PRDC_SAMEDICS_TEST(std::string dataset_path, int method_flag, int LOOP,
		int NUMBER_OF_DICS = 10, int NUMBER_OF_TEST_DATA = 30, int k = 5);

#endif /* SRC_TEST_H_ */
