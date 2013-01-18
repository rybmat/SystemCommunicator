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
    
    /*char cmd[300];
    gets(cmd);
    parser(cmd);
    */
    
    if(init() == 0){
        printf("Something went wrong, sorry...");
        return 0;
    }
    ui_main();
    
    return (EXIT_SUCCESS);
}

