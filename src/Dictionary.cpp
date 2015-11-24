/**
 *	@file Dictionary.cpp
 *
 *	@date 2015/11/13
 *	@author uchinosub
 *	@brief 辞書クラスの実装
 */

#include "Dictionary.h"
#include <stdlib.h>
namespace prdc_lzw {

LzwNode::LzwNode() :
		data(0), content() {
}
LzwNode::LzwNode(int d,char c) :
		data(d), content(c) {
}

LzwNode::~LzwNode() {
	for(int i = 0; i < children.size(); i++){
		delete children.at(i);
	}
}

LzwNode* LzwNode::FindChild(char c){
	for(int i = 0; i < children.size(); i++){
		LzwNode* tmp = children.at(i);
		if(tmp->content == c){
			return tmp;
		}
	}
	return NULL;
}

void LzwNode::InsertChild(char c , int data){
	LzwNode* tmp = new LzwNode(data, c);
	children.push_back(tmp);
}

Dictionary::Dictionary() :
		dict_size(256) {
	root = new LzwNode();
	current_node = NULL;

	root->children.resize(256);
	for (int i = 0; i < 256; i++) {
		root->children.at(i) = new LzwNode(char(i), i);
	}
}

Dictionary::~Dictionary() {
	delete root;
}


void Dictionary::AddNode(char key_word) {
	current_node->InsertChild(key_word, dict_size);
	dict_size++;
}

} /* namespace prdc_lzw */

