#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"

int main()
{
    process_s process;
    char json_str[1000];
    int res, i;

    strcpy(process.app_name, "hello");
    strcpy(process.cmd, "./a.out");
    strcpy(process.dir, "/home/kalen");
    process.is_running = 0;

    res = create_process_json_str(&process, json_str);
    if(res == -1){
        printf("1:parse failed.\n");
        exit(EXIT_FAILURE);
    }

    printf("%s\n\n", json_str);

    process_s process_list[100];
    char json_str2[1000];
    res = parse_process_list_from_path("./test.json", process_list);
    if(res == -1){
        printf("parse failed.\n");
        exit(EXIT_FAILURE);
    }
    for(i = 0; i < 2; i++){
        process = process_list[i];
        printf(" %d :%s  %s  %s  %d\n", i, process.app_name,
            process.cmd, process.dir, process.is_running);
    }
    exit(EXIT_SUCCESS);
}
