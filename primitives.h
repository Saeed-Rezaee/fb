#ifndef _PRIMITIVES_H_
#define _PRIMITIVES_H_

#include "fbdev.h"

void draw_hline(FBDev *dev, int x1, int x2, int y, uint32_t pixel);
void draw_vline(FBDev *dev, int x, int y1, int y2, uint32_t pixel);
void draw_line(FBDev *dev, int x1, int y1, int x2, int y2, uint32_t pixel);

void draw_circle(FBDev *dev, int cx, int cy, int radius, uint32_t pixel);
void draw_filled_circle(FBDev *dev, int cx, int cy, int radius, uint32_t pixel, uint32_t fill);

void draw_rect(FBDev *dev, int x, int y, int w, int h, uint32_t pixel);
void fill_rect(FBDev *dev, int x, int y, int w, int h, uint32_t pixel, uint32_t fill);
void draw_tri(FBDev *dev, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t pixel);

void draw_polygon(FBDev *dev, int *vx, int *vy, int n, uint32_t pixel);

#endif /* _PRIMITIVES_H_ */
