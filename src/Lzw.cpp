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
#include <ctype.h>

namespace prdc_lzw {

void Compress(const std::string &uncompressed,
		std::vector<std::string> &compressed, Dictionary &output_dic,
		unsigned int flags) {

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

			if (flags & ARROW_EDIT_DICTIONARY)
				output_dic.AddNode(current_node, c);//current_nodeの下に文字cのノードを作成

			//NOTE:圧縮文字列に256以上の文字コードが入っていた場合エラーになる
			current_node = output_dic.get_root()->FindChild(c);	//最初から検索し直す
		}
	}
	compressed.push_back(to_string(current_node->get_data()));
}

void CompressWithMakePair(const std::string &uncompressed,
		std::vector<std::string> &compressed, Dictionary &output_dic,
		LzwPair& pair, unsigned int flags) {

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
			if (flags & ARROW_EDIT_DICTIONARY)
				output_dic.AddNode(current_node, c);//current_nodeの下に文字cのノードを作成

			std::map<std::string, int>::iterator temp = pair.SearchPair(
					last_number, current_node->get_data());
			if (temp != pair.get_end()) {
				//以前に作成したペアを発見
				compressed.push_back(
						":" + to_string(temp->second) + " (" + temp->first
								+ ") <-find_pair");
			} else {
				//新たなペアを登録
				if (flags & ARROW_EDIT_PAIR) {
					pair.AddPair(last_number, current_node->get_data());
					compressed.push_back(
							":" + to_string(pair.get_current_pair_num()) + " ("
									+ to_string(last_number) + ","
									+ to_string(current_node->get_data())
									+ ") <-add_pair");
				}
			}
			last_number = current_node->get_data();

			//NOTE:圧縮文字列に256以上の文字コードが入っていた場合エラーになる
			current_node = output_dic.get_root()->FindChild(c);	//最初から検索し直す
		}
	}
	compressed.push_back(to_string(current_node->get_data()));
}
void CompressBoundData(const std::string &uncompressed,
		std::vector<std::string> &compressed, Dictionary &output_dic,
		BindingMap &binding_data, unsigned int flags,
		unsigned int max_dicsize) {

	LzwNode* current_node = output_dic.get_root(); //初期位置
	std::string w; //複数回の圧縮で共通数字を出力するため、元のテキストを保存しておく
	unsigned int dicsize = 0;

	for (std::string::const_iterator it = uncompressed.begin();
			it != uncompressed.end(); it++) {

		char c = *it;	//未圧縮の文字列から一文字取り出す
		std::string wc = w + c;

		LzwNode* q = current_node->FindChild(c);
		if (q != NULL) {
			//辞書に文字列が追加されていたら
			current_node = q;	//探索ノードを一つ進める
			w = wc;
		} else {
			int return_value = binding_data[w];
			compressed.push_back(to_string(return_value));
			if (flags & ARROW_EDIT_DICTIONARY) {
				if (dicsize < max_dicsize) {
					binding_data.AddMap(wc);
					dicsize++;
					output_dic.AddNode(current_node, c);//current_nodeの下に文字cのノードを作成
				}
			}
			//NOTE:圧縮文字列に256以上の文字コードが入っていた場合エラーになる
			current_node = output_dic.get_root()->FindChild(c);	//最初から検索し直す
			w = std::string(1, c);
		}
	}
	if(!w.empty()){
	compressed.push_back(to_string(binding_data[w]));
	}
}
}
