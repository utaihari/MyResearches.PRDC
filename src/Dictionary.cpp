/**
 *	@file Dictionary.cpp
 *
 *	@date 2015/11/13
 *	@author uchinosub
 *	@brief 辞書クラスの実装
 */

#include "Dictionary.h"
#include <stdlib.h>
#include <stack>
#include <algorithm>
#include <iostream>
#include "util.h"

namespace prdc_lzw {

LzwNode::LzwNode() :
		data(0), content(-1) {
}
LzwNode::LzwNode(int d, char c) :
		data(d), content(c) {
}

LzwNode::~LzwNode() {
//	for (auto c : children) {
//		delete c;
//	}
}

LzwNode* LzwNode::FindChild(char c) {
	for (unsigned int i = 0; i < children.size(); ++i) {
		LzwNode* tmp = children.at(i).get();
		if (tmp->content == c) {
			return tmp;
		}
	}
	return NULL;
}

void LzwNode::InsertChild(int data, char c) {
	children.resize(children.size() + 1);
	children.at(children.size() - 1).reset(new LzwNode(data, c));

	/*	LzwNode* tmp = new LzwNode(data, c);
	 children.push_back(tmp);*/
}

Dictionary::Dictionary() :
		max_dicsize(default_max_dicsize), dict_size(256), root(new LzwNode()) {
	root->children.resize(256);
	binding.resize(256);
	for (int i = 0; i < 256; i++) {
		root->children.at(i).reset(new LzwNode(char(i), i));
		binding.at(i) = char(i);
	}
}

Dictionary::~Dictionary() {
}

void Dictionary::AddNode(LzwNode* node, char key_word) {
	node->InsertChild(dict_size, key_word);
	dict_size++;
}

LzwNode* Dictionary::SearchNode(std::string keyword) {
	LzwNode* current_node = root.get();
	for (auto c : keyword) {
		current_node = current_node->FindChild(c);
		if (current_node == NULL) {
			return NULL;
		}
	}
	return current_node;
}

std::vector<int>& Dictionary::Compress(const std::string &uncompressed,
		unsigned int flags) {

	LzwNode* current_node = root.get(); //初期位置
	unsigned int dicsize = 256;
	std::string w; //複数回の圧縮で共通数字を出力するため、元のテキストを保存しておく

	//数値→文字列変換のための配列のサイズ設定
	binding.reserve(max_dicsize + 256);

	for (std::string::const_iterator it = uncompressed.begin();
			it != uncompressed.end(); ++it) {

		char c = *it;	//未圧縮の文字列から一文字取り出す
		std::string wc = w + c;
		LzwNode* q = current_node->FindChild(c);

		if (q != NULL) {
			//辞書に文字列が追加されていたら
			current_node = q;	//探索ノードを一つ進める
			w = wc;
		} else {
			compressed.push_back(current_node->get_data());

			if (flags & ARROW_EDIT_DICTIONARY) {
				if (dicsize < max_dicsize) {
					dicsize++;
					AddNode(current_node, c);	//current_nodeの下に文字cのノードを作成
					binding.push_back(wc);
				}
			}
			//NOTE:圧縮文字列に0以下、または256以上の文字コードが入っていた場合エラーになる
			current_node = root->FindChild(c);	//最初から検索し直す
			w = std::string(1, c);
		}
	}
	std::vector<std::string>(binding).swap(binding);
	compressed.push_back(current_node->get_data());
	return compressed;
}
std::vector<std::pair<std::string, double>>& Dictionary::MakeHistgram() {
	std::vector<int> output;
	const unsigned int max_value = binding.size();
	output.resize(max_value, 0);
	histgram.resize(max_value);
	for (auto i : compressed) {
		output[i]++;
	}

	for (int i = 0; i < output.size(); ++i) {
		if (output.at(i) == 0) {
			output.at(i) = 1;
		}
	}
	for (unsigned int i = 0; i < max_value; ++i) {
	histgram.at(i) = std::make_pair(binding.at(i),
			(double) (output.at(i)));
		}

		std::sort(histgram.begin(), histgram.end());
		return histgram;
	}
	} /* namespace prdc_lzw */

