/**
 *	@file Dictionary.cpp
 *
 *	@date 2015/11/13
 *	@author uchinosub
 *	@brief 辞書クラスの実装
 */

#include "Dictionary.h"
#include <stack>
#include <algorithm>
#include <iostream>
#include "util.h"

namespace prdc_lzw {

LzwNode::LzwNode() :
		abilable(true), data(0), content(-1) {
}
LzwNode::LzwNode(int d, char c) :
		abilable(true), data(d), content(c) {
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
	children.at(children.size() - 1) = std::make_shared<LzwNode>(data, c);
	children.at(children.size() - 1)->parent_node = this;
}

Dictionary::Dictionary() :
		max_dicsize(default_max_dicsize), max_length(default_max_length), dict_size(
				256), size(256), root(new LzwNode()) {
	root->children.resize(256);
	contents.resize(256);
	for (int i = 0; i < 256; i++) {
		root->children.at(i) = std::make_shared<LzwNode>(char(i), i);
		contents.at(i) = char(i);
	}
}
Dictionary::Dictionary(std::string uncompress, unsigned int max_dicsize,
		unsigned int max_length) :
		max_dicsize(max_dicsize), max_length(max_length), dict_size(256), size(
				256), root(new LzwNode()) {
	root->children.resize(256);
	contents.resize(256);
	for (int i = 0; i < 256; i++) {
		root->children.at(i) = std::make_shared<LzwNode>(char(i), i);
		contents.at(i) = char(i);
	}
	InnerCompress(uncompress);
}

std::vector<int> Dictionary::Compress(const std::string &uncompressed) {

	std::vector<int> compressed_string;
	prdc_lzw::LzwNode* current_node = root.get(); //初期位置
	unsigned int string_length = 1;
	std::string w; //複数回の圧縮で共通数字を出力するため、元のテキストを保存しておく

	for (std::string::const_iterator it = uncompressed.begin();
			it != uncompressed.end(); ++it) {

		char c = *it;	//未圧縮の文字列から一文字取り出す
		std::string wc = w + c;
		prdc_lzw::LzwNode* q = current_node->FindChild(c);

		if (q != NULL) {
			//辞書に文字列が追加されていたら
			current_node = q;	//探索ノードを一つ進める
			w = wc;
			string_length++;
		} else {
			if (current_node->abilable) {
				compressed_string.push_back(current_node->get_data());
			} else {
				//現在のノードが圧縮に使用できない場合は、現在の文字列を圧縮したものを出力する
				std::vector<int> recursion_string = this->Compress(w);
				for (auto s : recursion_string) {
					compressed_string.push_back(s);
				}
			}
			//NOTE:圧縮文字列に0以下、または256以上の文字コードが入っていた場合エラーになる
			current_node = get_root()->FindChild(c);	//最初から検索し直す
			w = std::string(1, c);
			string_length = 1;
		}
	}
	compressed_string.push_back(current_node->get_data());
	return compressed_string;
}
void Dictionary::InnerCompress(const std::string &uncompressed) {

	prdc_lzw::LzwNode* current_node = root.get(); //初期位置
	unsigned int dicsize = 256;
	unsigned int string_length = 1;
	std::string w; //複数回の圧縮で共通数字を出力するため、元のテキストを保存しておく
	//数値→文字列変換のための配列のサイズ設定
	contents.reserve(max_dicsize + 256);

	for (std::string::const_iterator it = uncompressed.begin();
			it != uncompressed.end(); ++it) {

		char c = *it;	//未圧縮の文字列から一文字取り出す
		std::string wc = w + c;
		prdc_lzw::LzwNode* q = current_node->FindChild(c);

		if (q != NULL) {
			//辞書に文字列が追加されていたら
			current_node = q;	//探索ノードを一つ進める
			w = wc;
			string_length++;
		} else {
			compressed.push_back(current_node->get_data());

			if (dicsize < max_dicsize && string_length < max_length) {
				this->size++;
				dicsize++;
				AddNode(current_node, c);	//current_nodeの下に文字cのノードを作成
				contents.push_back(wc);
			}

			//NOTE:圧縮文字列に0以下、または256以上の文字コードが入っていた場合エラーになる
			current_node = get_root()->FindChild(c);	//最初から検索し直す
			w = std::string(1, c);
			string_length = 1;
		}
	}
	std::vector<std::string>(contents).swap(contents);
	compressed.push_back(current_node->get_data());
}

Dictionary::~Dictionary() {
}

void Dictionary::AddNode(LzwNode* node, char key_word) {
	node->InsertChild(dict_size, key_word);
	dict_size++;
	this->size++;
}

LzwNode* Dictionary::SearchNode(std::string& keyword) {
	LzwNode* current_node = root.get();
	for (auto c : keyword) {
		current_node = current_node->FindChild(c);
		if (current_node == NULL) {
			return NULL;
		}
	}
	return current_node;
}

bool Dictionary::DisableNode(std::string& key_word) {
	LzwNode* tempNode = SearchNode(key_word);
	if (tempNode != NULL) {
		tempNode->abilable = false;
		this->size--;
	} else {
		return false;
	}
	return true;
}

} /* namespace prdc_lzw */

