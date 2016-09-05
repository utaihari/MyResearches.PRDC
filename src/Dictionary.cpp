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
#include <math.h>
#include "MultiByteCharList.h"
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
//template LzwNode<string>* LzwNode<string>::FindChild(string c) const;
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

template<>
inline LzwNode<string>* LzwNode<string>::FindChild(string c) const {
	for (int i = 0; i < (int) children.size(); ++i) {
		LzwNode<string> *tmp = children.at(i).get();
		if (tmp->content.find(c) == 0) {
			return tmp;
		}
	}
	return NULL;
}
template<typename T>
inline LzwNode<T>* LzwNode<T>::FindChild(T c) const {
	for (int i = 0; i < (int) children.size(); ++i) {
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
				256), size(256), root_char(new LzwNode<char>()),multibyte_string(false) {
	root_char->children.resize(256);
	contents.resize(256);
	for (int i = 0; i < 256; i++) {
		root_char->children.at(i) = std::make_shared<LzwNode<char> >(i,
				char(i));
		contents.at(i) = char(i);
	}
}
Dictionary::Dictionary(std::string uncompress, bool multibyte_string,
		unsigned int max_dicsize, unsigned int max_length) :
		max_dicsize(max_dicsize), max_length(max_length), multibyte_string(
				multibyte_string), root_char(new LzwNode<char>()), root_string(
				new LzwNode<string>()) {
	//もしも圧縮文字列にマルチバイト文字が入っているなら
	if (multibyte_string) {
		MultiByteCharList::MakeList();
		int num_of_char = (int) MultiByteCharList::list.size();

		dict_size = num_of_char;
		size = num_of_char;
		root_string->children.resize(num_of_char);
		contents.resize(num_of_char);

		for (int i = 0; i < num_of_char; ++i) {
			root_string->children.at(i) = std::make_shared<LzwNode<string>>(i,
					MultiByteCharList::list[i]);
			contents.at(i) = MultiByteCharList::list[i];
		}
		InnerCompressStrMultibyte(uncompress);
	} else {
		int num_of_char = 255;

		dict_size = num_of_char;
		size = num_of_char;
		root_string->children.resize(num_of_char);
		contents.resize(num_of_char);
		root_char->children.resize(256);
		contents.resize(256);
		for (int i = 0; i < 256; i++) {
			root_char->children.at(i) = std::make_shared<LzwNode<char>>(i,
					char(i));
			contents.at(i) = char(i);
		}
		InnerCompressStr(uncompress);
	}

}

Dictionary::Dictionary(std::vector<int> uncompress, unsigned int max_dicsize,
		unsigned int max_length) :
		max_dicsize(max_dicsize), max_length(max_length), dict_size(256), size(
				256), root_int(new LzwNode<int>()) {
	InnerCompress(uncompress);
}

EncodedText Dictionary::Compress(std::string& uncompressed) {

	EncodedText output;

	if (multibyte_string) {
		MakeCompressStrMultiByte(uncompressed, output);
	} else {
		MakeCompressStr(uncompressed, output);
	}

	return output;
}

void Dictionary::MakeCompressStr(const std::string& uncompressed,
		EncodedText& compressed_string) {
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
					cout << "圧縮文字列にマルチバイト文字が含まれています" << endl;
					cout << "utf-8対応版を使ってみてください" << endl;
				}
				compressed_string.push_back(current_node->get_data());
			} else {
				//現在のノードが圧縮に使用できない場合は、現在の文字列を圧縮したものを出力する
				EncodedText recursion_string = this->Compress(w);
				for (auto s : recursion_string.encoded) {
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
	compressed_string.number_of_type = contents.size();
}

void Dictionary::MakeCompressStrMultiByte(const std::string &uncompressed,
		EncodedText& compressed_string) {

	prdc_lzw::LzwNode<string>* current_node = root_string.get(); //初期位置
	unsigned int string_length = 1;
	int char_size = 0; //読み込んだ文字のバイト数

	std::string w; //複数回の圧縮で共通数字を出力するため、元のテキストを保存しておく

	for (std::string::const_iterator it = uncompressed.begin();
			it != uncompressed.end(); it += char_size) {
		unsigned char pre_lead = *it;	//未圧縮の文字列から一文字取り出す

		char_size = CharSize(pre_lead);

		string c = uncompressed.substr(distance(uncompressed.begin(), it),
				char_size);	//未圧縮の文字列から一文字取り出す
		std::string wc(w + c);

		if (current_node == NULL) {
			cout << "Character not found error." << endl;
			cout << "データセットの文字コードがutf-8ではないかもしれません" << endl;
		}

		prdc_lzw::LzwNode<string>* q = current_node->FindChild(c);

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
				EncodedText recursion_string = this->Compress(w);
				for (auto s : recursion_string.encoded) {
					compressed_string.push_back(s);
				}
			}
			//NOTE:圧縮文字列に0以下、または256以上の文字コードが入っていた場合エラーになる
			current_node = get_root_string()->FindChild(c);	//最初から検索し直す
			w = string(c);
			string_length = 1;
		}
	}
	compressed_string.push_back(current_node->get_data());
	compressed_string.number_of_type = contents.size();
}

void Dictionary::InnerCompressStr(const std::string &uncompressed) {

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
	compressed.number_of_type = contents.size();
}

void Dictionary::InnerCompressStrMultibyte(const std::string& uncompressed) {
	prdc_lzw::LzwNode<string>* current_node = root_string.get(); //初期位置

	std::string w; //複数回の圧縮で共通数字を出力するため、元のテキストを保存しておく
	int char_size = 0; //読み込んだ文字のバイト数

	unsigned int contents_size = max_dicsize + MultiByteCharList::list.size();
	//数値→文字列変換のための配列のサイズ設定
	contents.reserve(contents_size);
	for (std::string::const_iterator it = uncompressed.begin();
			it != uncompressed.end(); it += char_size) {

		unsigned char pre_lead = *it;	//未圧縮の文字列から一文字取り出す

		char_size = CharSize(pre_lead);

		string c = uncompressed.substr(distance(uncompressed.begin(), it),
				char_size);	//未圧縮の文字列から一文字取り出す
		std::string wc(w + c);
		prdc_lzw::LzwNode<string>* q = current_node->FindChild(c);
		if (q != NULL) {
			//辞書に文字列が追加されていたら
			current_node = q;	//探索ノードを一つ進める
			w = wc;
		} else {
			compressed.push_back(current_node->get_data());

			if (size < max_dicsize) {
				AddNode(current_node, c);	//current_nodeの下に文字cのノードを作成
				contents.push_back(wc);
			}

			//NOTE:圧縮文字列に0以下、または256以上の文字コードが入っていた場合エラーになる
			current_node = get_root_string()->FindChild(c);	//最初から検索し直す
			w = string(c);
		}
	}
	contents.shrink_to_fit();
	compressed.push_back(current_node->get_data());
	compressed.number_of_type = contents.size();
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

inline LzwNode<char>* Dictionary::SearchNode(std::string& keyword) {
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

inline int Dictionary::CharSize(unsigned char& cChar) const {

	int iByte;

	if ((cChar >= 0x00) && (cChar <= 0x7f)) {
		iByte = 1;
	} else if ((cChar >= 0xc2) && (cChar <= 0xdf)) {
		iByte = 2;
	} else if ((cChar >= 0xe0) && (cChar <= 0xef)) {
		iByte = 3;
	} else if ((cChar >= 0xf0) && (cChar <= 0xf7)) {
		iByte = 4;
	} else if ((cChar >= 0xf8) && (cChar <= 0xfb)) {
		iByte = 5;
	} else if ((cChar >= 0xfc) && (cChar <= 0xfd)) {
		iByte = 6;
	} else {
		iByte = 0;
	}

	if (iByte > 3 || iByte == 0) {
		cout << "length error" << endl;
	}
	return iByte;

}
void Dictionary::InnerCompress(const std::vector<int>& uncompressed) {

	prdc_lzw::LzwNode<int>* current_node = root_int.get(); //初期位置
	unsigned int string_length = 1;
	std::vector<int> w;

//複数回の圧縮で共通数字を出力するため、元のテキストを保存しておく
//数値→文字列変換のための配列のサイズ設定
	contents_int.reserve(max_dicsize + 256);

	for (auto it : uncompressed) {

		int c = it;	//未圧縮の文字列から一文字取り出す

		w.push_back(c);

		prdc_lzw::LzwNode<int>* q = current_node->FindChild(c);

		if (q != NULL) {
			//辞書に文字列が追加されていたら
			current_node = q;	//探索ノードを一つ進める
			string_length++;
		} else {
			compressed.push_back(current_node->get_data());

			if (dict_size < max_dicsize && string_length < max_length) {
				AddNode(current_node, c);	//current_nodeの下に文字cのノードを作成
				contents_int.push_back(w);
			}

			//NOTE:圧縮文字列に0以下、または256以上の文字コードが入っていた場合エラーになる
			current_node = root_int.get()->FindChild(c);	//最初から検索し直す

			if (current_node == NULL) {
				AddNode(root_int.get(), c);
				current_node = root_int.get()->FindChild(c);
			}

			w.clear();
			w.push_back(c);
			string_length = 1;
		}
	}
	std::vector<std::vector<int>>(contents_int).swap(contents_int);
	compressed.push_back(current_node->get_data());
	compressed.number_of_type = contents.size();
}
} /* namespace prdc_lzw */
