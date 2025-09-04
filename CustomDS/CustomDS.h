#ifndef CUSTOM_DS_H
#define CUSTOM_DS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ArrayList {
    int count;
    char **data;
} ArrayList;

ArrayList* parseTextToArrayList(const char *text, const char *delimiter);
ArrayList* getCommonArrayList(ArrayList *list1, ArrayList *list2);
void reverseArrayList(ArrayList *list);


#endif