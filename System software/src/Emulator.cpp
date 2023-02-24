#include "Emulator.h"

struct termios orig_termios;

void reset_terminal_mode()
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit()
{
    struct timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv) > 0;
}

int getch()
{
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0)
    {
        return r;
    }
    else
    {
        return c;
    }
}

void Emulator::raiseInterrupt(char ivtEntry)
{
    if (ivtEntry == IVT_ERROR_CODE)
    {
        error_code_line = true;
    }
    else if (ivtEntry == IVT_TERMINAL)
    {
        terminal_line == true;
    }
    else if (ivtEntry == IVT_TIMER)
    {
        timer_line = true;
    }
    else
    {
        global_line = true;
        global_entry = ivtEntry;
    }
}

void Emulator::handleInterrupts(string ins_name)
{
    unsigned short &pc = reg[7];
    unsigned short &sp = reg[6];
    unsigned short &psw = reg[8];

    if (error_code_line)
    {
        sp -= 2;
        writeShortToMemory(sp, pc);
        sp -= 2;
        writeShortToMemory(sp, psw);

        psw = 0b1110000000000000;
        pc = readShortFromMemory(IVT_ERROR_CODE * 2);
        error_code_line = false;
    }
    else if (timer_line)
    {
        if (!(psw & Tr))
        {
            sp -= 2;
            writeShortToMemory(sp, pc);
            sp -= 2;
            writeShortToMemory(sp, psw);

            psw = 0b1110000000000000;

            pc = readShortFromMemory(IVT_TIMER * 2);
            timer_line = false;
        }
    }
    else if (terminal_line)
    {
        if (!(psw & Tl))
        {
            sp -= 2;
            writeShortToMemory(sp, pc);
            sp -= 2;
            writeShortToMemory(sp, psw);

            psw = 0b1110000000000000;
            pc = readShortFromMemory(IVT_TERMINAL * 2);
            terminal_line = false;
        }
    }
    else if (global_line)
    {
        if (!(psw & I))
        {
            sp -= 2;
            writeShortToMemory(sp, pc);
            sp -= 2;
            writeShortToMemory(sp, psw);

            psw = 0b1110000000000000;
            pc = readShortFromMemory(global_entry * 2);
            global_line = false;
        }
    }
}

void Emulator::initMemory()
{
    ifstream *hex_file = new ifstream(input_name);

    hex_file->seekg(0, hex_file->end);
    int length = hex_file->tellg();
    hex_file->seekg(0, hex_file->beg);

    if (length > MEMORY_SIZE - 300)
    {
        cerr << "Errro: Not enough memory to load the program" << endl;
        exit(0);
    }

    hex_file->read((char *)&memory, sizeof(char) * length);
    hex_file->close();

    free(hex_file);

    memory[TIME_CFG] = 0x00;
    memory[TIME_CFG + 1] = 0x00;
}

unsigned char &Emulator::readFromMemory(unsigned short address)
{
    sem_wait(&bus);
    unsigned char &rd = memory[address];
    sem_post(&bus);
    return rd;
}

void Emulator::writeToMemory(unsigned short address, unsigned char value)
{
    sem_wait(&bus);
    memory[address] = value;
    sem_post(&bus);
}

void Emulator::writeShortToMemory(unsigned short address, unsigned short value)
{
    unsigned char low;
    unsigned char high;

    high = value >> 8;
    low = (value << 8) >> 8;

    sem_wait(&bus);
    memory[address] = low;
    memory[address + 1] = high;
    sem_post(&bus);

    if (address == TERM_OUT)
    {
        write_occured = true;
    }
}

unsigned short Emulator::readShortFromMemory(unsigned short address)
{
    unsigned char low;
    unsigned char high;

    sem_wait(&bus);
    low = memory[address];
    high = memory[address + 1];
    sem_post(&bus);

    unsigned short l = low;
    unsigned short h = high;
    h = h << 8;

    unsigned short val = h | l;
    return val;
}

void Emulator::initRegisters()
{
    unsigned char low;
    unsigned char high;
    low = memory[0];
    high = memory[1];

    unsigned short &pc = reg[7];
    unsigned short &sp = reg[6];
    unsigned short &psw = reg[8];

    unsigned short l = low;
    unsigned short h = high;
    h = h << 8;
    pc = h | l;
    sp = 0xFF00;

    for (int i = 0; i < 6; i++)
    {
        reg[i] = 0;
    }

    psw = 0b0000000000000000;
}

void Emulator::start()
{

    unsigned char instr_descr;
    unsigned char regs_descr;
    unsigned char addr_mode;
    unsigned char data_high;
    unsigned char data_low;

    string ins_name;
    int ins_size;
    bool error = false;

    running = true;
    while (running)
    {

        // fetch and decode
        fetchAndDecode(instr_descr, regs_descr, addr_mode, data_high, data_low, ins_name, ins_size, error);

        // execute
        if (!error)
            executeInstruction(instr_descr, regs_descr, addr_mode, data_high, data_low, ins_name, ins_size, error, running);

        // handle interrupts
        if (running)
            handleInterrupts(ins_name);
    }
    printCpuContext();
}

void Emulator::jmpToAddress(unsigned char s_reg, unsigned char addr_mode, unsigned short payload)
{
    unsigned char m = addr_mode & 0b00001111;
    unsigned short &pc = reg[7];
    unsigned short &sp = reg[6];

    if (m == IMMEDIATE)
    {
        pc = payload;
    }
    else if (m == REG_DIR_DISP)
    {
        pc = pc + payload;
    }
    else if (m == REG_DIR)
    {
        pc = reg[s_reg];
    }
    else if (m == MEM)
    {
        pc = readShortFromMemory(payload);
    }
    else if (m == REG_IND)
    {
        pc = readShortFromMemory(reg[s_reg]);
    }
    else if (m == REG_IND_DISP)
    {
        pc = readShortFromMemory(reg[s_reg] + payload);
    }
}

void Emulator::printCpuContext()
{

    unsigned short &pc = reg[7];
    unsigned short &sp = reg[6];
    unsigned short &psw = reg[8];

    std::bitset<16> y(psw);

    cout << "\rEmulated processor executed halt instruction" << endl;
    cout << "\rEmulated processor state: "
         << "psw=" << "0b" << y << endl;
    cout << "\rr0=0x" << HEX_LINE(reg[0]) << "    "
         << "r1=0x" << HEX_LINE(reg[1]) << "   "
         << "r2=0x" << HEX_LINE(reg[2]) << "   "
         << "r3=0x" << HEX_LINE(reg[3]) << endl;
    cout << "\rr4=0x" << HEX_LINE(reg[4]) << "    "
         << "r5=0x" << HEX_LINE(reg[5]) << "   "
         << "r6=0x" << HEX_LINE(sp) << "   "
         << "r7=0x" << HEX_LINE(pc) << endl;
}

void Emulator::fetchAndDecode(unsigned char &instr_descr, unsigned char &regs_descr, unsigned char &addr_mode, unsigned char &data_high, unsigned char &data_low,
                              string &ins_name, int &ins_size, bool &error)

{

    unsigned short &pc = reg[7];
    unsigned short &sp = reg[6];
    unsigned short &psw = reg[8];

    instr_descr = readFromMemory(pc++);

    if (instr_descr == 0x00)
    {
        ins_name = "halt";
        ins_size = 1;
    }
    else if (instr_descr == 0x20)
    {
        ins_name = "iret";
        ins_size = 1;
    }
    else if (instr_descr == 0x40)
    {
        ins_name = "ret";
        ins_size = 1;
    }
    else
    {
        regs_descr = readFromMemory(pc++);
        if (instr_descr == 0x10)
        {
            ins_name = "int";
            ins_size = 2;
        }
        else if (instr_descr == 0x60)
        {
            ins_name = "xchg";
            ins_size = 2;
        }
        else if (instr_descr >> 4 == 0x7)
        {
            ins_size = 2;
            unsigned char m = instr_descr & 0b00001111;

            if (m == 0)
            {
                ins_name = "add";
            }
            else if (m == 1)
            {
                ins_name = "sub";
            }
            else if (m == 2)
            {
                ins_name = "mul";
            }
            else if (m == 3)
            {
                ins_name = "div";
            }
            else if (m == 4)
            {
                ins_name = "cmp";
            }
            else
            {
                error = true;
                raiseInterrupt(IVT_ERROR_CODE);
            }
        }
        else if (instr_descr >> 4 == 0x8)
        {
            ins_size = 2;
            unsigned char m = instr_descr & 0b00001111;

            if (m == 0)
            {
                ins_name = "not";
            }
            else if (m == 1)
            {
                ins_name = "and";
            }
            else if (m == 2)
            {
                ins_name = "or";
            }
            else if (m == 3)
            {
                ins_name = "xor";
            }
            else if (m == 4)
            {
                ins_name = "test";
            }
            else
            {
                error = true;
                raiseInterrupt(IVT_ERROR_CODE);
            }
        }
        else if (instr_descr >> 4 == 0x9)
        {
            ins_size = 2;
            unsigned char m = instr_descr & 0b00001111;

            if (m == 0)
            {
                ins_name = "shl";
            }
            else if (m == 1)
            {
                ins_name = "shr";
            }
            else
            {
                error = true;
                raiseInterrupt(IVT_ERROR_CODE);
            }
        }
        else
        {
            addr_mode = readFromMemory(pc++);

            unsigned char m = addr_mode & 0b00001111;

            if (m != IMMEDIATE && m != REG_DIR && m != REG_DIR_DISP && m != REG_IND && m != REG_IND_DISP && m != MEM)
            {
                error = true;
                raiseInterrupt(IVT_ERROR_CODE);
            }

            addr_mode = m;

            if (!error)
            {
                if (m == IMMEDIATE || m == REG_DIR_DISP || m == REG_IND_DISP || m == MEM)
                {
                    ins_size = 5;
                    data_low = readFromMemory(pc++);
                    data_high = readFromMemory(pc++);
                }
                else
                {
                    ins_size = 3;
                }
            }

            if (!error)
            {
                if (instr_descr == 0x30)
                {
                    ins_name = "call";
                }
                else if (instr_descr >> 4 == 0x5)
                {
                    unsigned char m = instr_descr & 0b00001111;

                    if (m == 0)
                    {
                        ins_name = "jmp";
                    }
                    else if (m == 1)
                    {
                        ins_name = "jeq";
                    }
                    else if (m == 2)
                    {
                        ins_name = "jne";
                    }
                    else if (m == 3)
                    {
                        ins_name = "jgt";
                    }
                    else
                    {
                        error = true;
                        raiseInterrupt(IVT_ERROR_CODE);
                    }
                }
                else if (instr_descr == 0xA0)
                {
                    unsigned char m = regs_descr & 0b00001111;
                    if (m == 0x6 && ins_size == 3)
                    {
                        ins_name = "pop";
                    }
                    else
                    {
                        ins_name = "ldr";
                    }
                }
                else if (instr_descr == 0xB0)
                {
                    unsigned char m = regs_descr & 0b00001111;

                    if (m == 0x6  && ins_size == 3)
                    {
                        ins_name = "push";
                    }
                    else
                    {
                        ins_name = "str";
                    }
                }
                else
                {
                    error = true;
                    raiseInterrupt(IVT_ERROR_CODE);
                }
            }
        }
    }
}

void Emulator::printRAM(unsigned short start_address, unsigned short end_address)
{
    cout << endl;
    cout << "RAM: " << endl;
    for (unsigned short i = start_address; i <= end_address; i += 5)
    {
        cout << HEX_LINE(i) << ": ";
        for (int j = i; j < i + 5; j++)
        {
            cout << HEX(readFromMemory(j)) << " ";
        }
        cout << endl;
    }
}

void Emulator::printStack()
{
    cout << endl;
    cout << "Stack: " << endl;
    for (unsigned short i = 0xFF00 - 1; i >= reg[6]; i -= 5)
    {
        cout << HEX_LINE(i) << ": ";
        for (int j = i; j > i - 5; j--)
        {
            cout << HEX(readFromMemory(j)) << " ";
        }
        cout << endl;
    }
}

void Emulator::executeInstruction(unsigned char &instr_descr, unsigned char &regs_descr, unsigned char &addr_mode, unsigned char &data_high, unsigned char &data_low,
                                  string &ins_name, int &ins_size, bool &error, bool &running)
{

    unsigned short &pc = reg[7];
    unsigned short &sp = reg[6];
    unsigned short &psw = reg[8];

    unsigned char d_reg = (regs_descr & 0b11110000) >> 4;
    unsigned char s_reg = regs_descr & 0b00001111;

    unsigned char N = psw & N_FLAG;
    unsigned char Z = psw & Z_FLAG;
    unsigned char O = psw & O_FLAG;
    unsigned char C = psw & C_FLAG;

    unsigned short payload;

    if (ins_size == 5)
    {
        unsigned short h = data_high;
        h <<= 8;
        unsigned short l = data_low;
        payload = h | l;
    }

    if (ins_name == "halt")
    {
        running = false;
    }
    else if (ins_name == "int")
    {
        // push psw; pc<=mem16[(reg[DDDD]%8)*2]
        sp -= 2;
        writeShortToMemory(sp, pc);
        sp -= 2;
        writeShortToMemory(sp, psw);
        pc = readShortFromMemory((reg[d_reg] % 8) * 2);
	psw = 0b1110000000000000;
    }
    else if (ins_name == "iret")
    {
        // pop psw; pop pc;
        psw = readShortFromMemory(sp);
        sp += 2;
        pc = readShortFromMemory(sp);
        sp += 2;
    }
    else if (ins_name == "call")
    {
        // push pc; pc <= opperand;
        sp -= 2;
        writeShortToMemory(sp, pc);
        jmpToAddress(s_reg, addr_mode, payload);
    }
    else if (ins_name == "ret")
    {
        // pop pc;
        pc = readShortFromMemory(sp);
        sp += 2;
    }
    else if (ins_name == "jmp")
    {
        jmpToAddress(s_reg, addr_mode, payload);
    }
    else if (ins_name == "jeq")
    {
        if (Z)
        {
            jmpToAddress(s_reg, addr_mode, payload);
        }
    }
    else if (ins_name == "jne")
    {
        if (!Z)
        {
            jmpToAddress(s_reg, addr_mode, payload);
        }
    }
    else if (ins_name == "jgt")
    {
        if (!(N ^ O) & !Z)
        {
            jmpToAddress(s_reg, addr_mode, payload);
        }
    }
    else if (ins_name == "push")
    {
        sp -= 2;
        writeShortToMemory(sp, reg[d_reg]);
    }
    else if (ins_name == "pop")
    {
        reg[d_reg] = readShortFromMemory(sp);
        sp += 2;
    }
    else if (ins_name == "xchg")
    {
        unsigned short temp = reg[d_reg];
        reg[d_reg] = reg[s_reg];
        reg[s_reg] = temp;
    }
    else if (ins_name == "add")
    {
        reg[d_reg] = (unsigned short)((signed short)reg[d_reg] + (signed short)reg[s_reg]);
    }
    else if (ins_name == "sub")
    {
        reg[d_reg] = (unsigned short)((signed short)reg[d_reg] - (signed short)reg[s_reg]);
    }
    else if (ins_name == "mul")
    {
        reg[d_reg] = (unsigned short)((signed short)reg[d_reg] * (signed short)reg[s_reg]);
    }
    else if (ins_name == "div")
    {
        reg[d_reg] = (unsigned short)((signed short)reg[d_reg] / (signed short)reg[s_reg]);
    }
    else if (ins_name == "cmp")
    {
        short temp = ((signed short)reg[d_reg] - (signed short)reg[s_reg]);
        // update psw;
        if (temp == 0)
        {
            psw |= Z_FLAG;
        }
        else
        {
            psw &= ~Z_FLAG;
        }

        if (temp < 0)
        {
            psw |= N_FLAG;
        }
        else
        {
            psw &= ~N_FLAG;
        }

        short temp1 = (signed short)reg[d_reg];
        short temp2 = (signed short)reg[s_reg];
        if ((temp1 > 0 && temp2 < 0 && (temp1 - temp2) < 0) || (temp1 < 0 && temp2 > 0 && (temp1 - temp2) > 0))
        {
            psw |= O_FLAG;
        }
        else
        {
            psw &= ~O_FLAG;
        }

        if (temp1 < temp2)
        {
            psw |= C_FLAG;
        }
        else
        {
            psw &= ~C_FLAG;
        }
    }
    else if (ins_name == "not")
    {
        reg[d_reg] = ~reg[d_reg];
    }
    else if (ins_name == "and")
    {
        reg[d_reg] = reg[d_reg] & reg[s_reg];
    }
    else if (ins_name == "or")
    {
        reg[d_reg] = reg[d_reg] | reg[s_reg];
    }
    else if (ins_name == "xor")
    {
        reg[d_reg] = reg[d_reg] ^ reg[s_reg];
    }
    else if (ins_name == "test")
    {
        short temp = ((signed short)reg[d_reg]) & ((signed short)reg[s_reg]);
        // update psw;

        if (temp == 0)
        {
            psw |= Z_FLAG;
        }
        else
        {
            psw &= ~Z_FLAG;
        }
        if (temp < 0)
        {
            psw |= N_FLAG;
        }
        else
        {
            psw &= ~N_FLAG;
        }
    }
    else if (ins_name == "shl")
    {
        short temp1 = (signed short)reg[d_reg];
        short temp2 = (signed short)reg[s_reg];
        reg[d_reg] = (unsigned short)((signed short)reg[d_reg] << (signed short)reg[s_reg]);
        short temp = (signed short)reg[d_reg];
        // update psw;
        if (temp == 0)
        {
            psw |= Z_FLAG;
        }
        else
        {
            psw &= ~Z_FLAG;
        }

        if (temp < 0)
        {
            psw |= N_FLAG;
        }
        else
        {
            psw &= ~N_FLAG;
        }
        if (temp2 < 16 && ((temp1 >> (16 - temp2)) & 1))
        {
            psw |= C_FLAG;
        }
        else
        {
            psw &= ~C_FLAG;
        }
    }
    else if (ins_name == "shr")
    {
        short temp1 = (signed short)reg[d_reg];
        short temp2 = (signed short)reg[s_reg];
        reg[d_reg] = (unsigned short)((signed short)reg[d_reg] << (signed short)reg[s_reg]);
        short temp = (signed short)reg[d_reg];
        // update psw;
        if (temp == 0)
        {
            psw |= Z_FLAG;
        }
        else
        {
            psw &= ~Z_FLAG;
        }

        if (temp < 0)
        {
            psw |= N_FLAG;
        }
        else
        {
            psw &= ~N_FLAG;
        }
        if ((temp1 >> (temp2 - 1)) & 1)
        {
            psw |= C_FLAG;
        }
        else
        {
            psw &= ~C_FLAG;
        }
    }
    else if (ins_name == "ldr")
    {
        unsigned char m = addr_mode & 0b00001111;

        if (m == IMMEDIATE)
        {
            reg[d_reg] = payload;
        }
        else if (m == MEM)
        {
            reg[d_reg] = readShortFromMemory(payload);
        }
        else if (m == REG_IND_DISP)
        {
            reg[d_reg] = readShortFromMemory(reg[s_reg] + payload);
        }
        else if (m == REG_DIR)
        {
            reg[d_reg] = reg[s_reg];
        }
        else if (m == REG_IND)
        {
            reg[d_reg] = readShortFromMemory(reg[s_reg]);
        }
    }
    else if (ins_name == "str")
    {
        unsigned char m = addr_mode & 0b00001111;

        if (m == MEM)
        {
            writeShortToMemory(payload, reg[d_reg]);
        }
        else if (m == REG_IND_DISP)
        {
            writeShortToMemory(reg[s_reg] + payload, reg[d_reg]);
        }
        else if (m == REG_DIR)
        {
            reg[s_reg] = reg[d_reg];
        }
        else if (m == REG_IND)
        {
            writeShortToMemory(reg[s_reg], reg[d_reg]);
        }
    }
}

void *timer(void *emulator)
{
    unsigned short tim_cfg;

    Emulator *em = (Emulator *)emulator;

    while (em->running)
    {
        tim_cfg = em->readShortFromMemory(TIME_CFG);
        int time;

        if (tim_cfg == 0x0)
        {
            time = 500;
        }
        else if (tim_cfg == 0x1)
        {
            time = 1000;
        }
        else if (tim_cfg == 0x2)
        {
            time = 1500;
        }
        else if (tim_cfg == 0x3)
        {
            time = 2000;
        }
        else if (tim_cfg == 0x4)
        {
            time = 5000;
        }
        else if (tim_cfg == 0x5)
        {
            time = 10000;
        }
        else if (tim_cfg == 0x6)
        {
            time = 30000;
        }
        else if (tim_cfg == 0x7)
        {
            time = 60000;
        }

        usleep(time * 1000);
        em->raiseInterrupt(IVT_TIMER);
    }

    return NULL;
}

void *keyboard(void *emulator)
{
    Emulator *em = (Emulator *)emulator;

    int x = 0;

    set_conio_terminal_mode();

    while (em->running)
    {
        if (kbhit())
        {
            em->writeShortToMemory(TERM_IN, (unsigned char)getch());
            em->terminal_line = true;
        }
    }

    return NULL;
}

void *display(void *emulator)
{
    Emulator *em = (Emulator *)emulator;

    while (em->running)
    {
        if (em->write_occured)
        {
            unsigned char term_out_char = em->readShortFromMemory(TERM_OUT);
            cout << '\r' << term_out_char << endl;
            em->write_occured = false;
        }
    }

    return NULL;
}
