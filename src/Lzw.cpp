/**
 * @file Lzw.cpp
 *
 * @date 2015/11/17
 * @author uchinosub
 * @brief LZW圧縮(辞書抽出)実装ファイル
 */
#include "Lzw.h"
#include "Dictionary.h"
#include <string>

namespace prdc_lzw {

void compress(const std::string &uncompressed, std::vector<int> &compressed,
		Dictionary &output_dic) {

	std::string w;
	LzwNode* &current_node = output_dic.current_node;
	current_node = output_dic.root;

	for (std::string::const_iterator it = uncompressed.begin();
			it != uncompressed.end(); it++) {
		char c = *it;	//未圧縮の文字列から一文字取り出す
		LzwNode* q = current_node->FindChild(c);

		if (q != NULL) {
			current_node = q;
		} else {
			compressed.push_back(current_node->data);
			output_dic.AddNode(c);
			current_node = output_dic.root->FindChild(c);
		}
	}

	compressed.push_back(current_node->data);
}

}
