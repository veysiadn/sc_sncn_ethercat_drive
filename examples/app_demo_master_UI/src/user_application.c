#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include "user_application.h"

typedef struct t_ui_params {
    int velocity;
    int acceleration;
    int deceleration;
    int end_pos1;
    int end_pos2;
} t_ui_params;

void print_help(void)
{
    printf("How to use " PRINT_BOLD"SOMANET ETHERCAT"PRINT_END " interactive master\n");
    printf(PRINT_BOLD"\tv"PRINT_END": New velocity\n");
    printf(PRINT_BOLD"\ta"PRINT_END": New acceleration\n");
    printf(PRINT_BOLD"\td"PRINT_END": New Deceleration\n");
    printf(PRINT_BOLD"\ts"PRINT_END": New end 1 position\n");
    printf(PRINT_BOLD"\te"PRINT_END": New end 2 position\n");
    printf(PRINT_BOLD"\tp"PRINT_END": Print the EtherCAT variables\n");
    printf(PRINT_BOLD"\tl"PRINT_END": Print the local variables\n");
    printf(PRINT_BOLD"\tc"PRINT_END": Commit the variables to the EtherCAT thread\n");
    printf(PRINT_BOLD"\tn"PRINT_END": Set slave number\n");
    printf(PRINT_BOLD"\tx"PRINT_END": Returning to zero position and exiting the application\n");
    printf(PRINT_BOLD"\th"PRINT_END": Print this help\n");

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
    printf("Enter 'h' to show the help\n");

    while(1)
    {
        int value = 0;
        int sign = 1;
        char mode;
        printf(PRINT_BOLD"\nPlease enter a command:\n"PRINT_END);
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
                if (value >= 3)
                    printf(PRINT_FAIL"Wrong slave number\n"PRINT_END);
                else
                {
                    slave = value;
                    printf(PRINT_OK"OK! "PRINT_END"Slave: %d\n", slave);
                }
            }
            break;

            case 'v':
            {
                ui_params[slave].velocity = value*sign;
                printf(PRINT_OK"OK! "PRINT_END"New velocity: %d\n", ui_params[slave].velocity);
            }
            break;

        /*Prepare a new acceleration update*/
            case 'a':
            {
                ui_params[slave].acceleration = value*sign;
                printf(PRINT_OK"OK! "PRINT_END"New acceleration: %d\n", ui_params[slave].acceleration);
            }
            break;

            /*Prepare a new decelerration update*/
            case 'd':
            {
                ui_params[slave].deceleration = value*sign;
                printf(PRINT_OK"OK! "PRINT_END"New deceleration: %d\n", ui_params[slave].deceleration);
            }
            break;

            /*Prepare a new start position update*/
            case 's':
            {
                ui_params[slave].end_pos1 = value*sign;
                printf(PRINT_OK"OK! "PRINT_END"New end position 1: %d\n", ui_params[slave].end_pos1);
            }
            break;

            /*Prepare a new end position update*/
             case 'e':
             {
                 ui_params[slave].end_pos2 = value*sign;
                 printf(PRINT_OK"OK! "PRINT_END"New end position 2: %d\n", ui_params[slave].end_pos2);
             }
             break;

            /*Print the content of the LOCAL variables*/
            case 'l':
            {
                printf(PRINT_BOLD"Variables to commit:"PRINT_END"\nslave: %d\nvelocity: %d\nacceleration: %d\ndeceleration: %d\nend position 1: %d\nend position 2: %d\n", slave,
                        ui_params[slave].velocity, ui_params[slave].acceleration, ui_params[slave].deceleration, ui_params[slave].end_pos1, ui_params[slave].end_pos2);
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
                        printf(PRINT_BOLD"Current parameters:"PRINT_END"\nslave: %d\nvelocity: %d\nacceleration: %d\ndeceleration: %d\nend position 1: %d\nend position 2: %d\n",
                                parameters->slave, parameters->velocity, parameters->acceleration, parameters->deceleration, parameters->end_pos1, parameters->end_pos2);
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
                        parameters->end_pos1 = ui_params[slave].end_pos1;
                        parameters->end_pos2 = ui_params[slave].end_pos2;
                        parameters->slave = slave;
                        parameters->flag = true;
                        printf(PRINT_OK"Parameters updated\n"PRINT_END);
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
                printf(PRINT_FAIL"Unknown command please try again\n\n"PRINT_END);
                print_help();
            }
            break;
        }
    }    
}
