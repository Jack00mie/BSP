//
// Created by leonp on 04.06.2022.
//

#ifndef BSP_KEYVALSTORE_H
#define BSP_KEYVALSTORE_H


int put(char *key, char *value);

int get(char *key, char *res);

int del(char *key);

int initializeKeyValShM();

int dtKeyValShM();

#endif //BSP_KEYVALSTORE_H
