#include "user_application.h"

void *user_application(void *param)
{
    ECat_parameters *parameters =  (ECat_parameters*) param;
    int velocity = 200;
    int acceleration = 200;
    int deceleration = 200;
    int target = 100000;
    char buffer[20];

    while(1)
    {

        printf("Please enter a command:\n");
        scanf("%s", buffer);

        /*Prepare a new velocity update*/
        if(strcmp(buffer,"v") == 0)
        {
            printf("Please enter a new velocity:\n");
            scanf("%d", &velocity);
        }

        /*Prepare a new acceleration update*/
        else if(strcmp(buffer,"a") == 0)
        {
            printf("Please enter a new acceleration:\n");
            scanf("%d", &acceleration);
        }

        /*Prepare a new decelerration update*/
        else if(strcmp(buffer,"d") == 0)
        {
            printf("Please enter a new deceleration:\n");
            scanf("%d", &deceleration);
        }

        /*Prepare a new target position update*/
        else if(strcmp(buffer,"p") == 0)
        {
            printf("Please enter a new target position:\n");
            scanf("%d", &target);
        }

        /*Print the content of the LOCAL variables*/
        else if(strcmp(buffer,"peek") == 0)
        {
            printf("Variables to commit:\nvelocity=%d\nvelocity=%d\ndeceleration=%d\ntarget=%d\n", velocity, acceleration, deceleration, target);
        }

        /*Print the content of the variables shared with the ethercat thread*/
        else if(strcmp(buffer,"print") == 0)
        {
            int doing = 1;
            while(doing)
            {
                if(pthread_mutex_trylock(&(parameters->lock)) == 0)
                {
                    printf("Current parameters:\nvelocity=%d\nacceleration=%d\ndeceleration=%d\ntarget=%d\n", parameters->velocity, parameters->acceleration, parameters->deceleration, parameters->target_position);
                    doing = 0;
                    pthread_mutex_unlock(&(parameters->lock)); 
                }
            }
        }

        /*Copy the content of the local variables into the shared variables*/
        else if(strcmp(buffer,"commit") == 0)
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

        /*Exit the apllication (return to the start position before shutdown)*/
        else if(strcmp(buffer,"stop") == 0)
        {
            printf("Returning to zero position and exiting the application\n");
            raise(SIGINT);
            return NULL;
        }

        /*Prepare a new acceleration update*/
        else if(strcmp(buffer,"poumon") == 0)
        {
            printf("Poumon n'as pas de traduction\n");
        }

        else
        {
            printf("Unknown command please try again\n");
        }
    }    
}