#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <locale>
#include "SymbolTable.h"
#include "RelocationTable.h"
#include "BinaryFile.h"
#include "Linker.h"

using namespace std;

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        cerr << "Arguments error" << endl;
        exit(0);
    }

    string hex_string = argv[1];
    if (hex_string != "-hex")
    {
        cerr << "Arguments error" << endl;
        exit(0);
    }

    string o_string = argv[2];
    if (o_string != "-o")
    {
        cerr << "Arguments error" << endl;
        exit(0);
    }

    string output_name = argv[3];
    vector<string> input_names;

    for(int i=4; i<argc; i++){
        input_names.push_back(argv[i]);
    }

    Linker linker;
    linker.link_files(output_name, input_names);

    return 0;
}