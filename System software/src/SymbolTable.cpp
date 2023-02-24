#include "SymbolTable.h"

void SymbolTable::add_new_entry(SymbolTableEntry entry)
{
    all_entries.push_back(entry);
}

void SymbolTable::add_new_section(string name)
{
    SymbolTableEntry entry;

    entry.value = 0;
    entry.size = 0;
    entry.type = "SCTN";
    entry.bind = "LOC";
    entry.name = name;
    entry.section = name;

    all_entries.push_back(entry);
}

SymbolTableEntry *SymbolTable::get_entry(string name)
{
    for (SymbolTableEntry &entry : all_entries)
    {
        if (entry.name == name)
            return &entry;
    }

    return nullptr;
}

void SymbolTable::print_symbol_table()
{
    for (SymbolTableEntry &entry : all_entries)
    {
        cout << "Value: " << HEX_LINE(entry.value)
             << " Size: " << HEX_LINE(entry.size)
             << " Type: " << entry.type
             << " Bind: " << entry.bind
             << " Section: " << entry.section
             << " Name: " << entry.name << endl;
    }
}

void SymbolTable::print_symbol_table_to_file(ofstream& output_file)
{
    for (SymbolTableEntry &entry : all_entries)
    {
        output_file << "Value: " << HEX_LINE(entry.value)
             << " Size: " << HEX_LINE(entry.size)
             << " Type: " << entry.type
             << " Bind: " << entry.bind
             << " Section: " << entry.section
             << " Name: " << entry.name << endl;
    }
}