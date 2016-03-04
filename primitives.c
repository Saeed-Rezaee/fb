#include <stdlib.h> /* abs */

#include "primitives.h"

void draw_hline(FBDev *dev, int x1, int x2, int y, uint32_t pixel) {
	int x;
	if (x1 < x2) {
		for (x = x1; x <= x2; x++) {
			fb_draw(dev, x, y, pixel);
		}
	} else {
		for (x = x2; x <= x1; x++) {
			fb_draw(dev, x, y, pixel);
		}
	}
}

void draw_vline(FBDev *dev, int x, int y1, int y2, uint32_t pixel) {
	int y;
	if (y1 < y2) {
		for (y = y1; y <= y2; y++) {
			fb_draw(dev, x, y, pixel);
		}
	} else {
		for (y = y2; y <= y1; y++) {
			fb_draw(dev, x, y, pixel);
		}
	}
}

void draw_line(FBDev *dev, int x1, int y1, int x2, int y2, uint32_t pixel) {
	int i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py;

	// Handle simple cases first
	if (x1 == x2) {
		draw_vline(dev, x1, y1, y2, pixel);
	} else if (y1 == y2) {
		draw_hline(dev, x1, x2, y1, pixel);
	}

	dx = x2 - x1;				// Delta x
	dy = y2 - y1;				// Delta y
	dxabs = abs(dx);			// Absolute delta
	dyabs = abs(dy);			// Absolute delta
	sdx = (dx > 0) ? 1 : -1;	// signum function
	sdy = (dy > 0) ? 1 : -1;	// signum function
	x = dyabs >> 1;
	y = dxabs >> 1;
	px = x1;
	py = y1;

	if (dxabs >= dyabs) {
		// Draw along x
		for (i = 0; i < dxabs; i++) {
			y += dyabs;
			if (y >= dxabs) {
				y -= dxabs;
				py += sdy;
			}
			px += sdx;

			fb_draw(dev, px, py, pixel);
		}
	} else {
		// Draw along y
		for (i = 0; i < dyabs; i++) {
			x += dxabs;
			if (x >= dyabs) {
				x -= dyabs;
				px += sdx;
			}
			py += sdy;

			fb_draw(dev, px, py, pixel);
		}
	}
}

void draw_circle(FBDev *dev, int cx, int cy, int radius, uint32_t pixel) {
	int error = -radius;
	int x = radius;
	int y = 0;

	while (x >= y) {
		fb_draw(dev, cx + x, cy + y, pixel);
		fb_draw(dev, cx - x, cy + y, pixel);
		fb_draw(dev, cx + x, cy - y, pixel);
		fb_draw(dev, cx - x, cy - y, pixel);

		fb_draw(dev, cx + y, cy + x, pixel);
		fb_draw(dev, cx - y, cy + x, pixel);
		fb_draw(dev, cx + y, cy - x, pixel);
		fb_draw(dev, cx - y, cy - x, pixel);

		error += y;
		y++;
		error += y;

		if (error >= 0) {
			error += -x;
			x--;
			error += -x;
		}
	}
}

void draw_filled_circle(FBDev *dev, int cx, int cy, int radius, uint32_t pixel, uint32_t fill) {
	int error = -radius;
	int x = radius;
	int y = 0;

	while (x >= y) {
		draw_hline(dev, cx + x, cx - x, cy + y, fill);
		draw_hline(dev, cx + x, cx - x, cy - y, fill);
		draw_hline(dev, cx + y, cx - y, cy + x, fill);
		draw_hline(dev, cx + y, cx - y, cy - x, fill);

		fb_draw(dev, cx + x, cy + y, pixel);
		fb_draw(dev, cx - x, cy + y, pixel);
		fb_draw(dev, cx + x, cy - y, pixel);
		fb_draw(dev, cx - x, cy - y, pixel);
		fb_draw(dev, cx + y, cy + x, pixel);
		fb_draw(dev, cx - y, cy + x, pixel);
		fb_draw(dev, cx + y, cy - x, pixel);
		fb_draw(dev, cx - y, cy - x, pixel);

		error += y;
		y++;
		error += y;

		if (error >= 0) {
			error += -x;
			x--;
			error += -x;
		}
	}
}

void draw_rect(FBDev *dev, int x, int y, int w, int h, uint32_t pixel) {
	draw_hline(dev, x, x+w, y, pixel);
	draw_hline(dev, x, x+w, y+h, pixel);
	draw_vline(dev, x, y, y+h, pixel);
	draw_vline(dev, x+w, y, y+h, pixel);
}

void fill_rect(FBDev *dev, int x, int y, int w, int h, uint32_t pixel, uint32_t fill) {
	int i;
	for (i = y; i < y+h; i++) {
		draw_hline(dev, x, x+w, i, fill);
	}
	draw_rect(dev, x, y, w, h, pixel);
}

void draw_tri(FBDev *dev, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t pixel) {
	draw_line(dev, x1, y1, x2, y2, pixel);
	draw_line(dev, x2, y2, x3, y3, pixel);
	draw_line(dev, x3, y3, x1, y1, pixel);
}

void draw_polygon(FBDev *dev, int *vx, int *vy, int n, uint32_t pixel) {
	int i;
	for (i = 0; i < n; i++) {
		draw_line(dev, vx[i], vx[(i+1)%n], vy[i], vy[(i+1)%n], pixel);
	}
}
