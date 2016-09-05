/*
 * MultiByteCharList.h
 *
 *  Created on: 2016/07/28
 *      Author: uchinosub
 */

#ifndef SRC_MULTIBYTECHARLIST_H_
#define SRC_MULTIBYTECHARLIST_H_

#include <vector>
#include <string>
#include <math.h>

class MultiByteCharList {
public:
	static std::vector<std::string> list;
	static int MakeList() {
		if (list.size() != 0)
			return 1;

		int max_byte = 3;
		int char_max = 255;

		list.resize(pow(char_max, max_byte));

		index = 0;
		MakeOneByteChar();
		MakeTwoByteChar();
		MakeThreeByteChar();

		list.resize(index);
		list.shrink_to_fit();
		return 0;
	}
private:
	static int index;
	static void MakeOneByteChar() {

		//128 = 0x80
		for (int i = 0; i < 128; ++i) {
			list.at(index) = std::string(1, char(i));
			++index;
		}

	}
	static void MakeTwoByteChar() {
		int start_one_byte = 194; //0xC2
		int end_one_byte = 224; //0xE0
		int start_two_byte = 128; //0x80
		int end_two_byte = 192; //0xC0

		for (int i = start_one_byte; i < end_one_byte; ++i) {
			for (int j = start_two_byte; j < end_two_byte; ++j) {
				list.at(index) = std::string( { char(i), char(j), '\0' });
				++index;
			}
		}

	}
	static void MakeThreeByteChar() {
		int start_one_byte = 224; //0xE0
		int end_one_byte = 240; //0xF0
		int start_two_byte = 128; //0x80
		int end_two_byte = 192; //0xC0
		int start_tree_byte = 128; //0x80
		int end_three_byte = 192; //0xC0

		for (int i = start_one_byte; i < end_one_byte; ++i) {
			for (int j = start_two_byte; j < end_two_byte; ++j) {
				for (int k = start_tree_byte; k < end_three_byte; ++k) {
					list.at(index) = std::string( { char(i), char(j), char(k),
							'\0' });
					++index;
				}
			}
		}

	}

};



#endif /* SRC_MULTIBYTECHARLIST_H_ */
