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

void compress(const std::string &uncompressed,
		std::vector<std::string> &compressed, Dictionary &output_dic) {

	int last_number = 0;
	int pair_num =0;

	LzwNode* current_node = output_dic.getRoot(); //初期位置

	for (std::string::const_iterator it = uncompressed.begin();
			it != uncompressed.end(); it++) {

		char c = *it;	//未圧縮の文字列から一文字取り出す

		LzwNode* q = current_node->FindChild(c);
		if (q != NULL) {
			//辞書に文字列が追加されていたら
			current_node = q;	//探索ノードを一つ進める
		} else {
			compressed.push_back(to_string(current_node->getData()));
			output_dic.AddNode(current_node, c);	//current_nodeの下に文字cのノードを作成

			std::map<std::string,int>::iterator temp = output_dic.SearchPair(last_number,
					current_node->getData());
			if (temp != output_dic.pair.end()) {
				compressed.push_back(temp->first +":"+ to_string(temp->second));
			} else {
				output_dic.AddPair(last_number, current_node->getData());
				compressed.push_back(
						"(" + to_string(last_number) + ","
								+ to_string(current_node->getData()) + "):"+to_string(pair_num));
				pair_num++;
			}
			last_number = current_node->getData();

			//NOTE:圧縮文字列に256以上の文字コードが入っていた場合エラーになる
			current_node = output_dic.getRoot()->FindChild(c);	//最初から検索し直す


		}
	}
	compressed.push_back(to_string(current_node->getData()));
}

void compress_with_outer_dictionary(const std::string &uncompressed,
		std::vector<std::string> &compressed, Dictionary &input_dic) {
	LzwNode* current_node = input_dic.getRoot(); //初期位置

	int last_number = 0;

	for (std::string::const_iterator it = uncompressed.begin();
			it != uncompressed.end(); it++) {

		char c = *it;	//未圧縮の文字列から一文字取り出す

		LzwNode* q = current_node->FindChild(c);
		if (q != NULL) {
			//辞書に文字列が追加されていたら
			current_node = q;	//探索ノードを一つ進める
		} else {
			compressed.push_back(to_string(current_node->getData()));
			current_node = input_dic.getRoot()->FindChild(c);	//最初から検索し直す

			auto temp = input_dic.SearchPair(last_number,
					current_node->getData());
			if (temp != input_dic.pair.end()) {
				compressed.push_back(temp->first +":"+ to_string(temp->second));
			}
			last_number = current_node->getData();
		}

	}
	compressed.push_back(to_string(current_node->getData()));
}

void AddPair(int a,int b,int pair_number,std::map<std;;string,int>& pair){
	std::string temp = to_string(a) + "," + to_string(b);
	pair[temp] = pair_number;
}
std::map<std::string,int>::iterator SearchPair(int a, int b);
}
