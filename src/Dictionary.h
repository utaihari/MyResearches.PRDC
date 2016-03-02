/**
 * @file Dictionary.h
 * @brief LZW辞書クラス定義ファイル
 * @date 2015/11/13
 * @author: uchinosub
 */

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include <vector>
#include <string>
#include <map>

///PRDCで用いるLZW圧縮に関する名前空間
namespace prdc_lzw {

/**
 * @brief LZW辞書のノード
 *
 * 内部に辞書番号（辞書に何個目に追加されたか）と単語(検索用)を持つ
 */
class LzwNode {
	friend class Dictionary;
public:
	int get_data() {
		return data;
	}
	char get_content() {
		return content;
	}

	/**
	 * @brief 文字cを子供に持っていればそれを返す
	 * @param c 探索したい文字
	 * @return 存在すればノードのポインタ、なければNULL
	 */
	LzwNode* FindChild(char c);
	std::vector<LzwNode*> children; ///子ノード
private:
	LzwNode();
	LzwNode(int d, char c);
	virtual ~LzwNode();

	/**
	 * @brief 子ノードの挿入
	 *
	 * @note 辞書クラスからのみ呼び出される\n(外部から直接子ノードを挿入させない)
	 * @param data 辞書番号
	 * @param c 格納文字
	 */
	void InsertChild(int data, char c);

	int data; ///辞書番号
	char content; ///格納文字

};

/**
 * @brief LZW辞書クラス
 *
 * 文字列の登録と検索ができる
 * @author uchino
 */
class Dictionary {
public:
	Dictionary();
	virtual ~Dictionary();
	LzwNode* get_root() {
		return root;
	}

	/**
	 * @brief 辞書中の指定したノードに文字を追加する
	 * @param key_word 追加する文字
	 * @param node 文字を追加するノード
	 * @note 辞書番号の管理のため直接ノードに子を追加させない
	 */
	void AddNode(LzwNode* node, char key_word);

	/**
	 * @brief key_wordのノードを辞書から検索し返す。
	 * @param key_word 検索する文字列
	 * @return key_wordのノード。辞書中に存在しなければNULLを返す
	 */
	LzwNode* SearchNode(std::string key_word);
	unsigned int max_dicsize;//辞書に登録できる単語の数
	std::vector<std::string> binding;
	void OutputBinding(std::map<int,std::string>& output);

private:
	///辞書に単語がいくつ登録されているか(辞書番号をつける際に利用)
	int dict_size;
	LzwNode* root; ///ルートノード
};

class LzwPair {
public:
	LzwPair() :
			current_pair_num(0) {
	}
	;
	void AddPair(int a, int b);
	std::map<std::string, int>::iterator SearchPair(int a, int b);
	std::map<std::string, int>::iterator get_end() {
		return pair.end();
	}
	int get_current_pair_num() {
		return current_pair_num;
	}
private:
	std::map<std::string, int> pair;
	int current_pair_num;
};

class BindingMap: public std::map<std::string, int> {
public:
	BindingMap() :
			id(256) {
		for (int i = 0; i < 256; i++){
			this->insert(std::make_pair(std::string(1, i),i));
		}
	}
	int AddMap(std::string text);
private:
	int id;
};

} /* namespace prdc_lzw */
#endif /* DICTIONARY_H_ */

