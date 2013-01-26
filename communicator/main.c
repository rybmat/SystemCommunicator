/* 
 * File:   main.c
 * Author: mateusz
 *
 * Created on 17 styczeń 2013, 11:23
 */

#include <stdio.h>
#include <stdlib.h>
#include "ui.h"
#include "protocol.h"
#include <fcntl.h>
#include "client_operations.h"
/*
 * 
 */
int main(int argc, char** argv) {
    
    
    if(init() == 0){
        printf("Something went wrong, sorry...");
        return 0;
    }
    
    int p = fork();
    if(p == 0){
        while(1){
            process_heartbeat();
        }
    }else{
        if(p == -1){
            logout();
            return 1;
        }
        kill(p, SIGSTOP);
        set_child_pid(p);
        ui_main();  
    }
    
    return (EXIT_SUCCESS);
}

