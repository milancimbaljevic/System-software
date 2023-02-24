#include "Linker.h"

void print_hex_data_to_file(string filename, vector<unsigned char> bytes){
    ofstream out(filename);

    for(int i=0; i<bytes.size(); i++){
        if(i%8 == 0){
            out << endl;
            out << HEX_LINE(i) << ": ";
        }
        out << HEX(bytes[i]) << " ";
    }
    
    out.close();
}

void print_defined(vector<SymbolTableEntryF> &defined)
{
    cout << endl;

    for (SymbolTableEntryF &entf : defined)
    {
        cout << "File handle: " << entf.file_handle << endl;
        cout << "Value: " << HEX_LINE(entf.entry.value)
             << " Size: " << HEX_LINE(entf.entry.size)
             << " Type: " << entf.entry.type
             << " Bind: " << entf.entry.bind
             << " Section: " << entf.entry.section
             << " Name: " << entf.entry.name << endl;
    }

    cout << endl;
}

void print_section_entry_info(vector<SectionEntry> &entries)
{
    cout << endl;

    for (SectionEntry &ent : entries)
    {
        cout << "File handle: " << ent.file_handle << endl;
        cout << "Start address: " << ent.start_address << endl;
        cout << "Value: " << HEX_LINE(ent.entry.value)
             << " Size: " << HEX_LINE(ent.entry.size)
             << " Type: " << ent.entry.type
             << " Bind: " << ent.entry.bind
             << " Section: " << ent.entry.section
             << " Name: " << ent.entry.name << endl;
        cout << endl;
    }

    cout << endl;
}

SectionEntry get_section_from_file(int fh, string section_name, vector<SectionEntry> &entries)
{
    for (SectionEntry &ent : entries)
    {
        if (ent.file_handle == fh && ent.entry.name == section_name)
            return ent;
    }
    SectionEntry e;
    return e;
}

SymbolTableEntryF *get_defined_symbol(vector<SymbolTableEntryF> &defined, string name)
{
    for (SymbolTableEntryF &entf : defined)
    {
        if (entf.entry.name == name)
            return &entf;
    }
    return nullptr;
}

SectionContent &get_section_content(string name, vector<SectionContent> &content)
{
    for (SectionContent &cont : content)
    {
        if (cont.name == name)
            return cont;
    }
    return content[0]; // compiler keeps throwing warnigs so i added this
}

void Linker::link_files(string &output_name, vector<string> &input_names)
{
    int number_of_input_files = input_names.size();

    BinaryFile **input_files = new BinaryFile *[number_of_input_files];
    for (int i = 0; i < number_of_input_files; i++)
        input_files[i] = new BinaryFile(0, input_names[i]);

    for (int i = 0; i < number_of_input_files; i++)
        input_files[i]->open_file();

    vector<SectionContent> sections_contet[number_of_input_files];
    SymbolTable symbol_tables[number_of_input_files];
    vector<RelocationTable> relocation_tables[number_of_input_files];

    vector<SectionEntry> section_entry_info;

    for (int i = 0; i < number_of_input_files; i++)
    {
        sections_contet[i] = input_files[i]->read_sections_content_from_file();
        symbol_tables[i] = input_files[i]->read_symbol_table_from_file();
        relocation_tables[i] = input_files[i]->read_reloc_tables_from_file();
    }

    vector<SymbolTableEntryF> undefined;
    vector<SymbolTableEntryF> defined;

    /*
        Foreach binary file symbol table check if extern symbols are defined if they are not defined add
        them to undefined vector
    */

    for (int i = 0; i < number_of_input_files; i++)
    {
        SymbolTable &current = symbol_tables[i];
        for (SymbolTableEntry &entry : current.get_all_entries())
        {
            bool extern_symbol_defined = false;
            if (entry.bind == "EXT")
            {
                for (SymbolTableEntryF &entryf : defined)
                {
                    if (entry.name == entryf.entry.name)
                    {
                        extern_symbol_defined = true;
                        break;
                    }
                }

                bool already_in_undefined = false;

                if (!extern_symbol_defined)
                {
                    for (SymbolTableEntryF &entf : undefined)
                    {
                        if (entf.entry.name == entry.name)
                        {
                            already_in_undefined = true;
                            break;
                        }
                    }
                }
                if (!extern_symbol_defined && !already_in_undefined)
                {
                    SymbolTableEntryF ent;
                    ent.entry = entry;
                    ent.file_handle = -1;
                    undefined.push_back(ent);
                }
            }
            else if (entry.bind == "GLOB")
            {
                // if symbol is already defined exit program
                // if it is not defined add it to defined vector
                bool global_symbol_already_defined = false;
                for (SymbolTableEntryF &entryf : defined)
                {
                    if (entryf.entry.name == entry.name)
                    {
                        global_symbol_already_defined = true;
                        break;
                    }
                }
                if (global_symbol_already_defined)
                {
                    cerr << "Error: Multiple definition of global symbol " << entry.name << endl;
                    exit(0);
                }

                // if symbol is not defined add it do defined vector

                SymbolTableEntryF ent;
                ent.entry = entry;
                ent.file_handle = i;
                defined.push_back(ent);

                // if symbol is in undefined list remove it

                int j = 0;
                for (SymbolTableEntryF &entryf : undefined)
                {
                    if (entryf.entry.name == entry.name)
                    {
                        undefined.erase(undefined.begin() + j);
                        break;
                    }
                    j++;
                }
            }
            else if (entry.type == "SCTN")
            {
                SectionEntry ent;
                ent.entry = entry;
                ent.start_address = 0;
                ent.address_defined = false;
                ent.file_handle = i;
                section_entry_info.push_back(ent);
            }
        }
    }

    if (undefined.size() >= 0)
    {
        for (SymbolTableEntryF &ent : undefined)
        {
            cerr << "Error: Symbol " << ent.entry.name << " is never defined." << endl;
        }
    }

    // generate start address for each section in each file

    unsigned short location_counter = 0;

    for (SectionEntry &ent : section_entry_info)
    {
        if (!ent.address_defined)
        {
            ent.start_address = location_counter;
            ent.address_defined = true;
            location_counter += ent.entry.size;

            for (SectionEntry &ent1 : section_entry_info)
            {
                if (!ent1.address_defined && ent.entry.name == ent1.entry.name)
                {
                    ent1.start_address = location_counter;
                    ent1.address_defined = true;
                    location_counter += ent1.entry.size;
                }
            }
        }
    }

    sort(section_entry_info.begin(), section_entry_info.end(), [](const SectionEntry &ent1, const SectionEntry &ent2)
         { return ent1.start_address < ent2.start_address; });

    // generate address of each global symbol

    for (SymbolTableEntryF &entf : defined)
    {
        entf.entry.value += get_section_from_file(entf.file_handle, entf.entry.section, section_entry_info).start_address;
    }

    // generate address of each local symbol

    for (int i = 0; i < number_of_input_files; i++)
    {
        SymbolTable &smt = symbol_tables[i];
        for (SymbolTableEntry &entry : smt.get_all_entries())
        {
            if (entry.bind == "LOC" && entry.type != "SCTN" && entry.type != "EQU")
            {
                entry.value += get_section_from_file(i, entry.section, section_entry_info).start_address;
            }
            if(entry.size == 0xFFFF){
                
            }
        }
    }

    // do relocation

    for (int i = 0; i < number_of_input_files; i++)
    {
        vector<RelocationTable> &rtv = relocation_tables[i];
        for (RelocationTable &rt : rtv)
        {
            for (RelocationTableEntry &entry : rt.all_entries)
            {
                // section start_address

                unsigned char start_addres = get_section_from_file(i, entry.section_of_relocation_table, section_entry_info).start_address;
                unsigned char h;
                unsigned char l;
                unsigned short symbol_value;

                // check if relocation symbol is global
                if (get_defined_symbol(defined, entry.symbol) != nullptr)
                {
                    // global

                    SymbolTableEntryF *entf = get_defined_symbol(defined, entry.symbol);
                    symbol_value = entf->entry.value;
                    if (entry.type == "ETF_16")
                    {
                        h = (symbol_value >> 8);
                        l = ((symbol_value << 8) >> 8);
                    }
                    else
                    {
                        // pc relative
                        unsigned short addres_of_pc = start_addres + entry.offset + 2;
                        signed short relative_address = symbol_value - addres_of_pc;

                        h = (relative_address >> 8);
                        l = ((relative_address << 8) >> 8);
                    }
                }
                else
                {

                    symbol_value = get_section_from_file(i, entry.symbol, section_entry_info).start_address;
                    
                    if(entry.type == "EQU"){
                        symbol_value = entry.addend;
                        entry.type = "ETF_16";
                    }else{
                        symbol_value += entry.addend;
                    }

                    // local
                    if (entry.type == "ETF_16")
                    {
                        h = (symbol_value >> 8);
                        l = ((symbol_value << 8) >> 8);
                    }
                    else
                    {
                        // pc relative
                        unsigned short addres_of_pc = start_addres + entry.offset + 2;
                        signed short relative_address = symbol_value - addres_of_pc;

                        h = (relative_address >> 8);
                        l = ((relative_address << 8) >> 8);
                    }
                }

                // write new address to section content line
                unsigned char current_size = 0;

                SectionContent *cont;

                for (SectionContent &e : sections_contet[i])
                {
                    e.name.erase(std::remove(e.name.begin(), e.name.end(), '\0'), e.name.end());
                    if (e.name == entry.section_of_relocation_table)
                    {
                        cont = &e;
                    }
                }

                for (vector<unsigned char> &vec : cont->content)
                {
                    if (vec.size() == 0)
                        continue;

                    if (entry.offset >= (current_size + vec.size()))
                    {
                        current_size += vec.size();
                        continue;
                    }
                    else
                    {
                        unsigned short in_line = entry.offset - current_size;
                        vec.at(in_line) = l;
                        vec.at(in_line + 1) = h;
                        break;
                    }
                    current_size += vec.size();
                }
            }
        }
    }

    // copy to output file

    ofstream binary_file(output_name);
    vector<unsigned char> bytes;

    unsigned short line_counter = 0;
    for (SectionEntry &entry : section_entry_info)
    {
        SectionContent &cont = get_section_content(entry.entry.name, sections_contet[entry.file_handle]);
        for (vector<unsigned char> line : cont.content)
        {
            for (unsigned char c : line)
            {
                binary_file.write((char *)&c, sizeof(char));
                bytes.push_back(c);
            }
            line_counter += line.size();
        }
    }

    binary_file.close();
    print_hex_data_to_file(output_name+".txt", bytes);

    for (int i = 0; i < number_of_input_files; i++)
        input_files[i]->close_file();
}