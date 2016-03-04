#ifndef _FBDEV_H_
#define _FBDEV_H_

#include <linux/fb.h>
#include <stdint.h> /* uint8_t, uint32_t, ... */

typedef struct fbdev_t FBDev;

struct fbdev_t {
	int fd;
	struct fb_fix_screeninfo finfo;
	struct fb_var_screeninfo vinfo;
	int flags;
	int kdsave;
	long ssize;
	uint8_t *fbuf;
	uint8_t *bbuf;
	uint8_t *bufp;
	uint32_t black, white;
};

enum {
	DBL_BUF_NONE	= 1 << 0,
	DBL_BUF_MEM		= 1 << 1,
	DBL_BUF_PAN		= 1 << 2,

	SET_KD_MODE		= 1 << 3,
};

int fb_init(FBDev *dev, int flags);
void fb_size(FBDev *dev, int *w, int *h);
uint32_t fb_rgb(FBDev *dev, uint8_t r, uint8_t g, uint8_t b);
void fb_draw(FBDev *dev, int x, int y, uint32_t pixel);
void fb_clear(FBDev *dev, uint32_t pixel);
void fb_swap(FBDev *dev);
void fb_close(FBDev *dev);

#endif /* _FBDEV_H_ */
