/* 
 * File:   client_operations.h
 * Author: mateusz
 *
 * Created on 17 styczeń 2013, 20:31
 */

#ifndef CLIENT_OPERATIONS_H
#define	CLIENT_OPERATIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"
    
    
char* parser(char* command);

int init();

char* logout();

char* msg_snd(char* reciver, char* msg);

char* enter_channel(char* channel_name);

#ifdef	__cplusplus
}
#endif

#endif	/* CLIENT_OPERATIONS_H */

