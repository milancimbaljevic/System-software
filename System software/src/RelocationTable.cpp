#include "RelocationTable.h"

void RelocationTable::add_new_entry(RelocationTableEntry entry)
{
    all_entries.push_back(entry);
}

void RelocationTable::print_relocation_table()
{
    for (RelocationTableEntry &entry : all_entries)
    {
        cout << "Offset: " << HEX_LINE(entry.offset)
             << " Type: " << entry.type << " Symbol: " << entry.symbol
             << " Addend: " << HEX_LINE(entry.addend) << endl;
    }
}

void RelocationTable::print_relocation_table_to_file(ofstream &output_file)
{
    for (RelocationTableEntry &entry : all_entries)
    {
        output_file << "Offset: " << HEX_LINE(entry.offset)
             << " Type: " << entry.type << " Symbol: " << entry.symbol
             << " Addend: " << HEX_LINE(entry.addend) << endl;
    }

}