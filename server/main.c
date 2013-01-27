/* 
 * File:   main.c
 * Author: mateusz
 *
 * Created on 17 styczeń 2013, 19:49
 */

#include <stdio.h>
#include <stdlib.h>
#include "server_operations.h"
/*
 * 
 */

int parser(char * command);

int main(int argc, char** argv) {
    
    int chpid;
    
    
        chpid = fork();
        if(chpid == 0){
            if(init()){
                server_main();
            }
            exit(0);
        }else{
            char command[10];
            int i;
            
            do{
                for(i = 0; i < 10; ++i){
                    command[i] = '\0';
                }
                gets(command);
                command[9] = '\0';
            }while(!parser(command));
            kill(chpid, SIGTERM);
            
        }
        return (EXIT_SUCCESS);
}

int parser(char *command){
    if(strcmp(command, "close") == 0){
        return 1;
    }else{
        return 0;
    }
}
