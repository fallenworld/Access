#include "access_commands.h"
#include <stdio.h>

#ifdef TARGET_RASPBERRY

#include <wiringPi.h>
int is_setup = 0;
#define ACCESS_PIN 15

#endif

char writeFile(char state)
{
    FILE* fp = fopen("access_state", "wt+");
    if (fp == NULL)
    {
        puts("Cannot open file access_state");
        fclose(fp);
        return -1;
    }
    char ret = (char)fputc(state, fp);
    if(ret != state)
    {
        puts("Cannot write file access_state");
    }
    fclose(fp);
    return ret;
}

char readFile()
{
    char state;
    FILE* fp = fopen("access_state", "rt");
    if (fp == NULL)
    {
        puts("Cannot open file access_state");
        fclose(fp);
        return -1;
    }
    state = fgetc(fp);
    if(state != 0 && state != 1)
    {
        puts("Ilegal state number");
    }
    fclose(fp);
    return state;
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
    printf("state:%d\n", state);
    if (state != 0 && state != 1)
    {
        return -2;
    }
    else
    {
        return state;
    }
#else
    return 0;
#endif
}
