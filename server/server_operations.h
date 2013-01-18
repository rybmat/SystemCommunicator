/* 
 * File:   server_operations.h
 * Author: mateusz
 *
 * Created on 17 styczeń 2013, 20:28
 */

#ifndef SERVER_OPERATIONS_H
#define	SERVER_OPERATIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"    
    int init();
    void register_client();


#ifdef	__cplusplus
}
#endif

#endif	/* SERVER_OPERATIONS_H */

