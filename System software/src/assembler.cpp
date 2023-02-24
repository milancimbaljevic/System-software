#include "assembler.h"
#include <iomanip>

#define HEX(x) \
    setw(2) << setfill('0') << hex << (int)(x)

#define HEX_LINE(x) \
    setw(4) << setfill('0') << hex << (int)(x)

struct HexCharStruct
{
    unsigned char c;
    HexCharStruct(unsigned char _c) : c(_c) {}
};

inline std::ostream &operator<<(std::ostream &o, const HexCharStruct &hs)
{
    return (o << std::hex << (int)hs.c);
}

inline HexCharStruct hex(unsigned char _c)
{
    return HexCharStruct(_c);
}

void Assembler::beautify_line(string &str)
{

    regex delete_comments("#.*$");
    regex delete_starting_spaces("^ +");
    regex delete_ending_spaces(" +$");
    regex delete_spaces_after_commas(", +");
    regex delete_spaces_before_commas("  +,");
    regex delete_spaces_from_inside(" {2,}");
    regex delete_spaces_before_columns(" +:");
    regex delete_spaces_after_columns(": +");
    regex delete_spaces_after_brackets("\\[ +");
    regex delete_spaces_before_brackets(" +\\]");
    regex one_space_before_plus(" *\\+");
    regex one_space_after_plus("\\+ *");

    str = regex_replace(str, delete_comments, "");
    str = regex_replace(str, delete_starting_spaces, "");
    str = regex_replace(str, delete_spaces_after_commas, ",");
    str = regex_replace(str, delete_spaces_before_commas, ",");
    str = regex_replace(str, delete_ending_spaces, "");
    str = regex_replace(str, delete_spaces_from_inside, " ");
    str = regex_replace(str, delete_spaces_before_columns, ":");
    str = regex_replace(str, delete_spaces_after_columns, ":");
    str = regex_replace(str, delete_spaces_after_brackets, "[");
    str = regex_replace(str, delete_spaces_before_brackets, "]");
    str = regex_replace(str, one_space_after_plus, "+ ");
    str = regex_replace(str, one_space_before_plus, " +");
}

string Assembler::check_if_directive(string &str)
{

    regex global("^\\.global ([a-zA-Z][a-zA-Z0-9_]*(,[a-zA-Z][a-zA-Z0-9_]*)*)$");
    regex externd("^\\.extern ([a-zA-Z][a-zA-Z0-9_]*(,[a-zA-Z][a-zA-Z0-9_]*)*)$");
    regex section("^\\.section [a-zA-Z][a-zA-Z0-9_]*$");
    regex word("^\\.word (([a-zA-Z][a-zA-Z0-9_]*|-?[0-9]+|0x[0-9a-fA-F]+)(,([a-zA-Z][a-zA-Z0-9_]*|-?[0-9]+|0x[0-9A-F]))*)$");
    regex skip("^\\.skip ([0-9]+|0x[0-9a-fA-F]+)$");
    regex ascii("^\\.ascii \"[ a-zA-Z0-9_!@#\\$%\\^&\\*\\(\\)\\+-\\~,\\.\\[\\]<>\\?\\{\\}]*\"$");
    regex end("^\\.end$");
    regex equ("^\\.equ [a-zA-Z][a-zA-Z0-9_]*,(-?[0-9]+|0x[0-9a-fA-F]+)$");

    if (regex_search(str, global))
        return "GLOBAL";
    if (regex_search(str, externd))
        return "EXTERN";
    if (regex_search(str, section))
        return "SECTION";
    if (regex_search(str, word))
        return "WORD";
    if (regex_search(str, skip))
        return "SKIP";
    if (regex_search(str, end))
        return "END";
    if (regex_search(str, ascii))
        return "ASCII";
    if (regex_search(str, equ))
        return "EQU";

    return "NULL";
}

bool Assembler::check_if_only_label(string &str)
{
    regex find_one_line_label("^[a-zA-Z][a-zA-Z0-9_]*:$");
    return regex_search(str, find_one_line_label);
}

bool Assembler::check_if_label_plus_ins(string &str)
{
    regex find_label_and_something_else("^[a-zA-Z][a-zA-Z0-9_]*:.*$");
    return regex_search(str, find_label_and_something_else);
}

string Assembler::label_part(string &str)
{
    regex label("^[a-zA-Z][a-zA-z0-9_]*:");
    return get_first_regex_match(str, label);
}

string Assembler::ins_or_dir_part(string str)
{
    regex label("^[a-zA-Z][a-zA-z0-9_]*:");
    str = regex_replace(str, label, "");
    return str;
}

string Assembler::get_first_regex_match(string str, regex reg)
{

    sregex_iterator currentMatch(str.begin(), str.end(), reg);
    sregex_iterator lastMatch;

    if (currentMatch != lastMatch)
    {
        return (*currentMatch).str();
    }

    return "NULL";
}

vector<string> Assembler::get_all_regex_matches(string str, regex reg)
{

    vector<string> l;

    sregex_iterator currentMatch(str.begin(), str.end(), reg);
    sregex_iterator lastMatch;

    while (currentMatch != lastMatch)
    {
        l.push_back((*currentMatch).str());
        currentMatch++;
    }

    return l;
}

string Assembler::check_if_no_arg_ins(string &str)
{
    regex no_arg_ins("^(halt|iret|ret)$");
    if (regex_search(str, no_arg_ins))
    {
        string s = get_first_regex_match(str, no_arg_ins);
        return s.substr(0, s.find(" "));
    }
    return "NULL";
}

string Assembler::check_if_one_operand_ins(string &str)
{
    regex one_operand_ins("^(call|jmp|jeq|jne|jgt) (.+)$");
    if (regex_search(str, one_operand_ins))
    {
        string s = get_first_regex_match(str, one_operand_ins);
        return s.substr(0, s.find(" "));
    }
    return "NULL";
}

string Assembler::check_if_one_reg_ins(string &str)
{
    regex one_reg_ins("^(int|push|pop|not) (r[0-7]|psw)$");
    if (regex_search(str, one_reg_ins))
    {
        string s = get_first_regex_match(str, one_reg_ins);
        return s.substr(0, s.find(" "));
    }
    return "NULL";
}

string Assembler::check_if_two_reg_ins(string str)
{
    regex two_reg_ins("^(xchg|add|sub|mul|div|cmp|and|or|xor|test|shl|shr) (r[0-7]|psw),(r[0-7]|psw)$");
    if (regex_search(str, two_reg_ins))
    {
        string s = get_first_regex_match(str, two_reg_ins);
        return s.substr(0, s.find(" "));
    }
    return "NULL";
}

string Assembler::check_if_first_reg_second_operand(string &str)
{
    regex reg_operand("^(ldr|str) (r[0-7]|psw),(-?.+)$");
    if (regex_search(str, reg_operand))
    {
        string s = get_first_regex_match(str, reg_operand);
        return s.substr(0, s.find(" "));
    }
    return "NULL";
}

void Assembler::print_lines_in_hex(vector<vector<unsigned char>> &lines_in_hex)
{
    unsigned short line_counter = 0;

    for (vector<unsigned char> line : lines_in_hex)
    {

        cout << HEX_LINE(line_counter) << ": ";

        for (unsigned char c : line)
        {
            cout << HEX(c) << " ";
        }

        cout << endl;

        line_counter += line.size();
    }
}

void Assembler::print_lines_in_hex_to_file(ofstream &output_file, vector<vector<unsigned char>> &lines_in_hex)
{

    unsigned short line_counter = 0;

    for (vector<unsigned char> line : lines_in_hex)
    {
        output_file << HEX_LINE(line_counter) << ": ";
        for (unsigned char c : line)
        {
            output_file << HEX(c) << " ";
        }

        output_file << endl;
        line_counter += line.size();
    }
}

void Assembler::process_line(string line, SymbolTable &smt, string &current_section, unsigned char &location_counter, bool &assembling, vector<vector<unsigned char>> &lines_in_hex, vector<SectionContent> &sections_content, SectionContent &section_content, RelocationTable &rt, vector<RelocationTable> &rtv)
{

    vector<unsigned char> line_in_hex;

    bool extern_or_global = false;

    if (line == "")
        return;

    if (check_if_only_label(line))
    {
        regex find_one_line_label("^[a-zA-Z][a-zA-Z0-9_]*:$");
        string label_name = get_first_regex_match(line, find_one_line_label);
        label_name = label_name.substr(0, label_name.size() - 1);

        SymbolTableEntry *smte = smt.get_entry(label_name);

        if (smte != nullptr)
        {
            if (smte->bind == "UND")
            {
                smte->bind = "LOC";
                smte->section = current_section;
                smte->value = location_counter;
            }
            else if (smte->bind == "LOC" || (smte->bind == "GLOB" && smte->section != "UND"))
            {
                cerr << "Double defintion error!!!" << endl;
                exit(0);
            }
            else if (smte->bind == "GLOB" && smte->section == "UND")
            {
                smte->section = current_section;
                smte->value = location_counter;
            }
        }
        else
        {
            SymbolTableEntry entry;
            entry.size = 0;
            entry.type = "NOTYP";
            entry.bind = "LOC";
            entry.section = current_section;
            entry.name = label_name;
            entry.value = location_counter;

            smt.add_new_entry(entry);
        }
    }
    else if (check_if_label_plus_ins(line))
    {
        process_line(label_part(line), smt, current_section, location_counter, assembling, lines_in_hex, sections_content, section_content, rt, rtv);
        process_line(ins_or_dir_part(line), smt, current_section, location_counter, assembling, lines_in_hex, sections_content, section_content, rt, rtv);
        return;
    }
    else if (check_if_one_operand_ins(line) != "NULL")
    {
        string jump_ins = check_if_one_operand_ins(line);

        regex get_operand_raw(" .+");
        string operand_raw = get_first_regex_match(line, get_operand_raw);
        operand_raw = operand_raw.substr(1, operand_raw.size() - 1);

        // find addressing type

        string type;
        unsigned short literal = 0x0000;
        string symbol = "NULL";
        int reg = 0;

        unsigned char instr_descr = 0x00;
        unsigned char regs_descr = 0xff;
        unsigned char addr_mode = 0x00;
        unsigned char data_high = 0x00;
        unsigned char data_low = 0x00;
        int length_in_bytes = 0;

        if (regex_search(operand_raw, regex("^([0-9]+|0x[0-9a-fA-F]+)$")))
        {
            type = "<literal>"; // immediate pc <= literal, literal is the payload
            string temp = get_first_regex_match(operand_raw, regex("^([0-9]+|0x[0-9a-fA-F]+)$"));

            if (regex_search(temp, regex("0x")))
            {
                istringstream iss(temp);
                iss >> hex >> literal;
            }
            else
            {
                literal = (short)stoi(temp);
            }

            addr_mode = 0x00;
            data_high = (unsigned char)(literal >> 8);
            data_low = (unsigned char)((literal << 8) >> 8);

            length_in_bytes = 5;
        }
        else if (regex_search(operand_raw, regex("^[a-zA-Z][a-zA-Z0-9_]*$")))
        {
            type = "<symbol>"; // immediate paylod is value of symbol absoulue addressing ( pc<=symbol )
            symbol = get_first_regex_match(operand_raw, regex("^[a-zA-Z][a-zA-Z0-9_]*$"));

            addr_mode = 0x00;
            data_high = 0x00;
            data_low = 0x00;

            length_in_bytes = 5;

            // i need to insert symbol value into data hight and data low

            // check if symbol is in table

            SymbolTableEntry *smte = smt.get_entry(symbol);

            if (smte == nullptr)
            {
                SymbolTableEntry entry;
                entry.value = 0;
                entry.size = 0;
                entry.type = "NOTYP";
                entry.bind = "UND";
                entry.section = "UND";
                entry.name = symbol;

                smt.add_new_entry(entry);
            }

            // add new entry to relocation table

            RelocationTableEntry entry;
            entry.offset = location_counter + length_in_bytes - 2;
            entry.type = "ETF_16";
            entry.addend = 0;
            entry.section_of_relocation_table = current_section;
            entry.symbol = symbol;

            rt.add_new_entry(entry);
        }
        else if (regex_search(operand_raw, regex("^%[a-zA-Z][a-zA-Z0-9_]*$")))
        {
            type = "%<symbol>"; // register direct with displacement paylod is value of symbol pc relative addressing (pc<=pc + symbol)
            symbol = get_first_regex_match(operand_raw, regex("^%[a-zA-Z][a-zA-Z0-9_]*$"));
            symbol = symbol.substr(1, symbol.size() - 1);

            addr_mode = 0x05;
            regs_descr = 0xf7;
            data_high = 0x00;
            data_low = 0x00;

            length_in_bytes = 5;

            // i need to insert symbol value into data high and data low

            // check if symbol is in table

            SymbolTableEntry *smte = smt.get_entry(symbol);

            if (smte == nullptr)
            {
                SymbolTableEntry entry;
                entry.value = 0;
                entry.size = 0;
                entry.type = "NOTYP";
                entry.bind = "UND";
                entry.section = "UND";
                entry.name = symbol;

                smt.add_new_entry(entry);
            }

            RelocationTableEntry entry;
            entry.offset = location_counter + length_in_bytes - 2;
            entry.type = "ETF_PC_16";
            entry.addend = 0;
            entry.section_of_relocation_table = current_section;
            entry.symbol = symbol;

            rt.add_new_entry(entry);
        }
        else if (regex_search(operand_raw, regex("^\\*r[0-7]$")))
        {
            type = "*<reg>"; // register direct pc<=reg
            string temp = get_first_regex_match(operand_raw, regex("^\\*r[0-7]$"));
            temp = temp.substr(2, 1);
            reg = stoi(temp);

            regs_descr = 0xf0 | reg;
            addr_mode = 0x01;

            length_in_bytes = 3;
        }
        else if (regex_search(operand_raw, regex("^\\*([0-9]+|0x[0-9a-fA-F]+)$")))
        {
            type = "*<literal>"; // memory payload is address from wich you read address pc <= mem[literal]
            string temp = get_first_regex_match(operand_raw, regex("^\\*([0-9]+|0x[0-9a-fA-F]+)$"));
            temp = temp.substr(1, temp.size() - 1);

            if (regex_search(temp, regex("0x")))
            {
                istringstream iss(temp);
                iss >> hex >> literal;
            }
            else
            {
                literal = (short)stoi(temp);
            }

            addr_mode = 0x04;
            regs_descr = 0xff;

            data_high = (unsigned char)(literal >> 8);
            data_low = (unsigned char)(literal & 0x00ff);

            length_in_bytes = 5;
        }
        else if (regex_search(operand_raw, regex("^\\*[a-zA-Z][a-zA-Z0-9_]*$")))
        {
            type = "*<simbol>"; // memory payload is value of symbol ( pc <= mem[symbol] )
            symbol = get_first_regex_match(operand_raw, regex("^\\*[a-zA-Z][a-zA-Z0-9_]*$"));
            symbol = symbol.substr(1, symbol.size() - 1);

            addr_mode = 0x04;
            regs_descr = 0xff;
            data_high = 0x00;
            data_low = 0x00;

            length_in_bytes = 5;

            // check if symbol is in table

            SymbolTableEntry *smte = smt.get_entry(symbol);

            if (smte == nullptr)
            {
                SymbolTableEntry entry;
                entry.value = 0;
                entry.size = 0;
                entry.type = "NOTYP";
                entry.bind = "UND";
                entry.section = "UND";
                entry.name = symbol;

                smt.add_new_entry(entry);
            }

            RelocationTableEntry entry;
            entry.offset = location_counter + length_in_bytes - 2;
            entry.type = "ETF_16";
            entry.addend = 0;
            entry.section_of_relocation_table = current_section;
            entry.symbol = symbol;

            rt.add_new_entry(entry);
        }
        else if (regex_search(operand_raw, regex("^\\*\\[r[0-7]\\]$")))
        {
            type = "*[<reg>]"; // register indirect (pc <= mem[reg[i]])
            string temp = get_first_regex_match(operand_raw, regex("^\\*\\[r[0-7]\\]$"));
            temp = temp.substr(3, 1);
            reg = stoi(temp);

            addr_mode = 0x02;
            regs_descr = 0xf0 | reg;

            length_in_bytes = 3;
        }
        else if (regex_search(operand_raw, regex("^\\*\\[r[0-7] \\+ -?([0-9]+|0x[0-9a-fA-F]+)\\]$")))
        {
            type = "*[<reg> + <literal>]"; // register indirect with displacment payload is literal (pc <= mem[reg[i] + literal])
            string temp = get_first_regex_match(operand_raw, regex("^\\*\\[r[0-7]"));
            temp = temp.substr(3, 1);
            reg = stoi(temp);
            temp = get_first_regex_match(operand_raw, regex("\\+ -?([0-9]+|0x[0-9a-fA-F]+)\\]$"));
            temp = temp.substr(2, temp.size() - 3);

            if (regex_search(temp, regex("0x")))
            {
                istringstream iss(temp);
                iss >> hex >> literal;
            }
            else
            {
                literal = (short)stoi(temp);
            }

            addr_mode = 0x03;
            regs_descr = 0xf0 | reg;

            data_high = (unsigned char)(literal >> 8);
            data_low = (unsigned char)((literal << 8) >> 8);
            length_in_bytes = 5;
        }
        else if (regex_search(operand_raw, regex("^\\*\\[r[0-7] \\+ [a-zA-Z][a-zA-Z0-9]*\\]$")))
        {
            type = "*[<reg> + <symbol>]"; // register indirect with displacment payload is value of symbol (pc <= mem[reg[i] + symbol])
            string temp = get_first_regex_match(operand_raw, regex("^\\*\\[r[0-7]"));
            temp = temp.substr(3, 1);
            reg = stoi(temp);
            temp = get_first_regex_match(operand_raw, regex("\\+ [a-zA-Z][a-zA-Z0-9]*\\]$"));
            symbol = temp.substr(2, temp.size() - 3);

            addr_mode = 0x03;
            regs_descr = 0xf0 | reg;
            data_high = 0x00;
            data_low = 0x00;

            length_in_bytes = 5;

            // check if symbol is in table

            SymbolTableEntry *smte = smt.get_entry(symbol);

            if (smte == nullptr)
            {
                SymbolTableEntry entry;
                entry.value = 0;
                entry.size = 0;
                entry.type = "NOTYP";
                entry.bind = "UND";
                entry.section = "UND";
                entry.name = symbol;

                smt.add_new_entry(entry);
            }

            RelocationTableEntry entry;
            entry.offset = location_counter + length_in_bytes - 2;
            entry.type = "ETF_16";
            entry.addend = 0;
            entry.section_of_relocation_table = current_section;
            entry.symbol = symbol;

            rt.add_new_entry(entry);
        }
        else
        {
            cerr << "SYNTAX ERROR!!!" << endl;
            exit(0);
        }

        // check instruction name

        if (jump_ins == "call")
        {
            instr_descr = 0x30;
        }
        else if (jump_ins == "jmp")
        {
            instr_descr = 0x50;
        }
        else if (jump_ins == "jeq")
        {
            instr_descr = 0x51;
        }
        else if (jump_ins == "jne")
        {
            instr_descr = 0x52;
        }
        else if (jump_ins == "jgt")
        {
            instr_descr = 0x53;
        }
        else
        {
            cerr << "SYNTAX ERROR!!!" << endl;
            exit(0);
        }

        line_in_hex.push_back(instr_descr);
        line_in_hex.push_back(regs_descr);
        line_in_hex.push_back(addr_mode);

        if (length_in_bytes == 5)
        {
            line_in_hex.push_back(data_low);
            line_in_hex.push_back(data_high);
        }
    }
    else if (check_if_no_arg_ins(line) != "NULL")
    {
        string ins_name = check_if_no_arg_ins(line);

        char instr_descr = -1;

        if (ins_name == "halt")
        {
            instr_descr = 0x00;
        }
        else if (ins_name == "iret")
        {
            instr_descr = 0x20;
        }
        else if (ins_name == "ret")
        {
            instr_descr = 0x40;
        }
        else
        {
            cerr << "SYNTAX ERROR!!!" << endl;
            exit(0);
        }

        line_in_hex.push_back(instr_descr);
    }
    else if (check_if_one_reg_ins(line) != "NULL")
    {
        unsigned int reg;
        string ins_name = check_if_one_reg_ins(line);
        string reg_raw = get_first_regex_match(line, regex(" (r[0-7]|psw)$"));
        if (reg_raw[1] != 'p')
            reg = stoi(reg_raw.substr(2, 1));
        else
            reg = 8;

        unsigned char instr_descr = 0x00;
        unsigned char regs_descr = 0x00;
        unsigned char addr_mode = 0x00;
        int length_in_bytes = 0;

        if (ins_name == "int")
        {
            instr_descr = 0x10;
            regs_descr = (reg << 4) | 0xf;

            length_in_bytes = 2;
        }
        else if (ins_name == "push")
        {
            instr_descr = 0xb0;
            regs_descr = (reg << 4) | 0x6;
            addr_mode = 0x12;

            length_in_bytes = 3;
        }
        else if (ins_name == "pop")
        {
            instr_descr = 0xa0;
            regs_descr = (reg << 4) | 0x6;
            addr_mode = 0x42;

            length_in_bytes = 3;
        }
        else if (ins_name == "not")
        {
            instr_descr = 0x80;
            regs_descr = (reg << 4) | 0xf;

            length_in_bytes = 2;
        }
        else
        {
            cerr << "SYNTAX ERROR!!!" << endl;
            exit(0);
        }

        line_in_hex.push_back(instr_descr);
        line_in_hex.push_back(regs_descr);

        if (length_in_bytes == 3)
        {
            line_in_hex.push_back(addr_mode);
        }
    }
    else if (check_if_two_reg_ins(line) != "NULL")
    {
        string ins_name = check_if_two_reg_ins(line);

        string regs_raw = get_first_regex_match(line, regex(" .+"));
        string first_reg_raw = regs_raw.substr(1, regs_raw.find(',') - 1);
        string second_reg_raw = regs_raw.substr(regs_raw.find(",") + 1, regs_raw.size() - regs_raw.find(',') - 1);

        int reg1 = -1;
        int reg2 = -1;

        if (first_reg_raw[0] != 'p')
            reg1 = stoi(first_reg_raw.substr(1, 1));
        else
            reg1 = 8;

        if (second_reg_raw[0] != 'p')
            reg2 = stoi(second_reg_raw.substr(1, 1));
        else
            reg2 = 8;

        unsigned char instr_descr = 0x00;
        unsigned char regs_descr = 0x00;
        unsigned char addr_mode = 0x00;
        int length_in_bytes = 0;

        if (ins_name == "xchg")
        {
            instr_descr = 0x60;
            regs_descr = (reg1 << 4) | reg2;

            length_in_bytes = 2;
        }
        else if (ins_name == "add")
        {
            instr_descr = 0x70;
            regs_descr = (reg1 << 4) | reg2;

            length_in_bytes = 2;
        }
        else if (ins_name == "sub")
        {
            instr_descr = 0x71;
            regs_descr = (reg1 << 4) | reg2;

            length_in_bytes = 2;
        }
        else if (ins_name == "mul")
        {
            instr_descr = 0x72;
            regs_descr = (reg1 << 4) | reg2;

            length_in_bytes = 2;
        }
        else if (ins_name == "div")
        {
            instr_descr = 0x73;
            regs_descr = (reg1 << 4) | reg2;

            length_in_bytes = 2;
        }
        else if (ins_name == "cmp")
        {
            instr_descr = 0x74;
            regs_descr = (reg1 << 4) | reg2;

            length_in_bytes = 2;
        }
        else if (ins_name == "and")
        {
            instr_descr = 0x81;
            regs_descr = (reg1 << 4) | reg2;

            length_in_bytes = 2;
        }
        else if (ins_name == "or")
        {
            instr_descr = 0x82;
            regs_descr = (reg1 << 4) | reg2;

            length_in_bytes = 2;
        }
        else if (ins_name == "xor")
        {
            instr_descr = 0x83;
            regs_descr = (reg1 << 4) | reg2;

            length_in_bytes = 2;
        }
        else if (ins_name == "test")
        {
            instr_descr = 0x84;
            regs_descr = (reg1 << 4) | reg2;

            length_in_bytes = 2;
        }
        else if (ins_name == "shl")
        {
            instr_descr = 0x90;
            regs_descr = (reg1 << 4) | reg2;

            length_in_bytes = 2;
        }
        else if (ins_name == "shr")
        {
            instr_descr = 0x91;
            regs_descr = (reg1 << 4) | reg2;

            length_in_bytes = 2;
        }
        else
        {
            cerr << "SYNTAX ERROR!!!" << endl;
            exit(0);
        }

        line_in_hex.push_back(instr_descr);
        line_in_hex.push_back(regs_descr);

    }
    else if (check_if_first_reg_second_operand(line) != "NULL")
    {
        string ins_name = check_if_first_reg_second_operand(line);
        unsigned char reg = 0xff;
        string type;
        string args_raw = get_first_regex_match(line, regex(" .+"));
        string reg_raw = args_raw.substr(1, args_raw.find(',') - 1);
        string operand_raw = args_raw.substr(args_raw.find(',') + 1, args_raw.size() - args_raw.find(',') - 1);
        unsigned short literal;
        string symbol;
        unsigned char reg_op = 0x0f;

        unsigned char instr_descr = 0x00;
        unsigned char regs_descr = 0xff;
        unsigned char addr_mode = 0x00;
        unsigned char data_high = 0x00;
        unsigned char data_low = 0x00;
        int length_in_bytes = 0;

        if (reg_raw[0] != 'p')
            reg = stoi(reg_raw.substr(1, 1));
        else
            reg = 8;

        if (regex_search(operand_raw, regex("^\\$(-?[0-9]+|0x[0-9a-fA-F]+)$")))
        {
            type = "$<literal>"; // immediate payload is literal reg<=literal
            string temp = get_first_regex_match(operand_raw, regex("^\\$(-?[0-9]+|0x[0-9a-fA-F]+)$"));
            temp = temp.substr(1, temp.size() - 1);

            if (regex_search(temp, regex("0x")))
            {
                istringstream iss(temp);
                iss >> hex >> literal;
            }
            else
            {
                literal = stoi(temp);
            }

            addr_mode = 0x00;
            data_high = (unsigned short)(literal >> 8);
            data_low = (unsigned short)((literal << 8) >> 8);
            regs_descr = (reg << 4) | reg_op;

            length_in_bytes = 5;
        }
        else if (regex_search(operand_raw, regex("^\\$[a-zA-Z][a-zA-Z0-9_]*$")))
        {
            type = "$<symbol>"; // immediate payload is value of symbol reg<=symbol
            symbol = get_first_regex_match(operand_raw, regex("^\\$[a-zA-Z][a-zA-Z0-9_]*$"));
            symbol = symbol.substr(1, symbol.size() - 1);

            addr_mode = 0x00;
            regs_descr = (reg << 4) | reg_op;

            length_in_bytes = 5;

            // check if symbol is in table

            SymbolTableEntry *smte = smt.get_entry(symbol);

            if (smte == nullptr)
            {
                SymbolTableEntry entry;
                entry.value = 0;
                entry.size = 0;
                entry.type = "NOTYP";
                entry.bind = "UND";
                entry.section = "UND";
                entry.name = symbol;

                smt.add_new_entry(entry);
            }

            RelocationTableEntry entry;
            entry.offset = location_counter + length_in_bytes - 2;
            entry.type = "ETF_16";
            entry.addend = 0;
            entry.section_of_relocation_table = current_section;
            entry.symbol = symbol;

            rt.add_new_entry(entry);
        }
        else if (regex_search(operand_raw, regex("^(-?[0-9]+|0x[0-9a-fA-F]+)$")))
        {
            type = "<literal>"; // memory reg<=mem[literal] payload is literal
            string temp = get_first_regex_match(operand_raw, regex("^(-?[0-9]+|0x[0-9a-fA-F]+)$"));

            if (regex_search(temp, regex("0x")))
            {
                istringstream iss(temp);
                iss >> hex >> literal;
            }
            else
            {
                literal = stoi(temp);
            }

            addr_mode = 0x04;
            regs_descr = (reg << 4) | reg_op;
            data_high = (unsigned char)(literal >> 8);
            data_low = (unsigned char)((literal << 8) >> 8);

            length_in_bytes = 5;
        }

        else if (regex_search(operand_raw, regex("^%[a-zA-Z][a-zA-Z0-9_]*$")))
        {
            type = "%<symbol>"; // regind with displacement reg<=mem[pc + symbol] payload is value of symbol pc relative
            symbol = get_first_regex_match(operand_raw, regex("^%[a-zA-Z][a-zA-Z0-9_]*$"));
            symbol = symbol.substr(1, symbol.size() - 1);

            addr_mode = 0x03;
            regs_descr = (reg << 4) | 0x07;

            length_in_bytes = 5;

            // check if symbol is in table

            SymbolTableEntry *smte = smt.get_entry(symbol);

            if (smte == nullptr)
            {
                SymbolTableEntry entry;
                entry.value = 0;
                entry.size = 0;
                entry.type = "NOTYP";
                entry.bind = "UND";
                entry.section = "UND";
                entry.name = symbol;

                smt.add_new_entry(entry);
            }

            RelocationTableEntry entry;
            entry.offset = location_counter + length_in_bytes - 2;
            entry.type = "ETF_PC_16";
            entry.addend = 0;
            entry.section_of_relocation_table = current_section;
            entry.symbol = symbol;

            rt.add_new_entry(entry);
        }
        else if (regex_search(operand_raw, regex("^(r[0-7]|psw)$")))
        {
            type = "<reg>"; // regdir reg<=reg1
            string reg_raw = get_first_regex_match(operand_raw, regex("^(r[0-7]|psw)$"));

            if (reg_raw[0] != 'p')
                reg_op = stoi(reg_raw.substr(1, 1));
            else
                reg_op = 8;

            addr_mode = 0x01;
            regs_descr = (reg << 4) | reg_op;

            length_in_bytes = 3;
        }
        else if (regex_search(operand_raw, regex("^[a-zA-Z][a-zA-Z0-9_]*$")))
        {
            type = "<symbol>"; // memory reg<=mem[symbol] payload is value of symbol absolute address
            symbol = get_first_regex_match(operand_raw, regex("^[a-zA-Z][a-zA-Z0-9_]*$"));

            addr_mode = 0x04;
            regs_descr = (reg << 4) | reg_op;

            length_in_bytes = 5;

            // check if symbol is in table

            SymbolTableEntry *smte = smt.get_entry(symbol);

            if (smte == nullptr)
            {
                SymbolTableEntry entry;
                entry.value = 0;
                entry.size = 0;
                entry.type = "NOTYP";
                entry.bind = "UND";
                entry.section = "UND";
                entry.name = symbol;

                smt.add_new_entry(entry);
            }

            RelocationTableEntry entry;
            entry.offset = location_counter + length_in_bytes - 2;
            entry.type = "ETF_16";
            entry.addend = 0;
            entry.section_of_relocation_table = current_section;
            entry.symbol = symbol;

            rt.add_new_entry(entry);
        }
        else if (regex_search(operand_raw, regex("^\\[r[0-7]\\]$")))
        {
            type = "[<reg>]"; // reg<=mem[reg1] regind
            string temp = get_first_regex_match(operand_raw, regex("^\\[r[0-7]\\]$"));
            temp = temp.substr(2, 1);
            reg_op = stoi(temp);

            addr_mode = 0x02;
            regs_descr = (reg << 4) | reg_op;

            length_in_bytes = 3;
        }
        else if (regex_search(operand_raw, regex("^\\[r[0-7] \\+ (-?[0-9]+|0x[0-9a-fA-F]+)\\]$")))
        {
            type = "[<reg> + <literal>]"; // regind with displacment literal is payload reg<=mem[reg1 + displacement]
            string temp = get_first_regex_match(operand_raw, regex("^\\[r[0-7]"));
            temp = temp.substr(2, 1);
            reg_op = stoi(temp);
            temp = get_first_regex_match(operand_raw, regex("\\+ (-?[0-9]+|0x[0-9a-fA-F]+)\\]$"));
            temp = temp.substr(2, temp.size() - 3);

            if (regex_search(temp, regex("0x")))
            {
                istringstream iss(temp);
                iss >> hex >> literal;
            }
            else
            {
                literal = stoi(temp);
            }

            addr_mode = 0x03;
            data_high = (unsigned char)(literal >> 8);
            data_low = (unsigned char)((literal << 8) >> 8);
            regs_descr = (reg << 4) | reg_op;

            length_in_bytes = 5;
        }
        else if (regex_search(operand_raw, regex("^\\[r[0-7] \\+ [a-zA-Z][a-zA-Z0-9]*\\]$")))
        {
            type = "[<reg> + <symbol>]"; // regind with displacement reg<=mem[reg1 + symbol]
            string temp = get_first_regex_match(operand_raw, regex("^\\[r[0-7]"));
            temp = temp.substr(2, 1);
            reg_op = stoi(temp);
            temp = get_first_regex_match(operand_raw, regex("\\+ [a-zA-Z][a-zA-Z0-9]*\\]$"));
            symbol = temp.substr(2, temp.size() - 3);

            addr_mode = 0x03;
            regs_descr = (reg << 4) | reg_op;

            length_in_bytes = 5;

            // check if symbol is in table

            SymbolTableEntry *smte = smt.get_entry(symbol);

            if (smte == nullptr)
            {
                SymbolTableEntry entry;
                entry.value = 0;
                entry.size = 0;
                entry.type = "NOTYP";
                entry.bind = "UND";
                entry.section = "UND";
                entry.name = symbol;

                smt.add_new_entry(entry);
            }

            RelocationTableEntry entry;
            entry.offset = location_counter + length_in_bytes - 2;
            entry.type = "ETF_16";
            entry.addend = 0;
            entry.section_of_relocation_table = current_section;
            entry.symbol = symbol;

            rt.add_new_entry(entry);
        }
        else
        {
            cerr << "SYNTAX ERROR!!!" << endl;
            exit(0);
        }

        if (ins_name == "ldr")
        {
            instr_descr = 0xa0;
        }
        else if (ins_name == "str")
        {
            instr_descr = 0xb0;
        }
        else
        {
            cerr << "SYNTAX ERROR!!!" << endl;
            exit(0);
        }

        line_in_hex.push_back(instr_descr);
        line_in_hex.push_back(regs_descr);
        line_in_hex.push_back(addr_mode);

        if (length_in_bytes == 5)
        {
            line_in_hex.push_back(data_low);
            line_in_hex.push_back(data_high);
        }
    }
    else if (check_if_directive(line) != "NULL")
    {
        string directive_name = check_if_directive(line);
        vector<string> symbol_list;
        unsigned short literal = 0;
        string section_name;

        if (directive_name == "GLOBAL")
        {
            extern_or_global = true;

            string first_symbol = get_first_regex_match(line, regex(" [a-zA-Z0-9][a-zA-Z0-9_]*"));
            first_symbol = first_symbol.substr(1, first_symbol.find(',') - 1);

            symbol_list.push_back(first_symbol);

            vector<string> raw_symbol_list = get_all_regex_matches(line, regex(",[a-zA-Z0-9][a-zA-Z0-9_]*"));

            for (string s : raw_symbol_list)
            {
                symbol_list.push_back(s.substr(1, s.size() - 1));
            }

            // do all the other stuff here

            // check if symbol is in table
            for (string symbol : symbol_list)
            {

                SymbolTableEntry *smte = smt.get_entry(symbol);

                if (smte == nullptr)
                {
                    SymbolTableEntry entry;
                    entry.value = 0;
                    entry.size = 0;
                    entry.type = "NOTYP";
                    entry.bind = "GLOB";
                    entry.section = "UND";
                    entry.name = symbol;

                    smt.add_new_entry(entry);
                }
                else
                {
                    SymbolTableEntry *entry = smt.get_entry(symbol);

                    if (entry->bind != "LOC")
                        entry->section = "UND";

                    entry->bind = "GLOB";
                }
            }
        }
        else if (directive_name == "EXTERN")
        {
            extern_or_global = true;

            string first_symbol = get_first_regex_match(line, regex(" [a-zA-Z0-9][a-zA-Z0-9_]*"));
            first_symbol = first_symbol.substr(1, first_symbol.find(',') - 1);

            symbol_list.push_back(first_symbol);

            vector<string> raw_symbol_list = get_all_regex_matches(line, regex(",[a-zA-Z0-9][a-zA-Z0-9_]*"));

            for (string s : raw_symbol_list)
            {
                symbol_list.push_back(s.substr(1, s.size() - 1));
            }

            // do all the other stuff here

            for (string symbol : symbol_list)
            {
                SymbolTableEntry *smte = smt.get_entry(symbol);

                if (smte == nullptr)
                {
                    SymbolTableEntry entry;
                    entry.value = 0;
                    entry.size = 0;
                    entry.type = "NOTYP";
                    entry.bind = "EXT";
                    entry.section = "UND";
                    entry.name = symbol;

                    smt.add_new_entry(entry);
                }
                else
                {
                    SymbolTableEntry *entry = smt.get_entry(symbol);

                    if (entry->bind == "LOC" || entry->bind == "GLOB")
                    {
                        cerr << "Error: Extern symbols cant be defined inside current file." << endl;
                        exit(0);
                    }

                    entry->bind = "EXT";
                    entry->section = "UND";
                }
            }
        }
        else if (directive_name == "SECTION")
        {
            section_name = line.substr(9, line.size() - 9);

            string old_section = current_section;

            SymbolTableEntry *smte = smt.get_entry(section_name);

            if (smte != nullptr)
            {
                cerr << "Double section definition error!!!" << endl;
                exit(0);
            }
            else
            {
                SymbolTableEntry entry;
                entry.value = 0;
                entry.size = 0;
                entry.type = "SCTN";
                entry.bind = "LOC";
                entry.section = section_name;
                entry.name = section_name;

                smt.add_new_entry(entry);

                if (current_section != "UND")
                    smt.get_entry(current_section)->size = location_counter;
                location_counter = 0;
                current_section = section_name;
            }

            if (section_content.name != "UND")
            {
                sections_content.push_back(section_content);
                rt.section_name = old_section;
                rtv.push_back(rt);
                rt.all_entries.clear();
            }

            section_content.content.clear();
            section_content.name = current_section;
        }
        else if (directive_name == "WORD")
        {
            if (regex_search(line, regex(" -?([0-9]+|0x[0-9a-fA-F]+)$")))
            {
                string temp = line.substr(6, line.size() - 6);
                if (regex_search(temp, regex("0x")))
                {
                    istringstream iss(temp);
                    iss >> hex >> literal;
                }
                else
                {
                    literal = stoi(temp);
                }

                unsigned char high = (unsigned short)(literal >> 8);
                unsigned char low = (unsigned short)((literal << 8) >> 8);

                line_in_hex.push_back(low);
                line_in_hex.push_back(high);
            }
            else
            {
                string first_symbol = get_first_regex_match(line, regex(" [a-zA-Z0-9][a-zA-Z0-9_]*"));
                first_symbol = first_symbol.substr(1, first_symbol.find(',') - 1);

                symbol_list.push_back(first_symbol);

                RelocationTableEntry we;

                we.offset = location_counter;
                we.type = "ETF_16";
                we.symbol = first_symbol;
                we.addend = 0;
                we.section_of_relocation_table = current_section;

                rt.all_entries.push_back(we);

                line_in_hex.push_back(0x00);
                line_in_hex.push_back(0x00);
                lines_in_hex.push_back(line_in_hex);
                section_content.content.push_back(line_in_hex);

                location_counter += 2;

                vector<string> raw_symbol_list = get_all_regex_matches(line, regex(",[a-zA-Z0-9][a-zA-Z0-9_]*"));

                for (string s : raw_symbol_list)
                {
                    line_in_hex.clear();
                    symbol_list.push_back(s.substr(1, s.size() - 1));
                    line_in_hex.push_back(0x00);
                    line_in_hex.push_back(0x00);
                    lines_in_hex.push_back(line_in_hex);
                    section_content.content.push_back(line_in_hex);

                    we.offset = location_counter;
                    we.type = "ETF_16";
                    we.symbol = s.substr(1, s.size() - 1);
                    we.addend = 0;
                    we.section_of_relocation_table = current_section;

                    rt.all_entries.push_back(we);

                    location_counter += 2;
                }

                line_in_hex.clear();

                return;
            }

        }
        else if (directive_name == "SKIP")
        {
            if (regex_search(line, regex(" ([0-9]+|0x[0-9a-fA-F]+)$")))
            {
                string temp = line.substr(6, line.size() - 6);
                if (regex_search(temp, regex("0x")))
                {
                    istringstream iss(temp);
                    iss >> hex >> literal;
                }
                else
                {
                    literal = stoi(temp);
                }


                for (int i = 0; i < literal; i++)
                {
                    line_in_hex.push_back(0x00);
                }
            }
            else
            {
                cerr << "SYNTAX ERROR!!!" << endl;
                exit(0);
            }
        }
        else if (directive_name == "ASCII")
        {
            string str = get_first_regex_match(line, regex("\"[ a-zA-Z0-9_!@#\\$%\\^&\\*\\(\\)\\+-\\~,\\.\\[\\]<>\\?\\{\\}]*\"$"));
            str = str.substr(1, str.size() - 2);

            for (unsigned char c : str)
            {
                line_in_hex.push_back(c);
            }
        }
        else if (directive_name == "EQU")
        {
            string symbol = get_first_regex_match(line, regex(" [a-zA-Z][a-zA-X0-9_]*,"));
            symbol = symbol.substr(symbol.find(' ') + 1, symbol.find(',') - symbol.find(' ') - 1);
            string temp = line.substr(line.find(',') + 1, line.size() - line.find(',') - 1);

            if (regex_search(temp, regex("0x")))
            {
                istringstream iss(temp);
                iss >> hex >> literal;
            }
            else
            {
                literal = stoi(temp);
            }

            SymbolTableEntry *entry = smt.get_entry(symbol);

            if(entry != nullptr){
                entry->value = literal;
                entry->bind = "LOC";
                entry->type = "EQU";
                entry->size = 0;
                entry->section = current_section;
                entry->name = symbol;
            }else{
                SymbolTableEntry entry;
                // value size type bind section name
                entry.value = literal;
                entry.size = 0; // flag for equ symbols
                entry.type = "EQU";
                entry.bind = "LOC";
                entry.section = current_section;
                entry.name = symbol;

                smt.add_new_entry(entry);
            }
        }
        else if (directive_name == "END")
        {
            // stop assembling
            assembling = false;
        }
    }
    else
    {
        cerr << "SYNTAX ERROR!!!" << endl;
        exit(0);
    }

    if (line_in_hex.size() != 0)
    {
        lines_in_hex.push_back(line_in_hex);
        section_content.content.push_back(line_in_hex);
        location_counter += line_in_hex.size();
    }

    if (current_section == "UND" && !extern_or_global)
    {
        cerr << "Code and data must be inside of section!!!" << endl;
        exit(0);
    }

    if (!assembling)
    {
        SymbolTableEntry *entry = smt.get_entry(current_section);
        entry->size = location_counter;
        sections_content.push_back(section_content);
        rt.section_name = current_section;
        rtv.push_back(rt);

        // check if some pc relative addressing symobols are in the same section as relocation address

        // check that for each section

        for (RelocationTable &r : rtv)
        {
            int entry_index = 0;

            for (RelocationTableEntry &entry : r.all_entries)
            {
                if (entry.type == "ETF_PC_16")
                {
                    if (entry.section_of_relocation_table == smt.get_entry(entry.symbol)->section)
                    {
                        signed short diff = (signed char)smt.get_entry(entry.symbol)->value - (signed char)entry.offset - 2;
                        unsigned char h = (diff >> 8);
                        unsigned char l = ((diff << 8) >> 8);

                        for (SectionContent &cont : sections_content)
                        {
                            if (cont.name == entry.section_of_relocation_table)
                            {

                                unsigned short current_size = 0;

                                for (vector<unsigned char> &vec : cont.content)
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

                                        r.all_entries.erase(r.all_entries.begin() + entry_index);

                                        break;
                                    }

                                    current_size += vec.size();
                                }
                                break;
                            }
                        }
                    }
                }
                entry_index++;
            }
        }
    }
}
