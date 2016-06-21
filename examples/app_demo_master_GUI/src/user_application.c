#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include "user_application.h"

typedef struct t_ui_params {
    int velocity;
    int acceleration;
    int deceleration;
    int target;
} t_ui_params;

void print_help(void)
{
    printf("How to use SOMANET ETHERCAT interactive master\n");
    printf("\tv: New velocity\n");
    printf("\ta: New acceleration\n");
    printf("\td: New Deceleration\n");
    printf("\tt: New target position\n");
    printf("\tp: Print the EtherCAT variables\n");
    printf("\tl: Print the local variables\n");
    printf("\tc: Commit the variables to the EtherCAT thread\n");
    printf("\tn: Set slave number\n");
    printf("\tx: Returning to zero position and exiting the application\n");
    printf("\th: Print this help\n");

}

void *user_application(void *param)
{
    ECat_parameters *parameters =  (ECat_parameters*) param;
    int slave = 0;
    int value;
    int sign;
    char mode;
    char c;

    t_ui_params ui_params[3];

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

        /*Prepare a new velocity update*/
        switch(mode)
        {
            case 'n':
            {
                if (slave >= 3)
                    printf("Wrong slave number\n");
                else
                {
                    slave = value;
                    printf("Slave: %d\n", slave);
                }
            }
            break;

            case 'v':
            {
                ui_params[slave].velocity = value*sign;
                printf("New velocity: %d\n", ui_params[slave].velocity);
            }
            break;

        /*Prepare a new acceleration update*/
            case 'a':
            {
                ui_params[slave].acceleration = value*sign;
                printf("New acceleration: %d\n", ui_params[slave].acceleration);
            }
            break;

            /*Prepare a new decelerration update*/
            case 'd':
            {
                ui_params[slave].deceleration = value*sign;
                printf("New deceleration: %d\n", ui_params[slave].deceleration);
            }
            break;

            /*Prepare a new target position update*/
            case 't':
            {
                ui_params[slave].target = value*sign;
                printf("New target position: %d\n", ui_params[slave].target);
            }
            break;

            /*Print the content of the LOCAL variables*/
            case 'l':
            {
                printf("Variables to commit:\nslave=%d\nvelocity=%d\nacceleration=%d\ndeceleration=%d\ntarget=%d\n", slave,
                        ui_params[slave].velocity, ui_params[slave].acceleration, ui_params[slave].deceleration, ui_params[slave].target);
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
                        printf("Current parameters:\nslave=%d\nvelocity=%d\nacceleration=%d\ndeceleration=%d\ntarget=%d\n",
                                parameters->slave, parameters->velocity, parameters->acceleration, parameters->deceleration, parameters->target_position);
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
                        parameters->velocity = ui_params[slave].velocity;
                        parameters->acceleration = ui_params[slave].acceleration;
                        parameters->deceleration = ui_params[slave].deceleration;
                        parameters->target_position = ui_params[slave].target;
                        parameters->slave = slave;
                        parameters->flag = true;
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

            case 'h':
            {
                print_help();
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
