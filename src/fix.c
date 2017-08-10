#include "debug.h"
#include "pad.h"
#include "fix.h"
#include "padreader.h"
#include "fakeboard.h"

#include "ini.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/joystick.h>

int open_pads(struct ddrpad* pads, int padc);
void close_pads(struct ddrpad* pads, int padc);
void release_pads(struct ddrpad* pads, int padc);
struct pollfd* get_poll_fds(struct ddrpad* pads, int padc, int* fdc);
void fix_loop(struct ddrpad* pads, int padc);
keyboard* create_keyboard_for_pads(struct ddrpad* pads, int padc);
void fix_loop(struct ddrpad* pads, int padc);
void process_pad_state(struct ddrpad* pad, keyboard* kbd);

// entry point
void af_start_fix(char* inname) {
	FILE* input;
	struct ddrpad* pads;
	int padc;

	// open config file
	input = fopen(inname, "r");
	if (!input) {
		err_printf("Error opening '%s' for reading.\n", input);
		return;
	}

	// read pad data
	pads = read_pad_data(input, &padc);
	if (!pads) {
		fclose(input);
		return;
	}
	dbg_printf("Read pad data.\n");

	// close config file as not necessary anymore
	fclose(input);

	// open the joystick for the pads
	if (open_pads(pads, padc) == 0) {
		release_pads(pads, padc);
		return;
	}

	// run the fix loop
	fix_loop(pads, padc);

	// close and release the pads
	close_pads(pads, padc);
	release_pads(pads, padc);
}

int open_pads(struct ddrpad* pads, int padc) {
	int i, j;

	for (i = 0; i < padc; i++) {
		if ((pads[i].js_fd = open(pads[i].device, O_RDONLY)) < 0) {
			err_printf("Error opening pad %d, (device: %s).\n", i, pads[i].device);

			for (j = i - 1; j >= 0; j--) {
				dbg_printf("closing pad %s.\n", pads[j].device);
				close(pads[j].js_fd);
			}

			return 0;
		}
	}

	return 1;
}

void close_pads(struct ddrpad* pads, int padc) {
	int i;

	for (i = 0; i < padc; i++) {
		dbg_printf("closing pad %s.\n", pads[i].device);
		close(pads[i].js_fd);
	}
}

void release_pads(struct ddrpad* pads, int padc) {
	int i;

	for (i = 0; i < padc; i++) {
		dbg_printf("releasing pad %d (device: %s).\n", i, pads[i].device);
		free(pads[i].device);
	}

	free(pads);
}

struct pollfd* get_poll_fds(struct ddrpad* pads, int padc, int* fdc) {
	struct pollfd* fds;
	int i;

	*fdc = padc + 1;
	fds = (struct pollfd*)malloc(sizeof(*fds) * (*fdc));

	// add each pad to devices to poll
	for (i = 0; i < padc; i++) {
		fds[i].fd = pads[i].js_fd;
		fds[i].events = POLLIN;
	}

	// add stdin to devices to poll
	fds[i].fd = STDIN_FILENO;
	fds[i].events = POLLIN;

	return fds;
}

keyboard* create_keyboard_for_pads(struct ddrpad* pads, int padc) {
	int* keys;
	int keyc, i;
	keyboard* kbd;

	keyc = padc * 4;
	keys = (int*)malloc(sizeof(int) * keyc);

	for (i = 0; i < padc; i++) {
		keys[(i*4)] = pads[i].up_key;
		keys[(i*4) + 1] = pads[i].down_key;
		keys[(i*4) + 2] = pads[i].left_key;
		keys[(i*4) + 3] = pads[i].right_key;
	}

	kbd = af_keyboard_create(keys, keyc);

	free(keys);

	return kbd;
}

void fix_loop(struct ddrpad* pads, int padc) {
	struct pollfd* fds;
	int fdc, pollret, i;
	keyboard* kbd;

	fds = get_poll_fds(pads, padc, &fdc);

	dbg_printf("Creating fake keyboard.\n");

	kbd = create_keyboard_for_pads(pads, padc);

	printf("Entering fix mode, press anything to quit the program.\n");

	while (1) {
		pollret = poll(fds, fdc, -1);

		// poll returned either -1 or 0, which are both errors in this case
		if (pollret <= 0) {
			err_printf("poll returned %d.\n", pollret);
			break;
		}

		// check the devices available for reading.
		for (i = 0; i < padc; i++) {
			if (fds[i].revents & POLLIN == 0)
				break;

			process_pad_state(&(pads[i]), kbd);
		}

		// check stdin
		if (fds[i].revents & POLLIN) {
			printf("Terminating fix mode.\n");
			break;
		}
	}

	// release fds
	free(fds);
}

void process_pad_state(struct ddrpad* pad, keyboard* kbd) {
	struct js_event ev;
	ssize_t readret;
	int val, axis;

	if ((readret = read(pad->js_fd, &ev, sizeof(ev))) <= 0) {
		err_printf("error reading pad '%s' event. (read returned %d)\n", pad->device, readret);
		return;
	}

	if (ev.type & JS_EVENT_AXIS == 0)
		return;

	axis = ev.number;
	val = ev.value;

	// process current pad state
	if (axis == pad->hor_axis) {
		if (val == pad->hor_left)
			PAD_SET_LEFT(pad);
		else if (val == pad->hor_right)
			PAD_SET_RIGHT(pad);
		else if (val == pad->hor_mid)
			PAD_SET_LEFTRIGHT(pad);
		else if (val == pad->hor_idle)
			PAD_CLEAR_HORIZONTAL(pad);
		else
			dbg_printf("unknown horizontal axis value: %d\n", val);
	} else if (axis == pad->vert_axis) {
		if (val == pad->vert_up)
			PAD_SET_UP(pad);
		else if (val == pad->vert_down)
			PAD_SET_DOWN(pad);
		else if (val == pad->vert_mid)
			PAD_SET_UPDOWN(pad);
		else if (val == pad->vert_idle)
			PAD_CLEAR_VERTICAL(pad);
		else
			dbg_printf("unknown vertical axis value: %d\n", val);
	} else {
		dbg_printf("Unsupported axis on pad %s pressed. (axis: %d, value: %d)\n", pad->device, axis, val);
		return;
	}

	// process pad state change
	if (PAD_CMP_DOWN(pad))
		af_keyboard_send(kbd, pad->down_key, pad->current_state.down);
	if (PAD_CMP_UP(pad))
		af_keyboard_send(kbd, pad->up_key, pad->current_state.up);
	if (PAD_CMP_LEFT(pad))
		af_keyboard_send(kbd, pad->left_key, pad->current_state.left);
	if (PAD_CMP_RIGHT(pad))
		af_keyboard_send(kbd, pad->right_key, pad->current_state.right);

	// copy current state to last state
	memcpy(&(pad->last_state), &(pad->current_state), sizeof(struct direction_buffer));
}

// testing purposes only
/*
int main(int argc, char** argv) {
    af_start_fix("batata.cfg");
	return 0;
}
*/
