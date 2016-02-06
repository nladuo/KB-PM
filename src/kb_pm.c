#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "process.h"
int main(int argc, char** argv)
{
    int count = -1;
    process_s process_list[256];
    parse_process_list("./test.config", process_list);
    get_process_list_count(process_list, &count);
    fprintf(stdout, "count:%d\n", count);
    del_process_by_app_name(process_list, "hello_world");
    save_process_list("haha.config", process_list);
    exit(EXIT_SUCCESS);
}
