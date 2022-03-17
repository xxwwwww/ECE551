

#ifndef HOT_POTATO_POTATO_H
#define HOT_POTATO_POTATO_H

#include <iostream>
#include <cstring>
#include <unistd.h>

class Potato{
    public:
    int hops;
    int round;
    int path[512];
    Potato() : hops(0), round(0) {
        memset(path, 0, sizeof(path));
    }
};

#endif //HOT_POTATO_POTATO_H
