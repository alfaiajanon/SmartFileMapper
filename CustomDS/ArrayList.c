#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CustomDS.h"

ArrayList* parseTextToArrayList(const char *text, const char *delimiter){
    ArrayList *list = malloc(sizeof(ArrayList));
    list->count = 0;
    list->data = malloc(sizeof(char*) * 16); // initial capacity

    char *textCopy = strdup(text);
    char *token = strtok(textCopy, delimiter);
    
    while (token != NULL) {
        if (list->count >= 16) { // resize if needed
            list->data = realloc(list->data, sizeof(char*) * (list->count + 16));
        }
        list->data[list->count++] = strdup(token);
        token = strtok(NULL, delimiter);
    }

    free(textCopy);
    return list;
}


void reverseArrayList(ArrayList *list) {
    for (int i = 0; i < list->count / 2; i++) {
        char *temp = list->data[i];
        list->data[i] = list->data[list->count - 1 - i];
        list->data[list->count - 1 - i] = temp;
    }
}

ArrayList* getCommonArrayList(ArrayList *list1, ArrayList *list2) {
    ArrayList *common = malloc(sizeof(ArrayList));
    common->count = 0;
    common->data = malloc(sizeof(char*) * 16); // initial capacity

    for (int i = 0; i < list1->count; i++) {
        for (int j = 0; j < list2->count; j++) { 
            if (strcmp(list1->data[i], list2->data[j]) == 0) {
                if (common->count >= 16) { 
                    common->data = realloc(common->data, sizeof(char*) * (common->count + 16));
                }
                common->data[common->count++] = strdup(list1->data[i]);
            }
        }
    }

    return common;
}