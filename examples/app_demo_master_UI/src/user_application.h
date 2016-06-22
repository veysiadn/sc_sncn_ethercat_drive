#ifndef USER_H
#define USER_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>

#define PRINT_BOLD  "\033[1m"
#define PRINT_END   "\033[0m"
#define PRINT_OK    "\033[92m"
#define PRINT_FAIL  "\033[91m"


typedef struct {

    int slave;
    int end_pos1;
    int end_pos2;
    int acceleration;
    int deceleration;
    int velocity;
    bool flag;
    pthread_mutex_t lock;

}ECat_parameters;

void *user_application(void *param);

#endif /*USER_H*/
