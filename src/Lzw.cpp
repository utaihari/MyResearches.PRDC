/**
 * @file Lzw.cpp
 *
 * @date 2015/11/17
 * @author uchinosub
 * @brief LZW圧縮(辞書抽出)実装ファイル
 */
#include "Lzw.h"
#include "Dictionary.h"
#include "tools.h"
#include <string>

namespace prdc_lzw {

void Compress(const std::string &uncompressed,
		std::vector<std::string> &compressed, Dictionary &output_dic,
		bool allow_edit_dictionary) {

	LzwNode* current_node = output_dic.get_root(); //初期位置

	for (std::string::const_iterator it = uncompressed.begin();
			it != uncompressed.end(); it++) {

		char c = *it;	//未圧縮の文字列から一文字取り出す

		LzwNode* q = current_node->FindChild(c);
		if (q != NULL) {
			//辞書に文字列が追加されていたら
			current_node = q;	//探索ノードを一つ進める
		} else {
			compressed.push_back(to_string(current_node->get_data()));

			if (allow_edit_dictionary)
				output_dic.AddNode(current_node, c);//current_nodeの下に文字cのノードを作成

			//NOTE:圧縮文字列に256以上の文字コードが入っていた場合エラーになる
			current_node = output_dic.get_root()->FindChild(c);	//最初から検索し直す
		}
	}
	compressed.push_back(to_string(current_node->get_data()));
}

void CompressWithMakePair(const std::string &uncompressed,
		std::vector<std::string> &compressed, Dictionary &output_dic,
		LzwPair& pair, bool allow_edit_dictionary, bool allow_edit_pair) {

	int last_number = 0;

	LzwNode* current_node = output_dic.get_root(); //初期位置

	for (std::string::const_iterator it = uncompressed.begin();
			it != uncompressed.end(); it++) {

		char c = *it;	//未圧縮の文字列から一文字取り出す

		LzwNode* q = current_node->FindChild(c);
		if (q != NULL) {
			//辞書に文字列が追加されていたら
			current_node = q;	//探索ノードを一つ進める
		} else {
			compressed.push_back(to_string(current_node->get_data()));
			if (allow_edit_dictionary)
				output_dic.AddNode(current_node, c);//current_nodeの下に文字cのノードを作成

			std::map<std::string, int>::iterator temp = pair.SearchPair(
					last_number, current_node->get_data());
			if (temp != pair.get_end()) {
				//以前に作成したペアを発見
				compressed.push_back(
						"<" + to_string(temp->second) + ">find pair:(" + temp->first
								+ ")");
			} else {
				//新たなペアを登録
				if (allow_edit_pair) {
					pair.AddPair(last_number, current_node->get_data());
					compressed.push_back(
							"<"+to_string(pair.get_current_pair_num()) +">"+ "add new pair:(" + to_string(last_number)
									+ "," + to_string(current_node->get_data())
									+ ")");
				}
			}
			last_number = current_node->get_data();

			//NOTE:圧縮文字列に256以上の文字コードが入っていた場合エラーになる
			current_node = output_dic.get_root()->FindChild(c);	//最初から検索し直す
		}
	}
	compressed.push_back(to_string(current_node->get_data()));
}
}
