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
#include <algorithm>
#include <map>
#include <memory>
#include <math.h>

///PRDCで用いるLZW圧縮に関する名前空間
namespace prdc_lzw {

//! 辞書の最大値のデフォルト値
const unsigned int default_max_dicsize = -1;
const unsigned int default_max_length = -1;

//!フラグ処理のための定数定義
const unsigned int DONOT_EDIT_DICTIONARY = 1;
const unsigned int ARROW_EDIT_DICTIONARY = 2;

/**
 * @brief LZW辞書のノード
 * @note T = char, int or string を想定しています
 * 内部に辞書番号（辞書に何個目に追加されたか）と単語(検索用)を持つ
 */
template<typename T = char>
class LzwNode {
	friend class Dictionary;
public:
	LzwNode();
	LzwNode(int d, T c);

	//! 圧縮に利用するかどうか
	bool abilable;

	//辞書番号を返す
	int get_data() {
		return data;
	}
	//ノードが持つ文字を返す
	T get_content() {
		return content;
	}
	LzwNode<T>* get_parent_node(){
		return parent_node;
	}

	/**
	 * @brief ノードの完全な文字列を返す
	 * @param current_node
	 * @return
	 * @note 未完成
	 */
	std::string get_strings(LzwNode<char>* current_node) {
		std::string strings;
		//ルートノードに達するまで親をたどる
		while (current_node->get_parent_node() != NULL) {
			strings += current_node->content;
			current_node = current_node->parent_node;
		}
		//下から辿ったので文字列を逆にする
		std::reverse(strings.begin(), strings.end());
		return strings;
	}

	/**
	 * @brief 文字cを子供に持っていればそれを返す
	 * @param c 探索したい文字
	 * @return 存在すればノードのポインタ、なければNULL
	 */
	LzwNode<T>* FindChild(T c) const;
	std::vector<std::shared_ptr<LzwNode<T>>>children; ///子ノード
private:
	/**
	 * @brief 子ノードの挿入
	 *
	 * @note 辞書クラスからのみ呼び出される\n(外部から直接子ノードを挿入させない)
	 * @param data 辞書番号
	 * @param c 格納文字
	 */
	void InsertChild(int data, T c);

	int data; ///辞書番号
	T content;///格納文字
	LzwNode<T>* parent_node;///親ノード

};

/**
 * @brief 符号化されたテキスト
 */
class EncodedText {
public:
	EncodedText() :
			number_of_type(0) {
	}
	EncodedText(const EncodedText &et) {
		encoded = et.encoded;
		number_of_type = et.number_of_type;
	}
	std::vector<int> encoded;
	int number_of_type;

	/**
	 * @brief 符号化文字列の長さ
	 * @note 符号化文字列の長さを変更するときはここを変えてください
	 * @return
	 */
	int length() {
		return encoded.size();
	}
	int size() {
		return length();
	}
	void push_back(int i) {
		encoded.push_back(i);
	}
	int at(int i) {
		return encoded.at(i);
	}
};

/**
 * @brief LZW辞書クラス
 *
 * 文字列の登録と検索ができる
 * @author uchino
 * @note 通常のLZW、マルチバイト用のLZW、圧縮符号列を再度圧縮する用のLZWが入っています
 */

class Dictionary {
public:
	Dictionary();
	Dictionary(std::string uncompress, bool multi_byte_char = false,
			unsigned int max_dicsize = default_max_dicsize,
			unsigned int max_length = default_max_length);
	Dictionary(std::vector<int> uncompress, unsigned int max_dicsize =
			default_max_dicsize, unsigned int max_length = default_max_length);
	virtual ~Dictionary();

	LzwNode<int>* get_root_int() {
		return root_int.get();
	}
	LzwNode<char>* get_root_char() const {
		return root_char.get();
	}
	LzwNode<std::string>* get_root_string() const {
		return root_string.get();
	}
	int get_size() {
		return this->size;
	}

	//!辞書に登録できる単語の数
	unsigned int max_dicsize;
	//!辞書に登録できる単語の長さの上限
	unsigned int max_length;

	//!文字と辞書番号を関連付ける配列（数値列の圧縮では利用できません）
	std::vector<std::string> contents;
	//!数値列と辞書番号を関連付ける配列
	std::vector<std::vector<int>> contents_int;

	//!圧縮後の辞書番号配列
	EncodedText compressed;
	//!圧縮後文字列中にそれぞれの単語が何回存在するか(全て足すと１になるように正規化)
	std::vector<std::pair<std::string, double>> histgram;
	std::map<int, double> histgram_int;

	/**
	 * @brief 辞書中の指定したノードに文字を追加する
	 * @param key_word 追加する文字
	 * @param node 文字を追加するノード
	 * @note 辞書番号の管理のため直接ノードに子を追加させない
	 */
	void AddNode(LzwNode<char>* node, char key_word);
	void AddNode(LzwNode<int>* node, int key_word);
	void AddNode(LzwNode<std::string>* node, std::string key_word);

	/**
	 * @brief key_wordのノードを辞書から検索し返す。
	 * @param key_word 検索する文字列
	 * @return key_wordのノード。辞書中に存在しなければNULLを返す
	 */
	LzwNode<char>* SearchNode(std::string& key_word);

	/**
	 * @brief 引数の文字列を、辞書に登録された単語のみを用いて圧縮する。
	 * @param uncompressed 圧縮する文字列
	 */
	EncodedText Compress(std::string &uncompressed);

	/**
	 * @brief 引数の文字列を圧縮に使用しないようにする
	 * @param key_word
	 * @return key_wordが辞書中に存在しなければfalse
	 */
	bool DisableNode(std::string& key_word);

private:
	//!辞書に単語がいくつ登録されているか(辞書番号をつける際に利用)
	unsigned int dict_size;
	//!実際に圧縮に利用できる単語の数
	unsigned int size;
	bool multibyte_string;

	//文字列を圧縮する（通常版）
	void InnerCompressStr(const std::string &uncompressed);
	void MakeCompressStr(const std::string &uncompressed,
			EncodedText& compressed_string);
	//!ルートノード
	std::shared_ptr<LzwNode<char>> root_char;

	//Multibyte文字用
	void InnerCompressStrMultibyte(const std::string &uncompressed);
	void MakeCompressStrMultiByte(const std::string &uncompressed,
			EncodedText& compressed_string);
	//!ルートノード
	std::shared_ptr<LzwNode<std::string>> root_string;

	//再圧縮用
	void InnerCompress(const std::vector<int> &uncompressed);
	//!ルートノード
	std::shared_ptr<LzwNode<int>> root_int;

	/**
	 * @brief マルチバイト文字が何バイトで表現されているかを調べる
	 * @param c マルチバイト文字の１バイト目
	 * @return マルチバイト文字が何バイトで構成されているか
	 */
	int CharSize(unsigned char& c) const;
};

}
/* namespace prdc_lzw */
#endif /* DICTIONARY_H_ */

