/* 
 * File:   log_file.h
 * Author: mateusz
 *
 * Created on 17 styczeń 2013, 20:00
 */

#ifndef LOG_FILE_H
#define	LOG_FILE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

void add_log(char *filename, char *log);

#ifdef	__cplusplus
}
#endif

#endif	/* LOG_FILE_H */

