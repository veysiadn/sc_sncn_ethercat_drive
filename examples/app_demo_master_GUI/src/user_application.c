#include <stdio.h>
#include <ctype.h>
#include "user_application.h"


void print_help(void)
{
    printf("How to use SOMANET ETHERCAT interactive master\n");
    printf("v:\tNew velocity\n");
    printf("a:\tNew acceleration\n");
    printf("d:\tNew Deceleration\n");
    printf("t:\tNew target position\n");
    printf("p:\tPrint the EtherCAT variables\n");
    printf("l:\tPrint the local variables\n");
    printf("c:\tCommit the variables to the EtherCAT thread\n");
    printf("x:\tReturning to zero position and exiting the application\n");
    printf("h:\tPrint this help\n");

}

void *user_application(void *param)
{
    ECat_parameters *parameters =  (ECat_parameters*) param;
    int velocity = 200;
    int acceleration = 200;
    int deceleration = 200;
    int target = 100000;
    char buffer[20];
    int value;
    int sign;
    char mode;
    char c;

    printf("\n*** Interactive Mode ***\n");

    while(1)
    {
        int value = 0;
        int sign = 1;
        char mode;
        printf("\nPlease enter a command:\n");
        while((c = getchar ()) != '\n'){
            if(isdigit(c)>0){
                value *= 10;
                value += c - '0';
            } else if (c == '-') {
                sign = -1;
            } else if (c != ' ')
                mode = c;
        }
        //scanf("%s", buffer);

        /*Prepare a new velocity update*/
        switch(mode)
        {
            case 'v':
            {
                velocity = value*sign;
                printf("\nNew velocity: %d\n", velocity);
            }
            break;

        /*Prepare a new acceleration update*/
            case 'a':
            {
//                scanf("%d", &acceleration);
                acceleration = value*sign;
                printf("\nNew acceleration: %d\n", acceleration);
            }
            break;

            /*Prepare a new decelerration update*/
            case 'd':
            {
//                scanf("%d", &deceleration);
                deceleration = value*sign;
                printf("New deceleration: %d\n", deceleration);
            }
            break;

            /*Prepare a new target position update*/
            case 't':
            {
//                scanf("%d", &target);
                target = value*sign;
                printf("New target position: %d\n", target);
            }
            break;

            /*Print the content of the LOCAL variables*/
            case 'l':
            {
                printf("\nVariables to commit:\nvelocity=%d\nvelocity=%d\ndeceleration=%d\ntarget=%d\n", velocity, acceleration, deceleration, target);
            }
            break;

            /*Print the content of the variables shared with the ethercat thread*/
            case 'p':
            {
                int doing = 1;
                while(doing)
                {
                    if(pthread_mutex_trylock(&(parameters->lock)) == 0)
                    {
                        printf("\nCurrent parameters:\nvelocity=%d\nacceleration=%d\ndeceleration=%d\ntarget=%d\n", parameters->velocity, parameters->acceleration, parameters->deceleration, parameters->target_position);
                        doing = 0;
                        pthread_mutex_unlock(&(parameters->lock));
                    }
                }
            }
            break;

            /*Copy the content of the local variables into the shared variables*/
            case 'c':
            {
                int doing = 1;
                while(doing)
                {
                    if(pthread_mutex_trylock(&(parameters->lock)) == 0)
                    {
                        parameters->velocity = velocity;
                        parameters->acceleration = acceleration;
                        parameters->deceleration = deceleration;
                        parameters->target_position = target;
                        printf("Parameters updated\n");
                        doing = 0;
                        pthread_mutex_unlock(&(parameters->lock));
                    }
                }
            }
            break;

            /*Exit the apllication (return to the start position before shutdown)*/
            case 'x':
            {
                printf("Returning to zero position and exiting the application\n");
                raise(SIGINT);
                return NULL;
            }
            break;

            default:
            {
                printf("Unknown command please try again\n");
            }
            break;
        }
    }    
}
