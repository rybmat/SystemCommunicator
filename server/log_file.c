#include "log_file.h"

void add_log(char* filename, char* log){
    FILE *F;
    F = fopen("log.log", 'a');
    fprintf(F, "%s", log);
    fclose(F);
}
