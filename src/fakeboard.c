#include "fakeboard.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stropts.h>
#include <sys/ioctl.h>

#include <linux/input.h>
#include <linux/uinput.h>

typedef struct keyboard keyboard;

struct keyboard {
	int fd;
};

keyboard* af_keyboard_create(int* keys, int keycount) {
	int fd, i;
	struct uinput_user_dev uidev;
	keyboard* kb;

	kb = (keyboard*)malloc(sizeof(*kb));
	if (!kb)
		return NULL;

	fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (!fd)
		return NULL;

	if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0) {
		err_printf("Error on ioctl set evbit.\n");
		close(fd);
		return NULL;
	}

	// enable each key to be sent
	for (i = 0; i < keycount; i++) {
		if (ioctl(fd, UI_SET_KEYBIT, keys[i]) < 0) {
			err_printf("Error setting key bit (key = %d).\n", keys[i]);
		}
	}

	memset(&uidev, 0, sizeof(uidev));
	strncpy(uidev.name, AF_KEYBOARD_NAME, UINPUT_MAX_NAME_SIZE);
	uidev.id.bustype = BUS_USB;
	uidev.id.vendor = AF_KEYBOARD_VENDOR;
	uidev.id.product = AF_KEYBOARD_PRODUCT;
	uidev.id.version = 1;

	if (write(fd, &uidev, sizeof(uidev)) < 0) {
		err_printf("Error setting up fake keyboard. (write error)\n");
		close(fd);
		return NULL;
	}

	if (ioctl(fd, UI_DEV_CREATE) < 0) {
		err_printf("Error creating fake keyboard. (ioctl error)\n");
		close(fd);
		return NULL;
	}

	kb->fd = fd;

	return kb;
}

static void emit(int fd, int type, int code, int val) {
	struct input_event event;

	event.type = type;
	event.code = code;
	event.value = val;
	event.time.tv_sec = 0;
	event.time.tv_usec = 0;

	if (write(fd, &event, sizeof(event)) < 0)
		err_printf("Couldn't emit event (type: %d, code: %d, val: %d)\n", type, code, val);
}

void af_keyboard_send(keyboard* kb, int key, int press) {
	if (!kb)
		return;

	emit(kb->fd, EV_KEY, key, press ? 1 : 0);
	emit(kb->fd, EV_SYN, SYN_REPORT, 0);
}

void af_keyboard_destroy(keyboard* kb) {
	if (!kb)
		return;

	if (ioctl(kb->fd, UI_DEV_DESTROY) < 0)
		err_printf("ioctl failed to destroy fake keyboard.\n");
	close(kb->fd);

	free(kb);
}

// testing purposes only
/*
int main(int argc, char** argv) {
	keyboard* kbd;
	int keys[] = { KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_A };
	int keyc = 5;

	kbd = af_keyboard_create(keys, keyc);

	sleep(5);

	af_keyboard_send(kbd, KEY_A, 1);
	af_keyboard_send(kbd, KEY_A, 0);

	af_keyboard_destroy(kbd);
	return 0;
}
*/