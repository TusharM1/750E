/** @file opcontrol.c
 * @brief File for operator control code
 *
 * This file should contain the user operatorControl() function and any functions related to it.
 *
 * Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/
 *
 * PROS contains FreeRTOS (http://www.freertos.org) whose source code may be
 * obtained from http://sourceforge.net/projects/freertos/files/ or on request.
 */

#include "main.h"

/*
 * Runs the user operator control code. This function will be started in its own task with the
 * default priority and stack size whenever the robot is enabled via the Field Management System
 * or the VEX Competition Switch in the operator control mode. If the robot is disabled or
 * communications is lost, the operator control task will be stopped by the kernel. Re-enabling
 * the robot will restart the task, not resume it from where it left off.
 *
 * If no VEX Competition Switch or Field Management system is plugged in, the VEX Cortex will
 * run the operator control task. Be warned that this will also occur if the VEX Cortex is
 * tethered directly to a computer via the USB A to A cable without any VEX Joystick attached.
 *
 * Code running in this task can take almost any action, as the VEX Joystick is available and
 * the scheduler is operational. However, proper use of delay() or taskDelayUntil() is highly
 * recommended to give other tasks (including system tasks such as updating LCDs) time to run.p
 *
 * This task should never exit; it should end with some kind of infinite loop, even if empty.
 */
#define LCD_TEST 0

static const unsigned char LF = 2;	// Front left drive wheel
static const unsigned char LB = 3;	// Back left drive wheel
static const unsigned char RF = 9;	// Front Right drive wheel
static const unsigned char RB = 8;	// Back right drive wheel

void operatorControl() {
	speakerInit();
	MenuItem *root = lcdmInit(uart1);
	lcdmAddDefaults(root);
	printf("3: %p\n\r",root);
	holoInitX4(LF, LB, RF, RB);
	while (1) {
		lcdmLoop(root);
		holoSet(joystickGetAnalog(1, 4), joystickGetAnalog(1, 3), joystickGetAnalog(1, 1));
		delay(200);
	}
}
