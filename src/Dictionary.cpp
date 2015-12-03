/**
 *	@file Dictionary.cpp
 *
 *	@date 2015/11/13
 *	@author uchinosub
 *	@brief 辞書クラスの実装
 */

#include "Dictionary.h"
#include "tools.h"
#include <stdlib.h>
namespace prdc_lzw {

LzwNode::LzwNode() :
		data(0), content() {
}
LzwNode::LzwNode(int d, char c) :
		data(d), content(c) {
}

LzwNode::~LzwNode() {
	for (auto c : children) {
		delete c;
	}
}

LzwNode* LzwNode::FindChild(char c) {
	for (auto child : children) {
		LzwNode* tmp = child;
		if (tmp->content == c) {
			return tmp;
		}
	}
	return NULL;
}

void LzwNode::InsertChild(int data, char c) {
	LzwNode* tmp = new LzwNode(data, c);
	children.push_back(tmp);
}

Dictionary::Dictionary() :
		dict_size(256), current_pair(0) {
	root = new LzwNode();

	root->children.resize(256);
	for (int i = 0; i < 256; i++) {
		root->children.at(i) = new LzwNode(char(i), i);
	}
}

Dictionary::~Dictionary() {
	delete root;
}

void Dictionary::AddNode(LzwNode* node, char key_word) {
	node->InsertChild(dict_size, key_word);
	dict_size++;
}

void Dictionary::AddPair(int a, int b) {
	std::string temp = to_string(a) + "," + to_string(b);
	pair[temp] = current_pair;
	current_pair++;
}
std::map<std::string, int>::iterator Dictionary::SearchPair(int a, int b) {
	std::string temp = to_string(a) + "," + to_string(b);
	auto i = pair.find(temp);
	return i;
}

} /* namespace prdc_lzw */

