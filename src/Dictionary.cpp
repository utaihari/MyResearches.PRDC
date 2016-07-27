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
#include <iterator>     // std::back_inserter
#include <stdlib.h> //itoa
#include "util.h"

using std::cout;
using std::endl;
using std::string;

namespace prdc_lzw {

template LzwNode<char>::LzwNode();
template LzwNode<string>::LzwNode();
template LzwNode<int>::LzwNode();
template LzwNode<char>::LzwNode(int d, char c);
template LzwNode<string>::LzwNode(int d, string c);
template LzwNode<int>::LzwNode(int d, int c);
template LzwNode<char>* LzwNode<char>::FindChild(char c) const;
template LzwNode<string>* LzwNode<string>::FindChild(string c) const;
template LzwNode<int>* LzwNode<int>::FindChild(int c) const;
template void LzwNode<char>::InsertChild(int data, char c);
template void LzwNode<string>::InsertChild(int data, string c);
template void LzwNode<int>::InsertChild(int data, int c);

template<typename T>
inline LzwNode<T>::LzwNode() :
		abilable(true), data(0), content(), parent_node(NULL) {
}

template<typename T>
inline LzwNode<T>::LzwNode(int d, T c) :
		abilable(true), data(d), content(c), parent_node(NULL) {
}

template<typename T>
inline LzwNode<T>* LzwNode<T>::FindChild(T c) const {
	for (unsigned int i = 0; i < children.size(); ++i) {
		LzwNode<T> *tmp = children.at(i).get();
		if (tmp->content == c) {
			return tmp;
		}
	}
	return NULL;
}

template<typename T>
inline void LzwNode<T>::InsertChild(int data, T c) {
	children.resize(children.size() + 1);
	children.at(children.size() - 1) = std::make_shared<LzwNode<T> >(data, c);
	children.at(children.size() - 1)->parent_node = this;
}

Dictionary::Dictionary() :
		max_dicsize(default_max_dicsize), max_length(default_max_length), dict_size(
				256), size(256), root_char(new LzwNode<char>()) {
	root_char->children.resize(256);
	contents.resize(256);
	for (int i = 0; i < 256; i++) {
		root_char->children.at(i) = std::make_shared<LzwNode<char> >(char(i),
				i);
		contents.at(i) = char(i);
	}
}
Dictionary::Dictionary(std::string uncompress, bool multibyte_string,
		unsigned int max_dicsize, unsigned int max_length) :
		max_dicsize(max_dicsize), max_length(max_length), dict_size(256), size(
				256), multibyte_string(multibyte_string), root_char(
				new LzwNode<char>()), root_string(new LzwNode<string>) {

	//もしも圧縮文字列にマルチバイト文字が入っているなら
	if (multibyte_string) {
		root_string->children.resize(256);
		contents.resize(256);
		for (int i = 0; i < 256; i++) {
			root_string->children.at(i) = std::make_shared<LzwNode<string>>(
					char(i), i);
			contents.at(i) = char(i);
		}
		InnerCompressString(uncompress);
	} else {
		root_char->children.resize(256);
		contents.resize(256);
		for (int i = 0; i < 256; i++) {
			root_char->children.at(i) = std::make_shared<LzwNode<char>>(char(i),
					i);
			contents.at(i) = char(i);
		}
		InnerCompressChar(uncompress);
	}

}

Dictionary::Dictionary(std::vector<int> uncompress, unsigned int max_dicsize,
		unsigned int max_length) :
		max_dicsize(max_dicsize), max_length(max_length), dict_size(256), size(
				256), root_int(new LzwNode<int>()) {
	InnerCompress(uncompress);
}

std::vector<int> Dictionary::Compress(std::string& uncompressed) {

	std::vector<int> output;

	if (multibyte_string) {
		MakeCompressString(uncompressed, output);
	} else {
		MakeCompressChar(uncompressed, output);
	}

	return output;
}

void Dictionary::MakeCompressChar(const std::string& uncompressed,
		std::vector<int>& compressed_string) {
	prdc_lzw::LzwNode<char>* current_node = root_char.get(); //初期位置
	unsigned int string_length = 1;
	std::string w; //複数回の圧縮で共通数字を出力するため、元のテキストを保存しておく

	for (std::string::const_iterator it = uncompressed.begin();
			it != uncompressed.end(); ++it) {

		char c = *it;	//未圧縮の文字列から一文字取り出す

		std::string wc = w + c;

		prdc_lzw::LzwNode<char>* q = current_node->FindChild(c);

		if (q != NULL) {
			//辞書に文字列が追加されていたら
			current_node = q;	//探索ノードを一つ進める
			w = wc;
			string_length++;
		} else {
			if (current_node->abilable) {
				if (current_node->get_data() < 0) {
					cout << "error" << endl;
				}
				compressed_string.push_back(current_node->get_data());
			} else {
				//現在のノードが圧縮に使用できない場合は、現在の文字列を圧縮したものを出力する
				std::vector<int> recursion_string = this->Compress(w);
				for (auto s : recursion_string) {
					compressed_string.push_back(s);
				}
			}
			//NOTE:圧縮文字列に0以下、または256以上の文字コードが入っていた場合エラーになる
			current_node = get_root_char()->FindChild(c);	//最初から検索し直す
			w = std::string(1, c);
			string_length = 1;
		}
	}
	compressed_string.push_back(current_node->get_data());
}

void Dictionary::MakeCompressString(const std::string &uncompressed,
		std::vector<int>& compressed_string) {

	prdc_lzw::LzwNode<string>* current_node = root_string.get(); //初期位置
	unsigned int string_length = 1;
	std::string w; //複数回の圧縮で共通数字を出力するため、元のテキストを保存しておく

	for (int i = 0; i < (int) uncompressed.size(); ++i) {

		string c = uncompressed.substr(i, 1);	//未圧縮の文字列から一文字取り出す

		std::string wc = w + c;

		prdc_lzw::LzwNode<string>* q = current_node->FindChild(c);

		if (q != NULL) {
			//辞書に文字列が追加されていたら
			current_node = q;	//探索ノードを一つ進める
			w = wc;
			string_length++;
		} else {
			if (current_node->abilable) {
				if (current_node->get_data() < 0) {
					cout << "error" << endl;
				}
				compressed_string.push_back(current_node->get_data());
			} else {
				//現在のノードが圧縮に使用できない場合は、現在の文字列を圧縮したものを出力する
				std::vector<int> recursion_string = this->Compress(w);
				for (auto s : recursion_string) {
					compressed_string.push_back(s);
				}
			}
			//NOTE:圧縮文字列に0以下、または256以上の文字コードが入っていた場合エラーになる
			current_node = get_root_string()->FindChild(c);	//最初から検索し直す
			w = c.substr(0, 1);
			string_length = 1;
		}
	}
	compressed_string.push_back(current_node->get_data());
}

void Dictionary::InnerCompressChar(const std::string &uncompressed) {

	prdc_lzw::LzwNode<char>* current_node = root_char.get(); //初期位置
	unsigned int dicsize = 256;
	std::string w; //複数回の圧縮で共通数字を出力するため、元のテキストを保存しておく
	//数値→文字列変換のための配列のサイズ設定
	contents.reserve(max_dicsize + 256);

	for (std::string::const_iterator it = uncompressed.begin();
			it != uncompressed.end(); ++it) {

		char c = *it;	//未圧縮の文字列から一文字取り出す
		std::string wc = w + c;
		prdc_lzw::LzwNode<char>* q = current_node->FindChild(c);

		if (q != NULL) {
			//辞書に文字列が追加されていたら
			current_node = q;	//探索ノードを一つ進める
			w = wc;
		} else {
			compressed.push_back(current_node->get_data());

			if (dicsize < max_dicsize) {
				this->size++;
				dicsize++;
				AddNode(current_node, c);	//current_nodeの下に文字cのノードを作成
				contents.push_back(wc);
			}

			//NOTE:圧縮文字列に0以下、または256以上の文字コードが入っていた場合エラーになる
			current_node = get_root_char()->FindChild(c);	//最初から検索し直す
			w = std::string(1, c);
		}
	}
	contents.shrink_to_fit();
	compressed.push_back(current_node->get_data());
}

void Dictionary::InnerCompressString(const std::string& uncompressed) {
	prdc_lzw::LzwNode<string>* current_node = root_string.get(); //初期位置
	unsigned int dicsize = 256;
	std::string w; //複数回の圧縮で共通数字を出力するため、元のテキストを保存しておく
	//数値→文字列変換のための配列のサイズ設定
	contents.reserve(max_dicsize + 256);

	for (int i = 0; i < (int) uncompressed.size(); ++i) {

		string c = uncompressed.substr(i, 1);	//未圧縮の文字列から一文字取り出す
		std::string wc = w + c;
		prdc_lzw::LzwNode<string>* q = current_node->FindChild(c);

		if (q != NULL) {
			//辞書に文字列が追加されていたら
			current_node = q;	//探索ノードを一つ進める
			w = wc;
		} else {
			compressed.push_back(current_node->get_data());

			if (dicsize < max_dicsize) {
				this->size++;
				dicsize++;
				AddNode(current_node, c);	//current_nodeの下に文字cのノードを作成
				contents.push_back(wc);
			}

			//NOTE:圧縮文字列に0以下、または256以上の文字コードが入っていた場合エラーになる
			current_node = get_root_string()->FindChild(c);	//最初から検索し直す
			w = c.substr(c.length() - 1, 1);
		}
	}
	contents.shrink_to_fit();
	compressed.push_back(current_node->get_data());
}

Dictionary::~Dictionary() {
}

inline void Dictionary::AddNode(LzwNode<char>* node, char key_word) {
	node->InsertChild(dict_size, key_word);
	dict_size++;
	this->size++;
}
inline void Dictionary::AddNode(LzwNode<int>* node, int key_word) {
	node->InsertChild(dict_size, key_word);
	dict_size++;
	this->size++;
}
inline void Dictionary::AddNode(LzwNode<std::string>* node, string key_word) {
	node->InsertChild(dict_size, key_word);
	dict_size++;
	this->size++;
}

LzwNode<char>* Dictionary::SearchNode(std::string& keyword) {
	LzwNode<char>* current_node = root_char.get();
	for (auto c : keyword) {
		current_node = current_node->FindChild(c);
		if (current_node == NULL) {
			return NULL;
		}
	}
	return current_node;
}

bool Dictionary::DisableNode(std::string& key_word) {
	LzwNode<char>* tempNode = SearchNode(key_word);
	if (tempNode != NULL) {
		tempNode->abilable = false;
		this->size--;
	} else {
		return false;
	}
	return true;
}

} /* namespace prdc_lzw */

