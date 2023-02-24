#pragma once

#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iomanip>
#include <fstream>

#define HEX(x) \
    setw(2) << setfill('0') << hex << (int)(x)

#define HEX_LINE(x) \
    setw(4) << setfill('0') << hex << (int)(x)

using namespace std;


struct SectionContent
{
    string name;
    vector<vector<unsigned char>> content;
};

struct SymbolTableEntry
{
    unsigned short value;
    unsigned short size;
    string type;    // SCTN, NOTYP
    string bind;    // LOC, GLOB, EXT, UND, EQU
    string section; // something or UND ( section in witch symbol is located if entry is of type section then name and section are the same )
    string name;    // name of symbol or section
};

struct SectionEntry
{
    SymbolTableEntry entry;
    int file_handle;
    unsigned short start_address;
    bool address_defined = false;
};

class SymbolTable
{
private:
    vector<SymbolTableEntry> all_entries;

public:
    void add_new_section(string name);
    void add_new_entry(SymbolTableEntry entry);
    SymbolTableEntry *get_entry(string name);
    void print_symbol_table();
    void print_symbol_table_to_file(ofstream& output_file);
    vector<SymbolTableEntry> &get_all_entries() { return all_entries; }
};