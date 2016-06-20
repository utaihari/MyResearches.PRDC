/**
 *  @file main.cpp
 *
 *  @date 2015/11/19
 *  @author uchinosub
 */
#include "Dictionary.h"
#include <string>
#include <vector>
#include <iostream>
using namespace std;
using namespace prdc_lzw;

int main() {
	string A = "abcabcabcabc";
	string B = "abccbaabccba";

	/*Make dictionary.*/
	Dictionary DicA(A);
	Dictionary DicB(B);

	/*Show DicA contents.*/
	cout << "--DicA-contents--" << endl;
	/* Contents 0~255 is special character on LZWcompression. */
	for (int i = 256; i < (int) DicA.binding.size(); ++i) {
		if (DicA.SearchNode(DicA.binding[i])->abilable) {
			cout << DicA.binding[i] << endl;
		}
	}

	/*Show DicB contents.*/
	cout << "--DicB-contents--" << endl;
	/* Contents 0~255 is special character on LZWcompression. */
	for (int i = 256; i < (int) DicB.binding.size(); ++i) {
		if (DicB.SearchNode(DicB.binding[i])->abilable) {
			cout << DicB.binding[i] << endl;
		}
	}

	/* Delete DicA contents in the DicB */
	for (int i = 256; i < (int) DicA.binding.size(); ++i) {
		if (DicA.SearchNode(DicA.binding[i])->abilable) {
			LzwNode* find_node = DicB.SearchNode(DicA.binding[i]);
			if (find_node != NULL) {
				find_node->abilable = false;
			}
		}
	}

	/*ReShow DicB contents.*/
	cout << "--DicB-contents--" << endl;
	/* Contents 0~255 is special character on LZWcompression. */
	for (int i = 256; i < (int) DicB.binding.size(); ++i) {
		if (DicB.SearchNode(DicB.binding[i])->abilable) {
			cout << DicB.binding[i] << endl;
		}
	}

	/* Compress textA by DicB */
	vector<int> textA_compressed_by_DicB = DicB.Compress(A);

	/* Show textA length after compression by DicB */
	cout << "textA length compression by DicB" << endl;
	cout << textA_compressed_by_DicB.size() << endl;

	return 0;
}
