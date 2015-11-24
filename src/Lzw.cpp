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
	for (std::string::const_iterator it = uncompressed.begin();
			it != uncompressed.end(); it++) {
		char c = *it;	//未圧縮の文字列から一文字取り出す
		std::string wc = w + c;

		if (output_dic.IsExist(wc) == true) {
			w = wc;
		} else {
			compressed.push_back(output_dic.SearchNode(w)->data);
			output_dic.AddNodes(wc);
			w = std::string(1, c);
		}
	}

	if (!w.empty()){
		compressed.push_back(output_dic.SearchNode(w)->data);
	}
}

}
