#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "box_drawing.h"

void get_varible_strlen_max(int var_strlen_max[], char box_line_list[][VAR_NUM][STR_BUFFER_SIZE], int list_count);
void print_box(char box_line_list[][VAR_NUM][STR_BUFFER_SIZE], int list_count);
void print_ch(char* ch);
void print_box_letters(char box_line[VAR_NUM][STR_BUFFER_SIZE], int var_strlen_max[], int all_bold);
void print_top_line(int[]);
void print_mid_line(int[]);
void print_bot_line(int[]);

void print_process_list_box(process_s process_list[], int list_count)
{
    int i;
    time_t now;
    process_s *p;
    char box_line_list[LIST_SIZE][VAR_NUM][STR_BUFFER_SIZE]= {{" App name "," id ", " pid ", " status ", " restart ", " uptime "," memory "}};
    for(i = 0; i < list_count; i++){
        p = &process_list[i];
        sprintf(box_line_list[i + 1][0], " %s ", p->app_name);
        sprintf(box_line_list[i + 1][1], " %d ", p->id);
        sprintf(box_line_list[i + 1][2], " %d ", p->pid);
        if(p->is_running){
            time(&now);
            sprintf(box_line_list[i + 1][3], " %s ", "online");
            sprintf(box_line_list[i + 1][5], " %lds ", (now - p->start_time));
            sprintf(box_line_list[i + 1][6], " %s ", p->memory);
        }else{
            sprintf(box_line_list[i + 1][3], " %s ", "stopped");
            strcpy(box_line_list[i + 1][5], " 0s ");
            strcpy(box_line_list[i + 1][6], " 0 B ");
        }
        sprintf(box_line_list[i + 1][4], " %d ", p->restart_times);
    }
    print_box(box_line_list, list_count + 1);
}

void print_box(char box_line_list[][VAR_NUM][STR_BUFFER_SIZE], int list_count)
{
    int i;
    int var_strlen_max[VAR_NUM];
    get_varible_strlen_max(var_strlen_max, box_line_list, list_count);
    print_top_line(var_strlen_max);
    for (i = 0; i < list_count; i++){
        if (i){
            print_box_letters(box_line_list[i], var_strlen_max, 0);
        }else{
            print_box_letters(box_line_list[i], var_strlen_max, 1);
        }

        if (i < list_count - 1){
            print_mid_line(var_strlen_max);
        }
    }
    print_bot_line(var_strlen_max);
}


void print_box_letters(char box_line[VAR_NUM][STR_BUFFER_SIZE], int var_strlen_max[], int all_bold)
{
    int i, j;

    print_ch(VL);
    for (i = 0; i < VAR_NUM; i++){
        if (all_bold || i == 0){
            printf(ANSI_COLOR_CYAN ANSI_FONT_BOLD"%s" ANSI_COLOR_RESET, box_line[i]);
        }else{
            if (i == 3){
                if (strcmp(box_line[i], " online ") == 0){
                    printf(ANSI_COLOR_GREEN ANSI_FONT_BOLD"%s" ANSI_COLOR_RESET, box_line[i]);
                }else{
                    printf(ANSI_COLOR_RED ANSI_FONT_BOLD"%s" ANSI_COLOR_RESET, box_line[i]);
                }
            }else{
                printf("%s", box_line[i]);
            }

        }

        for (j = 0; j < var_strlen_max[i] - strlen(box_line[i]); j++){
            printf(" ");
        }
        print_ch(VL);
    }
    printf("\n");
}

void get_varible_strlen_max(int var_strlen_max[], char box_line_list[][VAR_NUM][STR_BUFFER_SIZE], int list_count)
{
    int i, j;
    int max;
    int str_len;

    for (i = 0; i < VAR_NUM; i++){
        max = 0;
        for (j = 0; j < list_count; j++){
            str_len = strlen(box_line_list[j][i]);
            if (str_len > max){
                max = str_len;
            }
        }
        var_strlen_max[i] = max;
    }
}



void print_ch(char* ch)
{
    printf(ANSI_FONT_FAINT ANSI_COLOR_GRAY "%s" ANSI_COLOR_RESET, ch);
}

void print_top_line(int var_strlen_max[])
{
    int i, j;
    print_ch(LT);
    for (i = 0; i < VAR_NUM; i++){
        for (j = 0; j < var_strlen_max[i]; j++){
            print_ch(HL);
        }
        if (i == VAR_NUM -1){
            print_ch(RT);
        }else{
            print_ch(TC);
        }
    }
    printf("\n");
}

void print_bot_line(int var_strlen_max[])
{
    int i, j;
    print_ch(LB);
    for (i = 0; i < VAR_NUM; i++){
        for (j = 0; j < var_strlen_max[i]; j++){
            print_ch(HL);
        }
        if (i == VAR_NUM -1){
            print_ch(RB);
        }else{
            print_ch(BC);
        }
    }
    printf("\n");
}

void print_mid_line(int var_strlen_max[])
{
    int i, j;
    print_ch(LC);
    for (i = 0; i < VAR_NUM; i++){
        for (j = 0; j < var_strlen_max[i]; j++){
            print_ch(HL);
        }
        if (i == VAR_NUM -1){
            print_ch(RC);
        }else{
            print_ch(MC);
        }
    }
    printf("\n");
}
