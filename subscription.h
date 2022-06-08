//
// Created by leonp on 04.06.2022.
//

#ifndef BSP_SUBSCRIPTION_H
#define BSP_SUBSCRIPTION_H

int initializeSubscriptionShM();
int subscribe(char *key);
int subService(int pid);
int notifySubscribers(char *key, char *msg);
int initializeMsg();

#endif //BSP_SUBSCRIPTION_H
