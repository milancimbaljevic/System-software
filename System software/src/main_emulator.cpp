#include <iostream>
#include <string>
#include "Emulator.h"

using namespace std;

extern void *timer(void *emulator);
extern void *display(void *emulator);
extern void *keyboard(void *emulator);

void *emulator_f(void *emulator)
{
    Emulator *em = (Emulator *)emulator;
    em->start();
    return NULL;
}

int main(int argc, char **argv)
{

    if(argc != 2){
        cerr << "Input arguments error!!!" << endl;
        exit(0);
    }

    string input_name = argv[1];
    void *status;

    Emulator emulator(input_name);
    emulator.initMemory();
    emulator.initRegisters();
    emulator.running = true;

    sem_init(&emulator.bus, 0, 1);

    pthread_t emulatort;
    pthread_create(&emulatort, NULL, &emulator_f, (void *)&emulator);

    pthread_t timert;
    pthread_create(&timert, NULL, timer, (void *)&emulator);

    pthread_t displayt;
    pthread_create(&displayt, NULL, display, (void *)&emulator);

    pthread_t keyboardt;
    pthread_create(&keyboardt, NULL, keyboard, (void *)&emulator);

    pthread_join(emulatort, &status);
    pthread_join(timert, &status);
    pthread_join(displayt, &status);
    pthread_join(keyboardt, &status);

    sem_destroy(&emulator.bus);
}