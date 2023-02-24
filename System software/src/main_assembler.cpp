#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <locale>
#include "assembler.h"
#include "SymbolTable.h"
#include "RelocationTable.h"
#include "BinaryFile.h"

using namespace std;

int main(int argc, char **argv)
{

	if (argc < 4 || strcmp(argv[1], "-o") != 0)
	{
		cerr << "Invalid input" << endl;
		exit(0);
	}

	string input_name = argv[3];
	string output_name = argv[2];

	fstream newfile;
	Assembler assembler;
	newfile.open(input_name, ios::in);
	vector<vector<string>> lines;

	SymbolTable smt;
	vector<RelocationTable> rtv;
	RelocationTable rt;
	string current_section = "UND";
	unsigned char location_counter = 0;
	bool assembling = true;
	vector<SectionContent> sections_content;

	SectionContent section_content;
	section_content.name = "UND";

	if (newfile.is_open())
	{
		vector<string> out;
		string line;
		vector<vector<unsigned char>> lines_in_hex;

		while (getline(newfile, line) && assembling)
		{
			if (regex_search(line, regex("^ *$")))
				continue; // skiping empty lines

			out.clear();
			assembler.beautify_line(line);
			assembler.process_line(line, smt, current_section, location_counter, assembling, lines_in_hex, sections_content, section_content, rt, rtv);
		}

		//assembler.print_lines_in_hex(lines_in_hex);

		newfile.close();
	}

	// if there is a symbol that is not local and is not extern or global
	// or if there is a global symbol that wasnt defined that is an error

	for (SymbolTableEntry entry : smt.get_all_entries())
	{
		if (entry.bind == "UND")
		{
			cerr << "Error: Symbol " << entry.name << " is not global or extern or local." << endl;
			exit(0);
		}
		if (entry.bind == "GLOB" && entry.section == "UND")
		{
			cerr << "Error: Symbol " << entry.name << " is marked global but never defined." << endl;
			exit(0);
		}
	}

	for (RelocationTable &rt : rtv)
	{
		for (RelocationTableEntry &entry : rt.all_entries)
		{
			SymbolTableEntry *e = smt.get_entry(entry.symbol);
			if (e->bind == "LOC")
			{
				entry.symbol = e->section;
				entry.addend = e->value;
				
				if(e->type == "EQU"){
					entry.type = "EQU";
				}

			}
		}
	}


	//cout << endl;

	ofstream output_text_file;
	output_text_file.open(output_name + ".txt");

	for (SectionContent s : sections_content)
	{
		//cout << "Section " << s.name << " data: " << endl;
		output_text_file << "Section " << s.name << " data: " << endl;

		//assembler.print_lines_in_hex(s.content);
		assembler.print_lines_in_hex_to_file(output_text_file, s.content);

		output_text_file << endl;
		//cout << endl;
	}

	//cout << endl;
	output_text_file << endl;

	//smt.print_symbol_table();
	smt.print_symbol_table_to_file(output_text_file);

	//cout << endl;
	output_text_file << endl;

	for (RelocationTable &rt : rtv)
	{
		//cout << "Relocation table for section " << rt.section_name << ":" << endl;
		output_text_file << "Relocation table for section " << rt.section_name << ":" << endl;

		//rt.print_relocation_table();
		rt.print_relocation_table_to_file(output_text_file);

		//cout << endl;
		output_text_file << endl;
	}

	output_text_file.close();

	BinaryFile bf(1, output_name);

	bf.open_file();
	bf.write_sections_content_to_file(sections_content);
	bf.write_symbol_table_to_file(smt);
	bf.write_reloc_tables_to_file(rtv);
	bf.close_file();

	return 0;
}
