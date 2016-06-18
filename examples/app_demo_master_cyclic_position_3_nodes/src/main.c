/**
 * @file main.c
 * @brief Example Master App for Cyclic Synchronous Position (on PC)
 * @author Synapticon GmbH (www.synapticon.com)
 */

#include <ctrlproto_m.h>
#include <ecrt.h>
#include <stdio.h>
#include <stdbool.h>
#include <profile.h>
#include <drive_function.h>
#include <motor_define.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include "ethercat_setup.h"

enum {ECAT_SLAVE_0=0, ECAT_SLAVE_1, ECAT_SLAVE_2};

typedef struct t_node_data {
    int slave_num;
    int acc;        // rpm/s
    int dec;        // rpm/s
    int velocity;   // rpm
    int act_pos;    // ticks
    int zero_pos;   // ticks
    int target_pos; // ticks
    int act_velocity;   // rpm
    float act_torque;   // mNm
    int steps_drive;
    int inc_drive;
    int next_target_pos;    // ticks
} t_node_data;

/* Only here for interrupt signaling */
bool break_loop = false;

/* Interrupt signal handler */
void  INThandler(int sig)
{
     signal(sig, SIG_IGN);
     break_loop = true;
     signal(SIGINT, INThandler);
}


int main()
{
    t_node_data node_1, node_2, node_3;

    node_1.slave_num = 0;
    node_1.acc = 20;
    node_1.dec = 20;
    node_1.velocity = 20;
    node_1.act_pos = 0;
    node_1.zero_pos = 0;
    node_1.target_pos = 0;
    node_1.act_velocity = 0;
    node_1.act_torque = 0.0;
    node_1.steps_drive = 0;
    node_1.inc_drive = 0;
    node_1.next_target_pos = 0;

    node_2.slave_num = 1;
    node_2.acc = 20;
    node_2.dec = 20;
    node_2.velocity = 20;
    node_2.act_pos = 0;
    node_2.zero_pos = 0;
    node_2.target_pos = 0;
    node_2.act_velocity = 0;
    node_2.act_torque = 0.0;
    node_2.steps_drive = 0;
    node_2.inc_drive = 0;
    node_2.next_target_pos = 0;

    node_3.slave_num = 2;
    node_3.acc = 20;
    node_3.dec = 20;
    node_3.velocity = 20;
    node_3.act_pos = 0;
    node_3.zero_pos = 0;
    node_3.target_pos = 0;
    node_3.act_velocity = 0;
    node_3.act_torque = 0.0;
    node_3.steps_drive = 0;
    node_3.inc_drive = 0;
    node_3.next_target_pos = 0;

    t_node_data slaves[3] = {node_1, node_2, node_3};

    int node;
    int one_rotation = 16384;
    bool absolute_position_taken = false;

    bool new_target = true;
    int delay_inc = 0;

    /* Initialize EtherCAT Master */
    init_master(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Initialize torque parameters */
    for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
        initialize_torque(node, slv_handles);

    /* Initialize all connected nodes with Mandatory Motor Configurations (specified in config)*/
    init_nodes(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Initialize the node specified with slave_number with CSP configurations (specified in config)*/
    for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
        set_operation_mode(CSP, node, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Enable operation of node in CSP mode */
    for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
        enable_operation(node, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Initialize position profile parameters */
    for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
        initialize_position_profile_limits(node, slv_handles);

    /* catch interrupt signal */
    signal(SIGINT, INThandler);

    /* Just for better printing result */
    system("setterm -cursor off");

    while(1)
    {
        /* Update the process data (EtherCat packets) sent/received from the node */
        pdo_handle_ecat(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

        if(master_setup.op_flag && !break_loop)    /*Check if the master is active*/
        {
            if (new_target) { //has to be done only once for a new target value

                /* Read Actual Position from the node for initialization */
                if (!absolute_position_taken) {
                    printf("taking abs position\n");
                    for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
                        slaves[node].zero_pos = get_position_actual_ticks(slaves[node].slave_num, slv_handles);
                    pdo_handle_ecat(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);
                    absolute_position_taken = true;
                    printf("abs positions are taken: \n%i\n%i\n%i\n", slaves[node].zero_pos, node_2.zero_pos, node_3.zero_pos);
                }

                /* Setup Target Position */
                for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
                    slaves[node].target_pos = slaves[node].zero_pos + one_rotation;

                /* Read Actual Position */
                for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
                    slaves[node].act_pos = get_position_actual_ticks(slaves[node].slave_num, slv_handles);

                /* Compute steps needed for the target position */
                for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
                    slaves[node].steps_drive = init_position_profile_params(slaves[node].target_pos, slaves[node].act_pos,
                                slaves[node].velocity, slaves[node].acc, slaves[node].dec, slaves[node].slave_num, slv_handles);

                for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
                    printf("\rdrive %d: steps %d target_position %d actual_position %d                                    \n",
                            slaves[node].slave_num+1, slaves[node].steps_drive, slaves[node].target_pos, slaves[node].act_pos);
                //printf("\n");
                new_target = false;
            }

            for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
            {
                if(slaves[node].inc_drive < slaves[node].steps_drive)
                {
                    /* Generate target position steps */
                    slaves[node].next_target_pos =  generate_profile_position(slaves[node].inc_drive, slaves[node].slave_num, slv_handles);

                    /* Send target position for the node specified by slave_number */
                    set_position_ticks(slaves[node].next_target_pos, slaves[node].slave_num, slv_handles);
                    slaves[node].inc_drive = slaves[node].inc_drive + 1;
                }
            }

            if(slaves[0].inc_drive >= slaves[0].steps_drive
               && slaves[1].inc_drive >= slaves[1].steps_drive
               && slaves[2].inc_drive >= slaves[2].steps_drive)
            {
              delay_inc++;
              if(delay_inc > 500)//some delay to hold the position
              {
                  /* Set a new target position */
                  one_rotation = -one_rotation;

                  /* Reset increments */
                  slaves[0].inc_drive = 1;
                  slaves[1].inc_drive = 1;
                  slaves[2].inc_drive = 1;
                  delay_inc = 0;

                  /* Enable ramp calculation for new target position */
                  new_target = true;
              }
            }

            /* Read actual node sensor values */
            for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
            {
                slaves[node].act_pos = get_position_actual_ticks(slaves[node].slave_num, slv_handles);
                slaves[node].act_velocity = get_velocity_actual_rpm(slaves[node].slave_num, slv_handles);
                slaves[node].act_torque = get_torque_actual_mNm(slaves[node].slave_num, slv_handles);
                printf("DRIVE %d: pos:%7.d  vel:%6.d  tq:%6.2f    ",
                        slaves[node].slave_num, slaves[node].act_pos, slaves[node].act_velocity, slaves[node].act_torque);
            }
            printf("\r");


        }
        else if (break_loop){
            break;
        }

    }

    printf("\n");

    /* Quick stop position mode (for emergency) */
    for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
        quick_stop_position(slaves[node].slave_num, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Regain control of node to continue after quick stop */
    for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
        renable_ctrl_quick_stop(CSP, slaves[node].slave_num, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES); //after quick-stop

    for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
        set_operation_mode(CSP, slaves[node].slave_num, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
        enable_operation(slaves[node].slave_num, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Shutdown node operations */
    for (node=0; node < TOTAL_NUM_OF_SLAVES; node++)
        shutdown_operation(CSP, slaves[node].slave_num, &master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    pdo_handle_ecat(&master_setup, slv_handles, TOTAL_NUM_OF_SLAVES);

    /* Just for better printing result */
    system("setterm -cursor on");

    return 0;
}

