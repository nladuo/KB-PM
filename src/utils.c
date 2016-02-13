#include <string.h>
#include "utils.h"

/*split the str by spl, save into dst, and return the dst size.*/
int str_split(char dst[][STR_BUFFER_SIZE], char* str, const char* spl)
{
    int n = 0;
    char *result = NULL;
    result = strtok(str, spl);
    while(result != NULL){
        strcpy(dst[n++], result);
        result = strtok(NULL, spl);
    }
    return n;
}
