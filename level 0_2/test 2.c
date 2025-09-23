//
// Created by Lenovo on 2025/9/23.
//
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_LINE 256
#define MAX_KEY 20
#define MAX_VALUE 20

typedef struct {
    char key[MAX_KEY];
    char value[MAX_VALUE];
}KeyValue;

int loadFile(const char* filename, KeyValue data[], int maxsize);

const char* findValue(const char* str, KeyValue data[], int size);

int main() {
    KeyValue data[100];
    int count;
    char* filename = (char*)malloc(100 * sizeof(char));
    printf("Please input the name of the file: ");
    scanf("%s", filename);
    //printf("%s", filename);
    count = loadFile(filename, data, 100);
    if (!count) {
        printf("The file is empty, so no data was read.\n");
        return 0;
    }
    printf("File finally read successfully, proceeding to next step.\n");

    char input[MAX_KEY];
    while (1) {
        scanf("%s", input);
        int len = strlen(input);
        input[len] = '\0';
        if (!strcmp(input, "Quit")) {
            printf("Exited successfully.\n");
            break;
        }

        printf("%s\n", findValue(input, data, count));
    }
    return 0;
}

int loadFile(const char* filename, KeyValue data[], int maxsize) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("An error occurred: file read failed.\n");
        return 0;
    }
    char line[MAX_LINE];
    int count = 0;

    while (fgets(line, MAX_LINE * sizeof(char), fp) != NULL && count < maxsize) {
        char* pos;
        if ((pos = strchr(line, '\n')) != NULL) *pos = '\0';
        if (!strlen(line)) continue;

        char* colon = strchr(line, ':');
        if (colon == NULL) {
            printf("Error:Skip invalid line -> %s\n", line);
            continue;
        }

        *colon = '\0';
        char* key = line;
        char* value = colon + 1;

        if (strlen(key) > 10 || strlen(value) > 10) {
            printf("Error:The length of the key or value is too large.-> %s:%s\n", key, value);
            continue;
        }

        strncpy(data[count].key, key, MAX_KEY - 1);
        strncpy(data[count].value, value, MAX_VALUE - 1);
        data[count].key[MAX_KEY - 1] = '\0';
        data[count].value[MAX_VALUE - 1] = '\0';

        count++;
    }
    fclose(fp);
    return count;
}

const char* findValue(const char* str, KeyValue data[], int size) {
    for (int i = 0; i < size; i++) {
        if (!strcmp(str, data[i].key)) {
            return data[i].value;
        }
    }
    return "Error";
}
