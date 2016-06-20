#ifndef USER_H
#define USER_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>

typedef struct {

    int slave;
    int target_position;
    int acceleration;
    int deceleration;
    int velocity;
    bool flag;
    pthread_mutex_t lock;

}ECat_parameters;

void *user_application(void *param);

#endif /*USER_H*/
