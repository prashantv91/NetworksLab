/*
 * mask.h - Class to determine visibility
 */

#ifndef MASK_H_

#define MASK_H_

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/select.h>
#include "constants.h"

class Mask
{
        bool vis[MAP_MAXX][MAP_MAXY];
        public:
        Mask(); 
        bool getvis(int, int); 
        void update(int, int);
        void exchange(Mask *); 
};

#endif

