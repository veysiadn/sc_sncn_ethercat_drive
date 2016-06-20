/**
 * @file main.c
 * @brief Example Master App for Cyclic Synchronous Position (on PC)
 * @author Synapticon GmbH (www.synapticon.com)
 */

#include <ctrlproto_m.h>
#include <ecrt.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <profile.h>
#include <drive_function.h>
#include <motor_define.h>
#include <sys/time.h>
#include <time.h>
#include "ethercat_setup.h"
#include "user_application.h"
#include <gtk/gtk.h>

enum {ECAT_SLAVE_0=0, ECAT_SLAVE_1, ECAT_SLAVE_2};

typedef struct t_slave_data {
    int slave_num;
    int acc;        // rpm/s
    int dec;        // rpm/s
    int velocity;   // rpm
    int act_pos;    // ticks
    int zero_pos;   // ticks
    int target_pos; // ticks
    int start_pos;
    int end_pos;
    int act_velocity;   // rpm
    float act_torque;   // mNm
    int steps;
    int inc_steps;
    int next_target_pos;    // ticks
    int relative_target_pos;
    int position_ramp;
    int n_init_ticks;
    int direction;
    bool new_target;
} t_slave_data;

/* Only here for interrupt signaling */
bool break_loop = false;

/* Interrupt signal handler */
void  INThandler(int sig)
{
     signal(sig, SIG_IGN);
     break_loop = true;
     signal(SIGINT, INThandler);
}

void init_params(t_slave_data params[], int len)
{
    int slave;

    for(slave=0; slave < len; slave++)
    {
        params[slave].slave_num = slave;
        params[slave].acc = 20;
        params[slave].dec = 20;
        params[slave].velocity = 20;
        params[slave].act_pos = 0;
        params[slave].zero_pos = 0;
        params[slave].start_pos = 0;
        params[slave].end_pos = 0;
        params[slave].target_pos = 16000;
        params[slave].act_velocity = 0;
        params[slave].act_torque = 0.0;
        params[slave].steps = 0;
        params[slave].inc_steps = 0;
        params[slave].next_target_pos = 0;
        params[slave].n_init_ticks = 3;
        params[slave].position_ramp = 0;
        params[slave].direction = 1;
        params[slave].relative_target_pos = 0;
        params[slave].new_target = true;
    }
}

int main() 
{
    t_slave_data slave_1, slave_2, slave_3;
    ECat_parameters ui_params;

    t_slave_data slave_main_params[3] = {slave_1, slave_2, slave_3};
    bool absolute_position_taken = false;
    int node, delay_inc;

    pthread_t user_application_thread;

    ui_params.target_position = 16000; // ticks
    ui_params.acceleration = 20; // rpm/s
    ui_params.deceleration = 20; // rpm/s
    ui_params.velocity = 20; // rpm
    ui_params.slave = 0;

    init_params(slave_main_params, TOTAL_NUM_OF_SLAVES);

    /* Initialize the mutex */
    if(pthread_mutex_init(&(ui_params.lock), NULL) != 0)
    {
        printf("Mutex init failed!\n");
        return -1;
    }

    /* Initialize EtherCAT Master */
    init_master(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Initialize all connected nodes with Mandatory Motor Configurations (specified in config)*/
    init_nodes(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
    {
        /* Initialize torque parameters */
        initialize_torque(node, slv_handles);

        /* Initialize the node specified with ECAT_SLAVE_0 with CSP configurations (specified in config)*/
        set_operation_mode(CSP, node, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

        /* Enable operation of node in CSP mode */
        enable_operation(node, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

        /* Initialize position profile parameters */
        initialize_position_profile_limits(node, slv_handles);
    }

    pthread_create(&user_application_thread, NULL, user_application, &ui_params);

    /* Just for better printing result */
    printf("\n");

    for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
    {
        /* Getting actual position */
        slave_main_params[node].act_pos = get_position_actual_ticks(node, slv_handles);
        slave_main_params[node].start_pos = slave_main_params[node].act_pos;
        //printf("Our start position for slave %d: %i ticks\n", node, slave_main_params[node].start_pos);
    }

    /* catch interrupt signal */
    signal(SIGINT, INThandler);

    /* Moving one rotation back and forth */
    while(1)
    {
        pthread_mutex_lock(&(ui_params.lock));

        if (ui_params.flag)
        {
            /* Compute a target position */
            slave_main_params[ui_params.slave].target_pos = slave_main_params[ui_params.slave].start_pos + slave_main_params[ui_params.slave].direction
                    * ui_params.target_position;

//            /* Compute steps needed for the target position */
//            slave_main_params[ui_params.slave].steps = init_position_profile_params(slave_main_params[ui_params.slave].relative_target_pos,
//                    slave_main_params[ui_params.slave].act_pos, ui_params.velocity, ui_params.acceleration,
//                    ui_params.deceleration, ui_params.slave, slv_handles);
            slave_main_params[ui_params.slave].velocity = ui_params.velocity;
            slave_main_params[ui_params.slave].acc = ui_params.acceleration;
            slave_main_params[ui_params.slave].dec = ui_params.deceleration;

            ui_params.flag = false;
        }
        pthread_mutex_unlock(&(ui_params.lock));

        /* Update the process data (EtherCat packets) sent/received from the node */
        pdo_handle_ecat(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

        if(master_setup.op_flag && !break_loop)    /*Check if the master is active*/
        {
            for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
            {
                if (slave_main_params[node].new_target)
                { //has to be done only once for a new target value
                    /* Read Actual Position from the node for initialization */
                    if (!absolute_position_taken) {
                        //printf("taking abs position\n");
                        slave_main_params[node].zero_pos = get_position_actual_ticks(slave_main_params[node].slave_num, slv_handles);
                        pdo_handle_ecat(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
                        absolute_position_taken = true;
    //                    printf("abs positions are taken: \n%i\n%i\n%i\n",
    //                            slave_main_params[0].zero_pos, slave_main_params[1].zero_pos, slave_main_params[2].zero_pos);
                    }

                    /* Setup Target Position */
                    slave_main_params[node].target_pos = slave_main_params[node].zero_pos + slave_main_params[node].direction * slave_main_params[node].target_pos;
                    /* Read Actual Position */
                    slave_main_params[node].act_pos = get_position_actual_ticks(slave_main_params[node].slave_num, slv_handles);

                    /* Compute steps needed for the target position */
                    slave_main_params[node].steps = init_position_profile_params(slave_main_params[node].target_pos, slave_main_params[node].act_pos,
                                slave_main_params[node].velocity, slave_main_params[node].acc, slave_main_params[node].dec, slave_main_params[node].slave_num, slv_handles);

//                        printf("\rdrive %d: steps %d target_position %d actual_position %d                                    ",
//                                slave_main_params[node].slave_num+1, slave_main_params[node].steps, slave_main_params[node].target_pos, slave_main_params[node].act_pos);
                     slave_main_params[node].new_target = false;
                }


                if(slave_main_params[node].inc_steps < slave_main_params[node].steps)
                {
                    /* Generate target position steps */
                    slave_main_params[node].next_target_pos =  generate_profile_position(slave_main_params[node].inc_steps,
                            slave_main_params[node].slave_num, slv_handles);

                    /* Send target position for the node specified by slave_number */
                    set_position_ticks(slave_main_params[node].next_target_pos, slave_main_params[node].slave_num, slv_handles);
                    slave_main_params[node].inc_steps = slave_main_params[node].inc_steps + 1;

                    /* Read actual node sensor values */
                    slave_main_params[node].act_pos = get_position_actual_ticks(slave_main_params[node].slave_num, slv_handles);
                    slave_main_params[node].act_velocity = get_velocity_actual_rpm(slave_main_params[node].slave_num, slv_handles);
                    slave_main_params[node].act_torque = get_torque_actual_mNm(slave_main_params[node].slave_num, slv_handles);
//                    printf("DRIVE %d: pos:%7.d  vel:%6.d  tq:%6.2f    ", node, slave_main_params[node].act_pos,
//                            slave_main_params[node].act_velocity, slave_main_params[node].act_torque);
                }

                if(slave_main_params[node].inc_steps >= slave_main_params[node].steps)
                {
                    delay_inc++;
                    if(delay_inc > 500)//some delay to hold the position
                    {
                        slave_main_params[node].direction *= -1;
                        slave_main_params[node].new_target = true;
//                        printf("New target x: %d", slave_main_params[node].direction* slave_main_params[node].target_pos);
                        slave_main_params[node].inc_steps = 1;
                        delay_inc = 0;
                    }
                }
            }
            //printf("\r");
        }
        else if (break_loop) {
            break;
        }

    }
    break_loop = false;

    printf("\nReturn to start position\n");
    /* return to start position */
    while ( ((slave_main_params[ECAT_SLAVE_0].act_pos - slave_main_params[ECAT_SLAVE_0].start_pos) > 200
            || (slave_main_params[ECAT_SLAVE_0].act_pos - slave_main_params[ECAT_SLAVE_0].start_pos) < -200)
         && ((slave_main_params[ECAT_SLAVE_1].act_pos - slave_main_params[ECAT_SLAVE_1].start_pos) > 200
            || (slave_main_params[ECAT_SLAVE_1].act_pos - slave_main_params[ECAT_SLAVE_1].start_pos) < -200)
         && ((slave_main_params[ECAT_SLAVE_2].act_pos - slave_main_params[ECAT_SLAVE_2].start_pos) > 200
            || (slave_main_params[ECAT_SLAVE_2].act_pos - slave_main_params[ECAT_SLAVE_2].start_pos) < -200))
    {
        if (break_loop)
            break;

        pthread_mutex_lock(&(ui_params.lock));
        if (ui_params.flag)
        {
            /* Compute steps needed for the target position */
            slave_main_params[ui_params.slave].steps = init_position_profile_params(slave_main_params[ui_params.slave].start_pos,
                    slave_main_params[ui_params.slave].act_pos, ui_params.velocity, ui_params.acceleration,
                    ui_params.deceleration, ui_params.slave, slv_handles);
            ui_params.flag = false;
        }

        pthread_mutex_unlock(&(ui_params.lock));

        for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
        {
            if(slave_main_params[node].inc_steps < slave_main_params[node].steps)
            {
                /* Generate target position steps */
                slave_main_params[node].next_target_pos = generate_profile_position(slave_main_params[node].inc_steps, slave_main_params[node].slave_num, slv_handles);

                /* Send target position for the node specified by slave_number */
                set_position_ticks(slave_main_params[node].next_target_pos, slave_main_params[node].slave_num, slv_handles);
                slave_main_params[node].inc_steps++;

                /* Read actual node sensor values */
                slave_main_params[node].act_pos = get_position_actual_ticks(slave_main_params[node].slave_num,
                        slv_handles);
                slave_main_params[node].act_velocity = get_velocity_actual_rpm(slave_main_params[node].slave_num,
                        slv_handles);
                slave_main_params[node].act_torque = get_torque_actual_mNm(slave_main_params[node].slave_num, slv_handles);
            }
        }
    }

    printf("\n");

    /* Quick stop position mode (for emergency) */
    for (node=0; node < TOTAL_NUM_OF_SLAVES; node++) {
        printf("Stop Slave %d...\n", node);
        quick_stop_position(slave_main_params[node].slave_num, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Regain control of node to continue after quick stop */
    //for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
        renable_ctrl_quick_stop(CSP, slave_main_params[node].slave_num, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES); //after quick-stop

    //for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
        set_operation_mode(CSP, slave_main_params[node].slave_num, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    //for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
        enable_operation(slave_main_params[node].slave_num, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Shutdown node operations */
    //for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
        shutdown_operation(CSP, slave_main_params[node].slave_num, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
        printf("\n;");
    }
    pdo_handle_ecat(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    pthread_cancel(user_application_thread);
    pthread_mutex_destroy(&(ui_params.lock));

    /* Just for better printing result */
    //system("setterm -cursor on");

    return 0;
}

