#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "utils.h"
#include "proc_status.h"

#define PROC_DIR "/proc/"

void get_child_pids(pid_t parent_pid, pid_t pids[], int *p_pids_len)
{
    DIR *dp;
    struct dirent *entry;
    struct stat stat_buf;
    pid_t pid;
    char buf[STR_BUFFER_SIZE];
    char parent_pid_str[STR_BUFFER_SIZE];
    int pids_len = 0;

    *p_pids_len = -1;

    /*transfer pid to char*/
    sprintf(parent_pid_str, "%d", parent_pid);

    if((dp = opendir(PROC_DIR)) == NULL)
    {
        return;
    }
    chdir(PROC_DIR);
    while((entry = readdir(dp)) != NULL)
    {
        lstat(entry->d_name, &stat_buf);
        if(S_ISDIR(stat_buf.st_mode))
        {
            if(!is_number(entry->d_name))
            {
                continue;
            }
            pid = atoi(entry->d_name);
            if(pid > 0 && pid <= 32767)
            {
                parse_proc_status_val_by_tag(pid, "PPid", buf);
                if(strcmp(buf, ERR_RESULT) != 0
                    && strcmp(parent_pid_str, buf) == 0)
                {
                    //fprintf(stdout, "ppid:%s, pid:%s\n", buf, entry->d_name);
                    pids[pids_len++] = pid;
                }
            }
        }
    }
    *p_pids_len = pids_len;
}

void parse_proc_status_val_by_tag(pid_t pid, char* tag , char* result_val)
{
    FILE *fp;
    int flag = 0;
    char buf[STR_BUFFER_SIZE];
    char path[STR_BUFFER_SIZE];
    char *p_buf;

    sprintf(path, "/proc/%d/status", pid);

    fp = fopen(path, "r");
    while(fgets(buf, STR_BUFFER_SIZE, fp) > 0)
    {
        if (strncmp(buf, tag, strlen(tag)) == 0)
        {
            /*the buf[strlen(buf)-1] should be '\n', replace it with '\0' */
            buf[strlen(buf)-1] = 0;

            flag = 1;
            p_buf = buf;
            while(!(*p_buf >= '0' && *p_buf <= '9')) p_buf++;
            strcpy(result_val, p_buf);
            break;
        }
    }
    fclose(fp);
    if (!flag)
    {
        strcpy(result_val, ERR_RESULT);
    }

}
