//
// Created by leonp on 04.06.2022.
//

#ifndef BSP_SUB_H
#define BSP_SUB_H

int dtSubShM();
int executeCommand(const char *commandAndInput);
int initializeSubShM();
int writeMsg(char *msg);
int initializeSocket(int cfd);

#endif //BSP_SUB_H
