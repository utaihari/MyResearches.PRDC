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
	LzwNode(int d, char c);
	virtual ~LzwNode();

	/**
	 * @brief 文字cを子供に持っていればそれを返す
	 * @param c 探索したい文字
	 * @return 存在すればノードのポインタ、なければNULL
	 */
	LzwNode* FindChild(char c);
	void InsertChild(char c, int data);

	int data; ///辞書番号
	char content; ///格納文字
	std::vector<LzwNode*> children;
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
	void AddNode(char key_word);

	/**
	 * @brief key_wordのノードを辞書から検索し返す。
	 * @param key_word 検索する文字列
	 * @return key_wordのノード。辞書中に存在しなければNULLを返す
	 */
	LzwNode* SearchNode(std::string key_word);

	/**
	 * @brief key_wordが辞書に存在するかどうか
	 * @param key_word 検索する文字列
	 * @return 辞書中に文字列が存在すればtrue
	 */
	bool IsExist(std::string key_word);

	LzwNode* current_node;
	LzwNode* root;

private:
	///辞書に単語がいくつ登録されているか(辞書番号をつける際に利用)
	int dict_size;

};

} /* namespace prdc_lzw */
#endif /* DICTIONARY_H_ */

