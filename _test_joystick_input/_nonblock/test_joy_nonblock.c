/**
 * @file test_joy_nonblock.c
 *
 * Mimics the rc_project_template.c file
 * Read input from a joystick (/dev/input/js0)
 * Control LEDs on BeagleBone Blue
 * 
 */

#include <stdio.h>

#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <linux/input.h>
#include <linux/joystick.h>

#include <robotcontrol.h> // includes ALL Robot Control subsystems


/**
 * This template contains these critical components
 * - ensure no existing instances are running and make new PID file
 * - start the signal handler
 * - initialize subsystems you wish to use
 * - while loop that checks for EXITING condition
 * - cleanup subsystems at the end
 *
 * @return     0 during normal operation, -1 on error
 */
int main()
{
	// make sure another instance isn't running
	// if return value is -3 then a background process is running with
	// higher privaledges and we couldn't kill it, in which case we should
	// not continue or there may be hardware conflicts. If it returned -4
	// then there was an invalid argument that needs to be fixed.
	if(rc_kill_existing_process(2.0)<-2) return -1;

	// start signal handler so we can exit cleanly
	if(rc_enable_signal_handler()==-1){
		fprintf(stderr,"ERROR: failed to start signal handler\n");
		return -1;
	}

	// make PID file to indicate your project is running
	// due to the check made on the call to rc_kill_existing_process() above
	// we can be fairly confident there is no PID file already and we can
	// make our own safely.
	rc_make_pid_file();

	// JOYSTICK INIT
	// open device
	int fd = open("/dev/input/js0", O_NONBLOCK);
	// read number of axes
	char number_of_axes;
	ioctl(fd, JSIOCGAXES, &number_of_axes);
	// read number of buttons
	char number_of_buttons;
	ioctl(fd, JSIOCGBUTTONS, &number_of_buttons);
	// create pointers for axes and buttons
	int *axis;
	char *button;
	// allocate memory for pointers
	axis = calloc(number_of_axes, sizeof(int));
	button = calloc(number_of_buttons, sizeof(char));


	printf("\nPress and release BUTTON 0 to turn green LED on and off\n");
	printf("Press BUTTON 2 to exit\n");

	// Keep looping until state changes to EXITING
	rc_set_state(RUNNING);
	while(rc_get_state()!=EXITING){

		// READ FROM JOYSTICK
		struct js_event e;
		while(read(fd, &e, sizeof(e)) > 0)
		{
			// process event
		}
		// EAGAIN is returned when que is empty
		if(errno != EAGAIN)
		{
			// error
		}
		// determine if event is of type button or axis
		// assign value to corresponding button or axis
		switch(e.type & ~JS_EVENT_INIT)
		{
			case JS_EVENT_BUTTON:
				button[e.number] = e.value;
				break;

			case JS_EVENT_AXIS:
				axis[e.number] = e.value;
				break;
		}

		// do things based on buttons pressed
		if(button[1])
		{
			rc_led_set(RC_LED_RED, 0);
			float axis_5_val = axis[5];
			float blink_freq = (axis_5_val + 32767)/(32767*2)*10;
			rc_led_blink(RC_LED_GREEN, blink_freq, 2);
		}
		else if(button[0])
		{
			rc_led_set(RC_LED_GREEN, 1);
			rc_led_set(RC_LED_RED, 0);
		}
		else
		{
			rc_led_set(RC_LED_GREEN, 0);
			rc_led_set(RC_LED_RED, 1);
		}

		if(button[2])
		{
			rc_set_state(EXITING);
		}

		// always sleep at some point
		rc_usleep(100000);
	}

	// turn off LEDs and close file descriptors
	rc_led_set(RC_LED_GREEN, 0);
	rc_led_set(RC_LED_RED, 0);
	rc_led_cleanup();
	rc_button_cleanup();	// stop button handlers
	rc_remove_pid_file();	// remove pid file LAST
	return 0;
}
