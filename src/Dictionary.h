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
#include <memory>



///PRDCで用いるLZW圧縮に関する名前空間
namespace prdc_lzw {

//! 辞書の最大値のデフォルト値(ファイルサイスが大きく、辞書番号が50000では収まらない場合には増やす)
const unsigned int default_max_dicsize = 50000;

//!フラグ処理のための定数定義
const unsigned int ARROW_EDIT_DICTIONARY = 1;

/**
 * @brief LZW辞書のノード
 *
 * 内部に辞書番号（辞書に何個目に追加されたか）と単語(検索用)を持つ
 */
class LzwNode {
	friend class Dictionary;
public:
	LzwNode();
	LzwNode(int d, char c);
	virtual ~LzwNode();

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
	std::vector<std::unique_ptr<LzwNode>> children; ///子ノード
private:
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

	//!辞書に登録できる単語の数
	unsigned int max_dicsize; //辞書に登録できる単語の数

	//!文字と辞書番号を関連付ける配列
	std::vector<std::string> binding;
	//!圧縮後の辞書番号配列
	std::vector<int> compressed;
	//!圧縮後文字列中にそれぞれの単語が何回存在するか
	std::vector<std::pair<std::string, int>> histgram;

	/**
	 * @brief 文字列を圧縮し、辞書を抽出する
	 * @param uncompressed 圧縮する文字列
	 * @param flags 辞書に新たな文字列を追加するなら ARROW_EDIT_DICTIONARY を指定
	 */
	std::vector<int>& Compress(const std::string& uncompressed,
			unsigned int flags = 0);

	/**
	 * @brief 圧縮後の辞書番号のヒストグラムを作成し、辞書番号を元の文字列に置き換え、ソートを行う
	 * @note ソートまで行うことに注意
	 */
	std::vector<std::pair<std::string, int>>& MakeHistgram();
private:
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

	//!辞書に単語がいくつ登録されているか(辞書番号をつける際に利用)
	int dict_size;

	//!ルートノード
	std::unique_ptr<LzwNode> root;
};

} /* namespace prdc_lzw */
#endif /* DICTIONARY_H_ */

