#ifndef _AF_FAKE_KEYBOARD_
#define _AF_FAKE_KEYBOARD_

#define AF_KEYBOARD_VENDOR 0x1234
#define AF_KEYBOARD_PRODUCT 0x5678
#define AF_KEYBOARD_NAME "Axis Fix Keyboard" // 80 chars max, I think

typedef struct keyboard keyboard;

keyboard* af_keyboard_create(int* keys, int keycount);
void af_keyboard_destroy(keyboard* kbd);
void af_keyboard_send(keyboard* kbd, int key, int press);

#endif