#include "access_commands.h"

#ifdef TARGET_RASPBERRY

#include <wiringpi.h>
int is_setup = 0;
#define ACCESS_PIN 15

#endif

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
    // TODO
    return -2;
#else
    return 0;
#endif
}
