#pragma once

#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iomanip>
#include <fstream>
#include "SymbolTable.h"
#include "RelocationTable.h"

using namespace std;

class BinaryFile{
private:
    int mode; // 0-read, 1-write
    void* file_pointer;
    string file_name;
public:
    BinaryFile(int mode, string file_name){
        if(mode != 0 && mode != 1) exit(0);
        this->mode = mode;
        this->file_name = file_name;
    }
    void open_file();
    void close_file();
    void write_sections_content_to_file(vector<SectionContent>&);
    vector<SectionContent> read_sections_content_from_file();
    void write_reloc_tables_to_file(vector<RelocationTable>&);
    vector<RelocationTable> read_reloc_tables_from_file();
    void write_symbol_table_to_file(SymbolTable&);
    SymbolTable read_symbol_table_from_file();
    string convertToString(char*,int);
};

// file outline

/*

int number_of_sections
    {
        int number_of_lines_in_section
        int length_of_section_name without '\0'
        string section_name

        {
            number_of_bytes_in_line
            individual_bytes
        }

    }

*/