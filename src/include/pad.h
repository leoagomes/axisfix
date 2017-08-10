#ifndef _AF_PAD_
#define _AF_PAD_

#define PAD_SET_RIGHT(p) do{(p)->current_state.right=1;(p)->current_state.left=0;}while(0)
#define PAD_SET_LEFT(p) do{(p)->current_state.right=0;(p)->current_state.left=1;}while(0)
#define PAD_SET_LEFTRIGHT(p) do{(p)->current_state.right=1;(p)->current_state.left=1;}while(0)

#define PAD_SET_UP(p) do{(p)->current_state.up=1;(p)->current_state.down=0;}while(0)
#define PAD_SET_DOWN(p) do{(p)->current_state.up=0;(p)->current_state.down=1;}while(0)
#define PAD_SET_UPDOWN(p) do{(p)->current_state.up=1;(p)->current_state.down=1;}while(0)

#define PAD_CMP_UP(p) ((p)->current_state.up == (p)->last_state.up)
#define PAD_CMP_DOWN(p) ((p)->current_state.down == (p)->last_state.down)
#define PAD_CMP_LEFT(p) ((p)->current_state.left == (p)->last_state.left)
#define PAD_CMP_RIGHT(p) ((p)->current_state.right == (p)->last_state.right)

struct direction_buffer {
	int up, down, left, right;
};

struct ddrpad {
	char update;
	int js_fd;

	char* device;

	struct direction_buffer current_state, last_state;

	int vert_up, vert_down, vert_mid, vert_idle;
	int hor_right, hor_left, hor_mid, hor_idle;

	unsigned char vert_axis, hor_axis;

	int up_key, down_key, right_key, left_key;
};

#endif
