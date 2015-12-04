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
 * @param output_dic 出力される辞書
 */
void compress(const std::string &uncompressed, std::vector<std::string> &compressed,
		Dictionary &output_dic);

/**
 * @brief 指定辞書で文字列を圧縮する。
 * @param uncompressed 圧縮する文字列
 * @param compressed 圧縮された文字列（数値）
 * @param input_dic 圧縮に用いる辞書
 */
void compress_with_outer_dictionary(const std::string &uncompressed,
		std::vector<std::string> &compressed, Dictionary &input_dic);

void AddPair(int a, int b,int pair_number,std::map<std;;string,int>& pair);
std::map<std::string,int>::iterator SearchPair(int a, int b);

}



#endif /* LZW_H_ */
