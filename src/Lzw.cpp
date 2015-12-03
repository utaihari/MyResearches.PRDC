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

	LzwNode* current_node = output_dic.getRoot(); //初期位置

	for (std::string::const_iterator it = uncompressed.begin();
			it != uncompressed.end(); it++) {

		char c = *it;	//未圧縮の文字列から一文字取り出す

		LzwNode* q = current_node->FindChild(c);
		if (q != NULL) {
			//辞書に文字列が追加されていたら
			current_node = q;	//探索ノードを一つ進める
		} else {
			compressed.push_back(current_node->getData());
			output_dic.AddNode(current_node, c);	//current_nodeの下に文字cのノードを作成

			//NOTE:圧縮文字列に256以上の文字コードが入っていた場合エラーになる
			current_node = output_dic.getRoot()->FindChild(c);	//最初から検索し直す
		}
	}
	compressed.push_back(current_node->getData());
}

void compress_with_outer_dictionary(const std::string &uncompressed,
		std::vector<int> &compressed, Dictionary &input_dic) {
	LzwNode* current_node = input_dic.getRoot(); //初期位置

	for (std::string::const_iterator it = uncompressed.begin();
			it != uncompressed.end(); it++) {

		char c = *it;	//未圧縮の文字列から一文字取り出す

		LzwNode* q = current_node->FindChild(c);
		if (q != NULL) {
			//辞書に文字列が追加されていたら
			current_node = q;	//探索ノードを一つ進める
		} else {
			compressed.push_back(current_node->getData());
			current_node = input_dic.getRoot()->FindChild(c);	//最初から検索し直す
		}
	}
	compressed.push_back(current_node->getData());

}
}
