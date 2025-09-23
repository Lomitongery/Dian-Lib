//
// Created by Lenovo on 2025/9/23.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

char* Read(); //用于输入字符串

int main() {
    bool bp = 1;
    const int set_year = 2002;
    while (bp) {
        //printf("输入：");
        char* str = Read();
        if (!_stricmp(str, "Quit")) bp = 0;
        else if (!_stricmp(str, "Dian")) printf("%d\n", set_year);
        else printf("Error\n");
        free(str);
    }
    return 0;
}

char* Read() {
    int buffersize = 100; //预定义缓冲区大小
    int read;
    int cnt = 0; //用于表示有效字符存放在字符数组中的位置
    char* str = (char*)malloc(buffersize * sizeof(char)); //申请动态内存分配

    //处理内存申请失败情况
    if (str == NULL) {
        perror("malloc");
        return NULL;
    }
    while ((read = getchar()) != '\n' && read != EOF) {
        str[cnt++] = (char)read;

        //处理字符串长度超缓冲区大小情况
        if (cnt >= buffersize - 1) {
            buffersize *= 2;
            char* tmp = (char*)realloc(str, buffersize * sizeof(char));

            //处理内存拓展失败情况
            if (tmp == NULL) {
                perror("realloc");
                free(str);
                return NULL;
            }
            str = tmp; //记得传递
        }
    }
    str[cnt] = '\0'; //使其成为字符串
    return str;
}