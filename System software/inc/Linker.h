#pragma once

#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iomanip>
#include <fstream>
#include "SymbolTable.h"
#include "RelocationTable.h"
#include "BinaryFile.h"
#include <algorithm>

using namespace std;

struct SymbolTableF
{
    SymbolTable symbol_table;
    int file_handle;
};

struct SectionContentF
{
    SectionContent section_content;
    int file_handle;
};

struct RelocationTableF
{
    RelocationTable relocation_table;
    int file_handle;
};

struct SymbolTableEntryF
{
    SymbolTableEntry entry;
    int file_handle;
};

class Linker
{
public:
    void link_files(string&,vector<string>&);
};