#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

#include "fbdev.h"
#include "primitives.h"
#include "time_utils.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define FPS 30

int main() {
	FBDev dev;

	if (fb_init(&dev, DBL_BUF_MEM | SET_KD_MODE) < 0) {
		exit(EXIT_FAILURE);
	}

	int frame_count;
//	unsigned int start_ms, base_ms, current_ms, previous_ms;
	unsigned int start_ms, base_ms, current_ms;

	float frame_rate = (1000.0 / (float) FPS);
	start_ms = base_ms = current_ms = get_current_ms();
	frame_count = 0;

	int w, h;
	float a = 0, r;
	int x1, y1, x2, y2;

	fb_size(&dev, &w, &h);
	x1 = w/2; y1 = h/2;
	r = MIN(w/2, h/2);

	uint32_t red = fb_rgb(&dev, 0xFF, 0x00, 0x00);

	while ((current_ms - start_ms) < 5000) {
//		unsigned int delta_ms = get_current_ms() - previous_ms;

		// Clear screen
		fb_clear(&dev, dev.black);

		x2 = x1 + r * cos(a);
		y2 = y1 + r * sin(a);
		a = (a + 0.1);

		draw_filled_circle(&dev, x1, y1, r, red, dev.white);
		draw_line(&dev, x1, y1, x2, y2, dev.black);

		fb_swap(&dev);

		frame_count++;

		// Calculate delay and sleep if necessary
		current_ms = get_current_ms();
//		previous_ms = current_ms;
		uint target_ms = base_ms + (uint)((float) frame_count * frame_rate);

		if (current_ms <= target_ms) {
			sleep_ms(target_ms - current_ms);
		} else {
			frame_count = 0;
			base_ms = get_current_ms();
		}
	}

	fb_close(&dev);

	return 0;
}
