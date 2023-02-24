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

struct RelocationTableEntry
{
    unsigned short offset; // offset where we need to write 
    string type; // ETF_PC_16 or ETF_16
    string symbol; // symbol name
    unsigned short addend;
    string section_of_relocation_table; // name of section for relocation table 
};

class RelocationTable
{
public:
    vector<RelocationTableEntry> all_entries;
    void add_new_entry(RelocationTableEntry entry);
    RelocationTableEntry *get_entry(string name);
    vector<RelocationTableEntry> *get_all_entries() { return &all_entries; }
    void print_relocation_table();
    void print_relocation_table_to_file(ofstream& output_file);
    string section_name;
};