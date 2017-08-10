#include "configurator.h"
#include "ini.h"
#include "debug.h"
#include "pad.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <linux/joystick.h>
#include <linux/uinput.h>

#define MAX_JOYSTICKS 32

int config_pad(struct ddrpad*);
int export_pad(FILE*, struct ddrpad*, int);
void print_available_js(void);
int are_there_joysticks(void);
void get_device_event(int js_fd, int* value, int* number, int* event_type);

void af_start_config(char* outname) {
	FILE* output;
	struct ddrpad current_pad;
	int no_config = 0, padc = 0, i;

	if (!are_there_joysticks()) {
		err_printf("There are no joysticks to be configured!");
		return;
	}

	output = fopen(outname, "w+");
	if (!output) {
		err_printf("Error opening '%s' for writing.\n", outname);
		return;
	}

	printf("How many pads do you want to configure? ");
	scanf("%d", &padc);

	fprintf(output, "; DO NOT forget to change the value of 'pads' when adding pads manually\n");
	fprintf(output, "; you can find key code constants here:\n");
	fprintf(output, "; https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h\n");
	fprintf(output, "[general]\npads = %d\n\n", padc);

	for (i = 0; i < padc; i++) {
		config_pad(&current_pad);
		export_pad(output, &current_pad, i);

		free(current_pad.device); // free device string
	}

	fclose(output);
	dbg_printf("File '%s' closed.\n", outname);
}

// testing purposes only
/*
int main(int argc, char** argv) {
	af_start_config("batata.cfg");
	return 0;
}
*/

char* make_js_path(int num) {
	char buffer[50];
	snprintf(buffer, sizeof(buffer) - 1, "/dev/input/js%d", num);
	return strdup(buffer);
}

int config_pad(struct ddrpad* pad) {
	size_t strsize = 0;
	int js_fd, ev_num, ev_val, ev_type;
	int device_num, preset_num;

	pad->device = NULL;

	do {
		if (pad->device != NULL) {
			err_printf("Device '%s' could not be opened.\n", pad->device);
			free(pad->device);
		}

		
		print_available_js();
		printf("Number of device to use: ");
		scanf("%d", &device_num);

		pad->device = make_js_path(device_num);

		dbg_printf("Opening device '%s'...\n", pad->device);
	} while ((js_fd = open(pad->device, O_RDONLY)) < 0);

	printf("Using joystick '%s'.\n", pad->device);

	// Calibrate UP value
	printf("> press (only) UP on your pad and hold\n");
	ev_type = JS_EVENT_AXIS;
	get_device_event(js_fd, &ev_val, &ev_num, &ev_type);
	pad->vert_axis = ev_num;
	pad->vert_up = ev_val;
	dbg_printf("vertical (%d) axis up is %d\n", ev_num, ev_val);

	// Calibrate both up and down
	printf("> press DOWN (without releasing UP)\n");
	ev_type = JS_EVENT_AXIS;
	get_device_event(js_fd, &ev_val, &ev_num, &ev_type);
	pad->vert_mid = ev_val;
	dbg_printf("vertical axis mid value is %d\n", ev_val);

	// Calibrate down
	printf("> release UP (but keep pressing DOWN)\n");
	ev_type = JS_EVENT_AXIS;
	get_device_event(js_fd, &ev_val, &ev_num, &ev_type);
	pad->vert_down = ev_val;
	dbg_printf("vertical axis down is %d\n", ev_val);

	printf("> release DOWN.\n");
	get_device_event(js_fd, &ev_val, &ev_num, &ev_type);
	pad->vert_idle = ev_val;
	dbg_printf("vertical axis idle is %d\n", ev_val);

	// Calibrate left
	printf("> press (only) LEFT on your pad and hold\n");
	get_device_event(js_fd, &ev_val, &ev_num, &ev_type);
	pad->hor_left = ev_val;
	pad->hor_axis = ev_num;
	dbg_printf("horizontal (%d) axis left is %d\n", ev_num, ev_val);

	// Calibrate both
	printf("> press RIGHT (without releasing LEFT)\n");
	get_device_event(js_fd, &ev_val, &ev_num, &ev_type);
	pad->hor_mid = ev_val;
	dbg_printf("horizontal axis mid value is %d\n", ev_val);

	// Calibrate just right
	printf("> release LEFT (but keep pressing RIGHT)\n");
	get_device_event(js_fd, &ev_val, &ev_num, &ev_type);
	pad->hor_right = ev_val;
	dbg_printf("horizontal axis right is %d\n", ev_val);

	// calibrate zero
	printf("> release RIGHT.\n");
	get_device_event(js_fd, &ev_val, &ev_num, &ev_type);
	pad->hor_idle = ev_val;
	dbg_printf("horizontal axis idle is %d\n", ev_val);

	// key preset
	printf("Available key presets (can be edited on config file):\n");
	printf("\t(0) WASD - (up, left, down, right)\n");
	printf("\t(1) keyboard arrows\n");
	printf("\t(2) IJKL - (up, left, down, right)\n");
	printf("Which preset do you want to use? ");
	scanf("%d", &preset_num);

	switch(preset_num) {
	case 1:
		pad->up_key = KEY_UP; // up key uinput code
		pad->down_key = KEY_DOWN; // down key
		pad->left_key = KEY_LEFT; // left key
		pad->right_key = KEY_RIGHT; // right key
		break;
	case 2:
		pad->up_key = KEY_I; // i key
		pad->down_key = KEY_K; // k key
		pad->left_key = KEY_J; // j key
		pad->right_key = KEY_L; // l key
		break;	
	default:
		printf("Invalid preset, picking 0 by default\n");
	case 0:
		pad->up_key = KEY_W; // w key
		pad->down_key = KEY_S; // s key
		pad->left_key = KEY_A; // a key
		pad->right_key = KEY_D; // d key
		break;
	}

	printf("Done configuring pad '%s'.\n", pad->device);

	close(js_fd);
	dbg_printf("joystick closed.\n");
}

void get_device_event(int js_fd, int* value, int* number, int* event_type) {
	struct js_event event;

	do {
		read(js_fd, &event, sizeof(event));
	} while (event.type == JS_EVENT_INIT || (*event_type != 0 && event.type != * event_type));

	*value = event.value;
	*number = event.number;
	*event_type = event.type;
}

void print_available_js(void) {
	int i, fd, has_devices = 0;
	char buffer[50];

	printf("joystick devices ");

	for (i = 0; i < MAX_JOYSTICKS; i++) {
		snprintf(buffer, sizeof(buffer) - 1, "/dev/input/js%d", i);
		if ((fd = open(buffer, O_RDONLY)) > 0) {
			printf("(%d) '%s', ", i, buffer);
			close(fd);
		}
	}

	printf("\b\b are available.\n");
}

int are_there_joysticks(void) {
	int i, fd, has_devices = 0;
	char buffer[50];

	for (i = 0; i < MAX_JOYSTICKS; i++) {
		snprintf(buffer, sizeof(buffer) - 1, "/dev/input/js%d", i);
		if ((fd = open(buffer, O_RDONLY)) > 0) {
			has_devices++;
			close(fd);
		}
	}

	return has_devices;
}

int export_pad(FILE* file, struct ddrpad* pad, int padnumber) {
	fprintf(file, "[pad %d]\n");
	fprintf(file, "device = %s\n\n", pad->device);

	fprintf(file, "vert_axis = %d\n", pad->vert_axis);
	fprintf(file, "vert_up = %d\n", pad->vert_up);
	fprintf(file, "vert_down = %d\n", pad->vert_down);
	fprintf(file, "vert_middle = %d\n", pad->vert_mid);
	fprintf(file, "vert_idle = %d\n\n", pad->vert_idle);

	fprintf(file, "hor_axis = %d\n", pad->hor_axis);
	fprintf(file, "hor_right = %d\n", pad->hor_right);
	fprintf(file, "hor_left = %d\n", pad->hor_left);
	fprintf(file, "hor_middle = %d\n", pad->hor_mid);
	fprintf(file, "hor_idle = %d\n\n", pad->hor_idle);

	fprintf(file, "up_key = 0x%X\n", pad->up_key);
	fprintf(file, "down_key = 0x%X\n", pad->down_key);
	fprintf(file, "left_key = 0x%X\n", pad->left_key);
	fprintf(file, "right_key = 0x%X\n\n", pad->right_key);
}
