#pragma once

#include <iostream>
#include <regex>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <semaphore.h>
#include <pthread.h>
#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include <termios.h>

using namespace std;

#define HEX(x) \
    setw(2) << setfill('0') << hex << (int)(x)

#define HEX_LINE(x) \
    setw(4) << setfill('0') << hex << (int)(x)

#define MEMORY_SIZE 64 * 1024

#define I 0b1000000000000000
#define Tl 0b0100000000000000
#define Tr 0b0010000000000000
#define Z_FLAG 0b0000000000000001
#define O_FLAG 0b0000000000000010
#define C_FLAG 0b0000000000000100
#define N_FLAG 0b0000000000001000

#define IMMEDIATE 0
#define REG_DIR 1
#define REG_DIR_DISP 5
#define REG_IND 2
#define REG_IND_DISP 3
#define MEM 4

#define IVT_ERROR_CODE 1
#define IVT_TIMER 2
#define IVT_TERMINAL 3

#define TERM_OUT 0xFF00
#define TERM_IN 0xFF02

#define TIME_CFG 0xFF10

class Emulator
{
public:
    Emulator(string n){
        input_name = n;
    }
    string input_name;
    bool write_occured = false;
    unsigned char memory[MEMORY_SIZE];
    unsigned short reg[9];
    bool error_code_line = false;
    unsigned char global_entry;
    bool terminal_line = false;
    bool timer_line = false;
    bool global_line = false;
    sem_t bus;
    bool running = false;
    void start();
    void initMemory();
    unsigned char &readFromMemory(unsigned short address);
    void writeToMemory(unsigned short address, unsigned char value);
    unsigned short readShortFromMemory(unsigned short address);
    void writeShortToMemory(unsigned short address, unsigned short value);
    void raiseInterrupt(char ivtEntry);
    void handleInterrupts(string ins_name);
    void updatePSW();
    void initRegisters();
    void fetchAndDecode(unsigned char &instr_descr, unsigned char &regs_descr, unsigned char &addr_mode, unsigned char &data_high, unsigned char &data_low,
                        string &ins_name, int &ins_size, bool &error);
    void executeInstruction(unsigned char &instr_descr, unsigned char &regs_descr, unsigned char &addr_mode, unsigned char &data_high, unsigned char &data_low,
                            string &ins_name, int &ins_size, bool &error, bool &running);
    void jmpToAddress(unsigned char reg, unsigned char addr_mode, unsigned short payload);
    void printCpuContext();
    void printRAM(unsigned short start_address, unsigned short end_address);
    void printStack();
};