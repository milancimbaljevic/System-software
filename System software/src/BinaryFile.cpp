#include "BinaryFile.h"

void BinaryFile::open_file()
{
    if (mode == 0)
    {
        // read
        file_pointer = new ifstream(file_name, ios::in | ios::binary);
    }
    else
    {
        // write
        file_pointer = new ofstream(file_name, ios::out | ios::binary);
    }
}

void BinaryFile::close_file()
{
    if (mode == 0)
    {
        ((ifstream *)file_pointer)->close();
    }
    else
    {
        ((ofstream *)file_pointer)->close();
    }
}

void BinaryFile::write_sections_content_to_file(vector<SectionContent> &sections_content)
{
    ofstream *file = (ofstream *)file_pointer;

    int number_of_sections = sections_content.size();
    file->write((char *)&number_of_sections, sizeof(int));

    for (SectionContent &section : sections_content)
    {
        int sz = section.name.size() + 1;
        int nl = section.content.size();
        string name = section.name;
        file->write((char *)&nl, sizeof(int)); // number of lines in section
        file->write((char *)&sz, sizeof(int)); // size of section_name string including zero terminating char

        for (int i = 0; i < section.name.size() + 1; i++)
        {
            file->write((char *)&section.name[i], sizeof(char)); // bytes of string
        }

        for (vector<unsigned char> v : section.content)
        {
            int num_of_bytes_in_line = v.size();
            file->write((char *)&num_of_bytes_in_line, sizeof(int)); // number of bytes in line
            for (char c : v)
            {
                file->write((char *)&c, sizeof(char));
            }
        }
    }
}

string BinaryFile::convertToString(char *a, int size)
{
    int i;
    string s = "";
    for (i = 0; i < size; i++)
    {
        s = s + a[i];
    }
    return s;
}

vector<SectionContent> BinaryFile::read_sections_content_from_file()
{
    ifstream *file = (ifstream *)file_pointer;
    vector<SectionContent> sections;

    int number_of_sections;
    file->read((char *)&number_of_sections, sizeof(int));

    for (int i = 0; i < number_of_sections; i++)
    {
        int number_of_lines_in_section;
        int length_of_section_name;
        string section_name;

        file->read((char *)&number_of_lines_in_section, sizeof(int));
        file->read((char *)&length_of_section_name, sizeof(int));

        char *n = new char[length_of_section_name];

        for (int x = 0; x < length_of_section_name; x++)
        {
            file->read((char *)&n[x], sizeof(char)); // bytes of string
        }

        SectionContent section;
        section.name = convertToString(n, length_of_section_name);

        for (int j = 0; j < number_of_lines_in_section; j++)
        {
            int number_of_bytes_in_line;
            file->read((char *)&number_of_bytes_in_line, sizeof(int));

            vector<unsigned char> line;

            for (int z = 0; z < number_of_bytes_in_line; z++)
            {
                int byte;
                file->read((char *)&byte, sizeof(char));
                line.push_back(byte);
            }

            section.content.push_back(line);
        }

        sections.push_back(section);
    }
    return sections;
}

void BinaryFile::write_symbol_table_to_file(SymbolTable &smt)
{
    ofstream *file = (ofstream *)file_pointer;

    int number_of_entries_in_symbol_table = smt.get_all_entries().size();
    file->write((char *)&number_of_entries_in_symbol_table, sizeof(int));

    for (SymbolTableEntry &entry : smt.get_all_entries())
    {
        int type_size = entry.type.size();
        int bind_size = entry.bind.size();
        int section_size = entry.section.size();
        int name_size = entry.name.size();

        file->write((char *)&entry.value, sizeof(unsigned short));
        file->write((char *)&entry.size, sizeof(unsigned short));

        file->write((char *)&type_size, sizeof(int));
        file->write((char *)&bind_size, sizeof(int));
        file->write((char *)&section_size, sizeof(int));
        file->write((char *)&name_size, sizeof(int));

        for (char &c : entry.type)
        {
            file->write((char *)&c, sizeof(char));
        }

        for (char &c : entry.bind)
        {
            file->write((char *)&c, sizeof(char));
        }

        for (char &c : entry.section)
        {
            file->write((char *)&c, sizeof(char));
        }

        for (char &c : entry.name)
        {
            file->write((char *)&c, sizeof(char));
        }
    }
}

SymbolTable BinaryFile::read_symbol_table_from_file()
{
    ifstream *file = (ifstream *)file_pointer;

    int number_of_entries_in_symbol_table;
    file->read((char *)&number_of_entries_in_symbol_table, sizeof(int));

    SymbolTable smt;
    SymbolTableEntry entry;

    for (int i = 0; i < number_of_entries_in_symbol_table; i++)
    {
        file->read((char *)&entry.value, sizeof(unsigned short));
        file->read((char *)&entry.size, sizeof(unsigned short));

        int type_size;
        int bind_size;
        int section_size;
        int name_size;

        file->read((char *)&type_size, sizeof(int));
        file->read((char *)&bind_size, sizeof(int));
        file->read((char *)&section_size, sizeof(int));
        file->read((char *)&name_size, sizeof(int));

        char *a;

        a = new char[type_size];
        for (int i = 0; i < type_size; i++)
        {
            file->read((char *)&a[i], sizeof(char));
        }
        string type = convertToString(a, type_size);

        a = new char[bind_size];
        for (int i = 0; i < bind_size; i++)
        {
            file->read((char *)&a[i], sizeof(char));
        }
        string bind = convertToString(a, bind_size);

        a = new char[section_size];
        for (int i = 0; i < section_size; i++)
        {
            file->read((char *)&a[i], sizeof(char));
        }
        string section = convertToString(a, section_size);

        a = new char[name_size];
        for (int i = 0; i < name_size; i++)
        {
            file->read((char *)&a[i], sizeof(char));
        }
        string name = convertToString(a, name_size);

        entry.type = type;
        entry.bind = bind;
        entry.section = section;
        entry.name = name;

        smt.add_new_entry(entry);
    }

    return smt;
}

void BinaryFile::write_reloc_tables_to_file(vector<RelocationTable> &rtv)
{
    ofstream *file = (ofstream *)file_pointer;
    int number_of_relocation_tables = rtv.size();
    file->write((char *)&number_of_relocation_tables, sizeof(int));

    for (RelocationTable &rt : rtv)
    {
        int length_of_section_name = rt.section_name.size();
        file->write((char *)&length_of_section_name, sizeof(int));
        for (char &c : rt.section_name)
        {
            file->write((char *)&c, sizeof(char));
        }

        int number_of_entries_in_relocation_table = rt.all_entries.size();
        file->write((char *)&number_of_entries_in_relocation_table, sizeof(int));

        for (RelocationTableEntry &entry : rt.all_entries)
        {
            unsigned short offset = entry.offset;
            int length_of_string_type = entry.type.size();
            string type = entry.type;
            int length_of_string_symbol = entry.symbol.size();
            string symbol = entry.symbol;
            unsigned short addend = entry.addend;
            int length_of_string_relocation_table = entry.section_of_relocation_table.size();
            string section_of_relocation_table = entry.section_of_relocation_table;

            file->write((char *)&offset, sizeof(unsigned short));
            file->write((char *)&length_of_string_type, sizeof(int));

            for (char &c : type)
            {
                file->write((char *)&c, sizeof(char));
            }

            file->write((char *)&length_of_string_symbol, sizeof(int));

            for (char &c : symbol)
            {
                file->write((char *)&c, sizeof(char));
            }

            file->write((char*)&addend, sizeof(unsigned short));
            file->write((char *)&length_of_string_relocation_table, sizeof(int));

            for (char &c : section_of_relocation_table)
            {
                file->write((char *)&c, sizeof(char));
            }
        }
    }
}

vector<RelocationTable> BinaryFile::read_reloc_tables_from_file()
{
    ifstream *file = (ifstream *)file_pointer;
    vector<RelocationTable> rtv;

    int number_of_relocation_tables;
    file->read((char *)&number_of_relocation_tables, sizeof(int));

    for (int x = 0; x < number_of_relocation_tables; x++)
    {
        RelocationTable rt;
        RelocationTableEntry entry;

        int length_of_section_name;
        file->read((char *)&length_of_section_name, sizeof(int));
        char *str = new char[length_of_section_name];
        for (int i = 0; i < length_of_section_name; i++)
            file->read((char *)&str[i], sizeof(char));
        string section_name = convertToString(str, length_of_section_name);

        rt.section_name = section_name;

        int number_of_entries_in_relocation_table;
        file->read((char *)&number_of_entries_in_relocation_table, sizeof(int));

        for (int i = 0; i < number_of_entries_in_relocation_table; i++)
        {

            unsigned short offset;
            file->read((char *)&offset, sizeof(unsigned short));

            int length_of_string_type;
            file->read((char *)&length_of_string_type, sizeof(int));

            char *a;

            a = new char[length_of_string_type];
            for (int i = 0; i < length_of_string_type; i++)
                file->read((char *)&a[i], sizeof(char));
            string type = convertToString(a, length_of_string_type);

            int length_of_string_symbol;
            file->read((char *)&length_of_string_symbol, sizeof(int));

            a = new char[length_of_string_symbol];
            for (int i = 0; i < length_of_string_symbol; i++)
                file->read((char *)&a[i], sizeof(char));
            string symbol = convertToString(a, length_of_string_symbol);

            unsigned short addend;
            file->read((char *)&addend, sizeof(unsigned short));

            int length_of_string_relocation_table;
            file->read((char *)&length_of_string_relocation_table, sizeof(int));

            a = new char[length_of_string_relocation_table];
            for (int i = 0; i < length_of_string_relocation_table; i++)
                file->read((char *)&a[i], sizeof(char));
            string section_of_relocation_table = convertToString(a, length_of_string_relocation_table);

            entry.offset = offset;
            entry.type = type;
            entry.symbol = symbol;
            entry.addend = addend;
            entry.section_of_relocation_table = section_of_relocation_table;

            rt.add_new_entry(entry);
        }
        rtv.push_back(rt);
    }

    return rtv;
}

/*
    int number_od_entries_in_relocation_table

    {
        unsigned short offset; // offset where we need to write
        int length_of_string_type;
        string type; // ETF_PC_16 or ETF_16
        int length_of_string_symbol;
        string symbol; // symbol name
        char addend;
        int length_of_string_relocation_table;
        string section_of_relocation_table; // name of section for relocation table
    }

*/