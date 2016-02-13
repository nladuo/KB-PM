#ifndef __UTILS_H
#define __UTILS_H

#ifndef STR_BUFFER_SIZE
#define STR_BUFFER_SIZE 512
#endif /*STR_BUFFER_SIZE*/

/*split the str by spl, save into dst, and return the dst size.*/
extern int str_split(char dst[][STR_BUFFER_SIZE], char* str, const char* spl);


#endif /*__UTILS_H*/
