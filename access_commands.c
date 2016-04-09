#include "access_commands.h"
#ifdef TARGET_RASPBERRY
#include <wiringpi.h>
#endif

int openDoor()
{
    puts("Executing openDoor");
#ifdef TARGET_RASPBERRY
    // TODO
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
    // TODO
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
