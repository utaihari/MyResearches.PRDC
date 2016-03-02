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
#include "util.h"

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
		max_dicsize(-1), dict_size(256) {
	root = new LzwNode();

	root->children.resize(256);
	binding.resize(256);
	for (int i = 0; i < 256; i++) {
		root->children.at(i) = new LzwNode(char(i), i);
		binding.at(i) = char(i);
	}
}

Dictionary::~Dictionary() {
	delete root;
}

void Dictionary::AddNode(LzwNode* node, char key_word) {
	node->InsertChild(dict_size, key_word);
	dict_size++;
}

LzwNode* Dictionary::SearchNode(std::string keyword) {
	LzwNode* current_node = root;
	for (auto c : keyword) {
		current_node = current_node->FindChild(c);
		if (current_node == NULL) {
			return NULL;
		}
	}
	return current_node;
}

void LzwPair::AddPair(int a, int b) {
	std::string temp = to_string(a) + "," + to_string(b);
	pair[temp] = current_pair_num;
	current_pair_num++;
}
std::map<std::string, int>::iterator LzwPair::SearchPair(int a, int b) {
	std::string temp = to_string(a) + "," + to_string(b);
	auto i = pair.find(temp);
	return i;
}
int BindingMap::AddMap(std::string text) {
	int return_value;
	auto result = this->find(text);

	if (result == this->end()) {
		//textが登録されていない時の処理
		this->insert(std::make_pair(text, id));
		return_value = id;
		id++;
	} else {
		return_value = result->second;
	}
	return return_value;
}

} /* namespace prdc_lzw */

