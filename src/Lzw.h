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

/**
 * @brief 文字列を圧縮し、辞書を抽出する
 * @param uncompressed 圧縮する文字列
 * @param compressed 圧縮された文字列（数値）
 * @param output_dic 圧縮に用いる辞書
 * @param allow_edit_dictionary 辞書に新たな文字列を追加するならtrue
 */
void Compress(const std::string &uncompressed,
		std::vector<std::string> &compressed, Dictionary &output_dic,
		bool allow_edit_dictionary = true);

/**
 * @brief 文字列を圧縮し、圧縮後データ列からペアを作成する。以前に作成されたペアの探索も行う。
 * @param uncompressed 圧縮する文字列
 * @param compressed 圧縮されたデータ列(数値)
 * @param output_dic 圧縮に用いる辞書
 * @param pair 作成されたペア、もしくは探索するペア
 * @param allow_edit_dictionary 辞書に新たな文字列を追加するならtrue
 * @param allow_edit_pair 新たなpairを登録するならtrue
 */
void CompressWithMakePair(const std::string &uncompressed,
		std::vector<std::string> &compressed, Dictionary &output_dic,
		LzwPair& pair, bool allow_edit_dictionary = true, bool allow_edit_pair =
				true);

}

#endif /* LZW_H_ */
