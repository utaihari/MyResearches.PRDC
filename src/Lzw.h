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

namespace prdc_lzw {

/**
 * @brief 文字列を圧縮し、辞書を抽出する
 * @param uncompressed 圧縮する文字列
 * @param compressed 圧縮された文字列（数値）
 * @param output_dic 出力される辞書
 */
void compress(const std::string &uncompressed, std::vector<int> &compressed,
		Dictionary &output_dic);

}
#endif /* LZW_H_ */
