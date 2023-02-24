#pragma once

#include <iostream>
#include <regex>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <fstream>
#include "SymbolTable.h"
#include "RelocationTable.h"

using namespace std;

class Assembler
{
public:
    void beautify_line(string &str);
    bool check_if_only_label(string &str);
    bool check_if_label_plus_ins(string &str);
    string check_if_directive(string &str);
    string label_part(string &str);
    string ins_or_dir_part(string str);
    string get_first_regex_match(string str, regex reg);
    string check_if_no_arg_ins(string &str);
    string check_if_one_operand_ins(string &str);
    string check_if_one_reg_ins(string &str);
    string check_if_two_reg_ins(string str);
    string check_if_first_reg_second_operand(string &str);
    vector<string> get_all_regex_matches(string str, regex reg);
    void process_line(string str, SymbolTable &, string &current_section, unsigned char &location_counter, bool &assembling,
                      vector<vector<unsigned char>> &, vector<SectionContent> &, SectionContent &, RelocationTable &, vector<RelocationTable> &);
    void print_lines_in_hex(vector<vector<unsigned char>> &lines_in_hex);
    void print_lines_in_hex_to_file(ofstream &output_file, vector<vector<unsigned char>> &lines_in_hex);
};