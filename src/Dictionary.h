/**
 * @file Dictionary.h
 * @brief LZW辞書クラス定義ファイル
 * @date 2015/11/13
 * @author: uchinosub
 */

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include <map>
#include <string>

///PRDCで用いるLZW圧縮に関する名前空間
namespace prdc_lzw {

/**
 * @brief LZW辞書のノード
 *
 * 内部に辞書番号（辞書に何個目に追加されたか）を持つ
 */
class LzwNode {
public:
	LzwNode();
	LzwNode(int);
	virtual ~LzwNode();

	///辞書番号
	int data;
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

	/**
	 * @brief 辞書に文字列を追加する
	 * @param key_word 追加する文字列
	 */
	void AddNodes(std::string key_word);

	/**
	 * @brief key_wordのノードを辞書から検索し返す。
	 * @param key_word 検索する文字列
	 * @return key_wordのノード。辞書中に存在しなければNULLを返す
	 */
	LzwNode* SearchNord(std::string key_word);

	/**
	 * @brief key_wordが辞書に存在するかどうか
	 * @param key_word 検索する文字列
	 * @return 辞書中に文字列が存在すればtrue
	 */
	bool IsExist(std::string key_word);



private:
	///辞書に単語がいくつ登録されているか
	int dict_size;

	///辞書を構成するノード（それぞれ一つの文字列に対し１つの値を持つ）
	std::map<std::string, LzwNode*> nodes;
};


} /* namespace prdc_lzw */
#endif /* DICTIONARY_H_ */

