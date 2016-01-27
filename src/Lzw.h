/**
 * @file Lzw.h
 *
 * @date 2015/11/19
 * @author uchinosub
 * @brief LZWを用いて文字列の圧縮と辞書の抽出を行う
 */

#ifndef LZW_H_
#define LZW_H_

#include "Dictionary.h"
#include <vector>
#include <string>
#include <map>

namespace prdc_lzw {

//フラグ処理のための定数定義
const unsigned int ARROW_EDIT_DICTIONARY = 1;
const unsigned int ARROW_EDIT_PAIR = 2;

/**
 * @brief 文字列を圧縮し、辞書を抽出する
 * @param uncompressed 圧縮する文字列
 * @param compressed 圧縮された文字列（数値）
 * @param output_dic 圧縮に用いる辞書
 * @param flags 辞書に新たな文字列を追加するなら ARROW_EDIT_DICTIONARY を指定
 */
void Compress(const std::string &uncompressed,
		std::vector<std::string> &compressed, Dictionary &output_dic,
		unsigned int flags = 0);

/**
 * @brief 文字列を圧縮し、圧縮後データ列からペアを作成する。以前に作成されたペアの探索も行う。
 * @param uncompressed 圧縮する文字列
 * @param compressed 圧縮されたデータ列(数値)
 * @param output_dic 圧縮に用いる辞書
 * @param pair 作成されたペア、もしくは探索するペア
 * @param flags ペアを作成する | 辞書に新たな文字列を追加する
 */
void CompressWithMakePair(const std::string &uncompressed,
		std::vector<std::string> &compressed, Dictionary &output_dic,
		LzwPair& pair, unsigned int flags = 0);

void CompressBoundData(const std::string &uncompressed,
		std::vector<std::string> &compressed, Dictionary &output_dic,
		BindingMap &binding_data, unsigned int flags = 0,
		unsigned int max_dicsize = -1);
void CompressOriginString(const std::string &uncompressed,
		std::vector<std::string> &compressed, Dictionary &output_dic,
		unsigned int flags = 0, unsigned int max_dicsize = -1);
int ReturnMapValue(std::map<std::string, int>& binding_data,
		std::string origin_text, int get_data);
}
#endif /* LZW_H_ */
