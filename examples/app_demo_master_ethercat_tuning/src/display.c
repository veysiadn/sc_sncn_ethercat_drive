
#include "display.h"

void wmoveclr(WINDOW *wnd, int *row)
{
    wmove(wnd, *row, 0);
    wclrtoeol(wnd);
    (*row)++;
}

int draw(WINDOW *wnd, char c, int row, int column)
{
    wmove(wnd, row,column); // curses call to move cursor to row r, column c
    wdelch(wnd); winsch(wnd, c); // curses calls to replace character under cursor by dc
    return (column+1);
}

int display_tuning(WINDOW *wnd, struct _pdo_cia402_input pdo_input, InputValues input, int row)
{
    //row 0
    wmoveclr(wnd, &row);
    //motorcontrol mode
    wprintw(wnd, "** Operation mode: ");
    switch(input.motorctrl_status) {
    case TUNING_MOTORCTRL_OFF:
        wprintw(wnd, "off");
        break;
    case TUNING_MOTORCTRL_TORQUE:
        wprintw(wnd, "Torque control %5d", input.target);
        break;
    case TUNING_MOTORCTRL_POSITION:
        wprintw(wnd, "Position control %9d", input.target);
        break;
    case TUNING_MOTORCTRL_VELOCITY:
        wprintw(wnd, "Velocity control %5d", input.target);
        break;
    }
    wprintw(wnd, " **");
    //row 1
    wmoveclr(wnd, &row);
    wprintw(wnd, "Position %14d | Velocity %4d",  pdo_input.actual_position, pdo_input.actual_velocity);
    //row 2
    wmoveclr(wnd, &row);
    struct {signed int x:16;} s; //to sign extend 16 bit torque
    s.x = pdo_input.actual_torque;
    pdo_input.actual_torque = s.x;
    wprintw(wnd, "Torque computed %4d    | Torque sensor %d", pdo_input.actual_torque, pdo_input.user_in_1);
    //row 3
    wmoveclr(wnd, &row);
    wprintw(wnd, "Offset %4d             | Pole pairs %2d", input.offset, input.pole_pairs);
    //row 4
    wmoveclr(wnd, &row);
    if (input.motor_polarity == 0)
        wprintw(wnd, "Motor polarity normal   | ");
    else
        wprintw(wnd, "Motor polarity inverted | ");
    if (input.sensor_polarity == 0)
        wprintw(wnd, "Sensor polarity normal");
    else
        wprintw(wnd, "Sensor polarity inverted");
    //row 5
    wmoveclr(wnd, &row);
    if (input.torque_control_flag == 0)
        wprintw(wnd, "Motor control off       | ");
    else
        wprintw(wnd, "Motor control on        | ");
    if (input.brake_flag == 0)
        wprintw(wnd, "Brake blocking");
    else
        wprintw(wnd, "Brake released");
    //row 6
    wmoveclr(wnd, &row);
    wprintw(wnd, "Speed  limit %5d      | ", input.max_speed);
    wprintw(wnd, "Position min %d", input.min_position);
    //row 7
    wmoveclr(wnd, &row);
    wprintw(wnd, "Torque limit %5d      | ", input.max_torque);
    wprintw(wnd, "Position max %d", input.max_position);
    //row 8
    wmoveclr(wnd, &row);
    wprintw(wnd, "Positon P %8d      | ", input.P_pos);
    wprintw(wnd, "Position I %d", input.I_pos);
    //row 9
    wmoveclr(wnd, &row);
    wprintw(wnd, "Positon D %8d      | ", input.D_pos);
    wprintw(wnd, "Position I lim %d", input.integral_limit_pos);
    return row;
}

int display_tuning_help(WINDOW *wnd, int row)
{
    //init prompt
    wmoveclr(wnd, &row);
    printw("Commands:");
    wmoveclr(wnd, &row);
    printw("b: Release/Block Brake       | a: find offset (also release the brake)");
    wmoveclr(wnd, &row);
    printw("number: set torque command   | r: reverse torque command");
    wmoveclr(wnd, &row);
    printw("ep3: enable position control | p + number: set position command");
    wmoveclr(wnd, &row);
    printw("P + number: set pole pairs");
    wmoveclr(wnd, &row);
    printw("L s/t/p + number: set speed/torque/position limit");
    wmoveclr(wnd, &row);
    printw("** Double press Enter for emergency stop **");
    return row;
}