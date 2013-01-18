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
int main(int argc, char** argv) {
    
    if(init()){
        register_client();
    }
    
    return (EXIT_SUCCESS);
}

