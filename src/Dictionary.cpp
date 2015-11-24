/**
 *	@file Dictionary.cpp
 *
 *	@date 2015/11/13
 *	@author uchinosub
 *	@brief 辞書クラスの実装
 */

#include "Dictionary.h"

namespace prdc_lzw {

Dictionary::Dictionary() :
		dict_size(256) {

	for (int i = 0; i < 256; i++) {
		LzwNode* temp = new LzwNode(i);
		nodes[std::string(1, i)] = temp;
	}
}

Dictionary::~Dictionary() {
	for (auto iter = nodes.begin(); iter != nodes.end(); iter++) {
		delete iter->second;
	}
	nodes.clear();
}

LzwNode::LzwNode() {
	data = 0;
}
LzwNode::LzwNode(int d) :
		data(d) {
}

LzwNode::~LzwNode() {
	// TODO Auto-generated destructor stub
}

LzwNode* Dictionary::SearchNode(std::string key_word){
	if(nodes.count(key_word) == 0){
		return NULL;
	}
	return nodes[key_word];
}

void Dictionary::AddNodes(std::string key_word) {
	nodes[key_word] = new LzwNode(dict_size);
	dict_size++;
}

bool Dictionary::IsExist(std::string key_word) {
	if(nodes.count(key_word) == 0){
		return false;
	}
	return true;
}

} /* namespace prdc_lzw */




