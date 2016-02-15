/*
  The MIT License (MIT)

  Copyright (c) 2016 Kalen Blue

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "box_drawing.h"

/*get the columns of strlen maximum */
void get_varible_strlen_max(int var_strlen_max[], char box_line_list[][VAR_NUM][STR_BUFFER_SIZE], int list_count);

/*print the box.*/
void print_box(char box_line_list[][VAR_NUM][STR_BUFFER_SIZE], int list_count);

/*print a box character.*/
void print_ch(char* ch);

/*print letter.*/
void print_box_letters(char box_line[VAR_NUM][STR_BUFFER_SIZE], int var_strlen_max[], int all_bold);

/*print the top of box.*/
void print_top_line(int[]);

/*print the middle of box.*/
void print_mid_line(int[]);

/*print the bottom of box.*/
void print_bot_line(int[]);

/*according to sec, turn it into minutes, hours or days.*/
void turn_second(time_t sec, char* buffer);

void print_process_list_box(process_s process_list[], int list_count)
{
    int i;
    time_t now;
    process_s *p;
    char box_line_list[LIST_SIZE][VAR_NUM][STR_BUFFER_SIZE]= {{" App name "," id ", " pid ", " status ", " restart ", " uptime "," memory "}};
    for(i = 0; i < list_count; i++)
    {
        p = &process_list[i];
        sprintf(box_line_list[i + 1][0], " %s ", p->app_name);
        sprintf(box_line_list[i + 1][1], " %d ", p->id);
        sprintf(box_line_list[i + 1][2], " %d ", p->pid);
        if(p->is_running)
        {
            time(&now);
            sprintf(box_line_list[i + 1][3], " %s ", "online");
            turn_second(now - p->start_time, box_line_list[i + 1][5]);
            /*sprintf(box_line_list[i + 1][5], " %lds ", (now - p->start_time));*/
            sprintf(box_line_list[i + 1][6], " %s ", p->memory);
        }
        else
        {
            sprintf(box_line_list[i + 1][3], " %s ", "stopped");
            strcpy(box_line_list[i + 1][5], " 0s ");
            strcpy(box_line_list[i + 1][6], " 0 B ");
        }
        sprintf(box_line_list[i + 1][4], " %d ", p->restart_times);
    }
    print_box(box_line_list, list_count + 1);
}

void turn_second(time_t sec, char* buffer)
{
    if(sec < 60)
    {
        /*how many seconds.*/
        sprintf(buffer, " %lds ", sec);
    }
    else if(sec < 60 * 60)
    {
        /*how many minutes.*/
        sprintf(buffer, " %ldm ", sec / 60);
    }
    else if(sec < 60 * 60 * 24)
    {
        /*how many hours.*/
        sprintf(buffer, " %ldh ", sec / 60 / 60);
    }
    else
    {
        /*how many days.*/
        sprintf(buffer, " %ldd ", sec / 60 / 60 / 24);
    }

}

void print_box(char box_line_list[][VAR_NUM][STR_BUFFER_SIZE], int list_count)
{
    int i;
    int var_strlen_max[VAR_NUM];
    get_varible_strlen_max(var_strlen_max, box_line_list, list_count);
    print_top_line(var_strlen_max);
    for (i = 0; i < list_count; i++)
    {
        if (i)
        {
            print_box_letters(box_line_list[i], var_strlen_max, 0);
        }
        else
        {
            print_box_letters(box_line_list[i], var_strlen_max, 1);
        }

        if (i < list_count - 1)
        {
            print_mid_line(var_strlen_max);
        }
    }
    print_bot_line(var_strlen_max);
}


void print_box_letters(char box_line[VAR_NUM][STR_BUFFER_SIZE], int var_strlen_max[], int all_bold)
{
    int i, j;

    print_ch(VL);
    for (i = 0; i < VAR_NUM; i++)
    {
        if (all_bold || i == 0)
        {
            printf(ANSI_COLOR_CYAN ANSI_FONT_BOLD"%s" ANSI_COLOR_RESET, box_line[i]);
        }
        else
        {
            if (i == 3)
            {
                if (strcmp(box_line[i], " online ") == 0)
                {
                    printf(ANSI_COLOR_GREEN ANSI_FONT_BOLD"%s" ANSI_COLOR_RESET, box_line[i]);
                }
                else
                {
                    printf(ANSI_COLOR_RED ANSI_FONT_BOLD"%s" ANSI_COLOR_RESET, box_line[i]);
                }
            }
            else
            {
                printf("%s", box_line[i]);
            }

        }

        for (j = 0; j < var_strlen_max[i] - strlen(box_line[i]); j++)
        {
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

    for (i = 0; i < VAR_NUM; i++)
    {
        max = 0;
        for (j = 0; j < list_count; j++)
        {
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
    printf(ANSI_FONT_BOLD ANSI_COLOR_GRAY "%s" ANSI_COLOR_RESET, ch);
}

void print_top_line(int var_strlen_max[])
{
    int i, j;
    print_ch(LT);
    for (i = 0; i < VAR_NUM; i++)
    {
        for (j = 0; j < var_strlen_max[i]; j++)
        {
            print_ch(HL);
        }
        if (i == VAR_NUM -1)
        {
            print_ch(RT);
        }
        else
        {
            print_ch(TC);
        }
    }
    printf("\n");
}

void print_bot_line(int var_strlen_max[])
{
    int i, j;
    print_ch(LB);
    for (i = 0; i < VAR_NUM; i++)
    {
        for (j = 0; j < var_strlen_max[i]; j++)
        {
            print_ch(HL);
        }
        
        if (i == VAR_NUM -1){
            print_ch(RB);
        }
        else
        {
            print_ch(BC);
        }
    }
    printf("\n");
}

void print_mid_line(int var_strlen_max[])
{
    int i, j;
    print_ch(LC);
    for (i = 0; i < VAR_NUM; i++)
    {
        for (j = 0; j < var_strlen_max[i]; j++)
        {
            print_ch(HL);
        }
        
        if (i == VAR_NUM -1){
            print_ch(RC);
        }
        else
        {
            print_ch(MC);
        }
    }
    printf("\n");
}
