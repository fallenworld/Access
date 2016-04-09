#include "access_commands.h"
#include <stdio.h>

#ifdef TARGET_RASPBERRY

#include <wiringPi.h>
int is_setup = 0;
#define ACCESS_PIN 15

#endif

int writeFile(int state)
{
    FILE* fp = fopen("access_state", "wb+");
    if (fp == NULL)
    {
        puts("Cannot open file access_state");
    }
    int ret = fwrite(&state, sizeof(int), 1, fp);
    if(ret <= 0)
    {
        puts("Cannot write file access_state");
    }
    fclose(fd);
    return ret;
}

int readFile()
{
    int state
    FILE* fp = fopen("access_state", "rb");
    if (fp == NULL)
    {
        puts("Cannot open file access_state");
    }
    int ret = fread(&state, sizeof(int), 1, fp);
    if(ret <= 0)
    {
        puts("Cannot read file access_state");
    }
    fclose(fd);
    return ret;
}

int openDoor()
{
    puts("Executing openDoor");
#ifdef TARGET_RASPBERRY
    if (!is_setup)
    {
        wiringPiSetup();
        pinMode(ACCESS_PIN, OUTPUT);
        is_setup = 1;
    }
    digitalWrite(ACCESS_PIN, LOW);
    writeFile(1);
    if (getDoorState() == 1)
    {
        return 0;
    }
    else
    {
        puts("Cannot open the door");
        return -2;
    }
#else
    return 0;
#endif
}

int closeDoor()
{
    puts("Executing closeDoor");
#ifdef TARGET_RASPBERRY
    if (!is_setup)
    {
        wiringPiSetup();
        pinMode(ACCESS_PIN, OUTPUT);
        is_setup = 1;
    }
    digitalWrite(ACCESS_PIN, HIGH);
    writeFile(0);
    if (getDoorState() == 0)
    {
        return 0;
    }
    else
    {
        puts("Cannot close the door");
        return -2;
    }
#else
    return 0;
#endif
}

int getDoorState()
{
    puts("Executing getDoorState");
#ifdef TARGET_RASPBERRY
    int state = readFile();
    if (state != 0 && state != 1)
    {
        return -2
    }
    else
    {
        return state;
    }
#else
    return 0;
#endif
}
