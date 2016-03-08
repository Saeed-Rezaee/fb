#include <linux/fb.h>
#include <linux/kd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>

#include "fbdev.h"
#include "time_utils.h"
#include "log.h"

#define FPS 30

static int set_kd_mode(int kd_mode) {
	int kd_old = -1;
	int tty_fd = -1;
	char *tty_name = ttyname(STDIN_FILENO);
	if (!isatty(STDIN_FILENO) || !tty_name) {
		// stdin is not a tty
		return -1;
	}
	if (!strstr(tty_name, "/dev/tty") && !strstr(tty_name, "/dev/vc")) {
		// stdin is not an interactive tty
		return -1;
	}

	tty_fd = open(tty_name, O_RDWR);
	if (tty_fd < 0) {
		warn("failed to open tty: %s", strerror(errno));
		return -1;
	}
	// Get current kd mode
	if (ioctl(tty_fd, KDGETMODE, &kd_old) < 0) {
		warn("failed to get current kd mode: %s", strerror(errno));
	}
	if (ioctl(tty_fd, KDSETMODE, kd_mode) < 0) {
		warn("failed to set kd mode: %s", strerror(errno));
	}
	if (close(tty_fd) < 0) {
		warn("failed to close tty: %s", strerror(errno));
	}

	return kd_old;
}

int fb_init(FBDev *dev, int flags) {
	// Open FB device
	dev->fd = open("/dev/fb0", O_RDWR);
	if (dev->fd < 0) {
		dev->fd = open("/dev/fb/0", O_RDWR);
		if (dev->fd < 0) {
			warn("unable to open fb device: %s", strerror(errno));
			return -1;
		}
	}

	// Set variable screen information
	if (ioctl(dev->fd, FBIOGET_VSCREENINFO, &(dev->vinfo)) < 0) {
		warn("failed to get variable screen info: %s", strerror(errno));
	}

	dev->vinfo.grayscale = 0; // Disable grayscale

	// Set alpha offset and length if depth is sufficient
	uint32_t rgb_length = dev->vinfo.red.length + dev->vinfo.green.length + dev->vinfo.blue.length;
	if (rgb_length < dev->vinfo.bits_per_pixel) {
		dev->vinfo.transp.offset = rgb_length;
		dev->vinfo.transp.length = dev->vinfo.bits_per_pixel - rgb_length;
	}

	if (ioctl(dev->fd, FBIOPUT_VSCREENINFO, &(dev->vinfo)) < 0) {
		warn("failed to set variable screen info: %s", strerror(errno));
	}

	// Get variable and fixed screen information
	if (ioctl(dev->fd, FBIOGET_VSCREENINFO, &(dev->vinfo)) < 0) {
		warn("failed to get variable screen info: %s", strerror(errno));
	}
	if (ioctl(dev->fd, FBIOGET_FSCREENINFO, &(dev->finfo)) < 0) {
		warn("failed to get fixed screen info: %s", strerror(errno));
	}

	dev->ssize = dev->vinfo.yres_virtual * dev->finfo.line_length;

	if (flags & DBL_BUF_NONE) {
		// Do not use double buffering
		dev->fbuf = (uint8_t *) mmap(0, dev->ssize, PROT_READ | PROT_WRITE, MAP_SHARED, dev->fd, (off_t)0);
		if (dev->fbuf == MAP_FAILED) {
			warn("failed to map front buffer: %s", strerror(errno));
			return -1;
		}

		dev->bbuf = dev->bufp = dev->fbuf;
	} else if (flags & DBL_BUF_MEM) {
		// Use a separate back buffer for double buffering
		dev->fbuf = (uint8_t *) mmap(0, dev->ssize, PROT_READ | PROT_WRITE, MAP_SHARED, dev->fd, (off_t)0);
		if (dev->fbuf == MAP_FAILED) {
			warn("failed to map front buffer: %s", strerror(errno));
			return -1;
		}

		dev->bbuf = (uint8_t *) malloc(dev->ssize);
		if (dev->bbuf == NULL) {
			warn("failed to allocate back buffer: %s", strerror(errno));
			return -1;
		}

		dev->bufp = dev->bbuf;
	} else if (flags & DBL_BUF_PAN) {
		// Allocate both buffers at once and then "pan" between them
		dev->fbuf = (uint8_t *) mmap(0, dev->ssize*2, PROT_READ | PROT_WRITE, MAP_SHARED, dev->fd, (off_t)0);
		if (dev->fbuf == MAP_FAILED) {
			warn("failed to map double buffer: %s", strerror(errno));
			return -1;
		}

		dev->bbuf = dev->fbuf + dev->ssize;

		dev->bufp = dev->bbuf;
	}

	if (flags & SET_KD_MODE) {
		dev->kdsave = set_kd_mode(KD_GRAPHICS);
	} else {
		// Indicate that the kd mode should not be reset on exit
		dev->kdsave = -1;
	}

	dev->flags = flags;

	dev->blend = BLEND_NONE;

	// Masks for extracting specific bitfields from any pixel
	dev->rmask = ((1 << dev->vinfo.red.length) - 1) << dev->vinfo.red.offset;
	dev->gmask = ((1 << dev->vinfo.green.length) - 1) << dev->vinfo.green.offset;
	dev->bmask = ((1 << dev->vinfo.blue.length) - 1) << dev->vinfo.blue.offset;
	dev->amask = ((1 << dev->vinfo.transp.length) - 1) << dev->vinfo.transp.offset;

	// Commonly used colors
	dev->black = fb_rgb(dev, 0x00, 0x00, 0x00);
	dev->white = fb_rgb(dev, 0xFF, 0xFF, 0xFF);

	return 0;
}

void fb_size(FBDev *dev, int *w, int *h) {
	*w = dev->vinfo.xres;
	*h = dev->vinfo.yres;
}

inline uint32_t fb_rgb(FBDev *dev, uint8_t r, uint8_t g, uint8_t b) {
	return (r<<dev->vinfo.red.offset) | (g<<dev->vinfo.green.offset) | (b<<dev->vinfo.blue.offset);
}

inline uint32_t fb_rgba(FBDev *dev, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	return (r<<dev->vinfo.red.offset) | (g<<dev->vinfo.green.offset) | (b<<dev->vinfo.blue.offset) | (a<<dev->vinfo.transp.offset);
}

void fb_blend(FBDev *dev, int blend) {
	dev->blend = blend;
}

void fb_draw(FBDev *dev, int x, int y, uint32_t src) {
#define RED(v)		((uint8_t)((v & dev->rmask) >> dev->vinfo.red.offset))
#define GREEN(v)	((uint8_t)((v & dev->gmask) >> dev->vinfo.green.offset))
#define BLUE(v)		((uint8_t)((v & dev->bmask) >> dev->vinfo.blue.offset))
#define ALPHA(v)	((uint8_t)((v & dev->amask) >> dev->vinfo.transp.offset))

	long location = (x+dev->vinfo.xoffset) * (dev->vinfo.bits_per_pixel/8)
					+ (y+dev->vinfo.yoffset) * dev->finfo.line_length;
	if (location > dev->ssize) {
		return;
	}
	switch (dev->blend) {
		default:
		case BLEND_NONE:
			*((uint32_t*)(dev->bufp + location)) = src;
			break;
		case BLEND_ALPHA:
			*((uint32_t*)(dev->bufp + location)) = src;
			break;
		case BLEND_ADD:
			*((uint32_t*)(dev->bufp + location)) = src;
			break;
	}
}

void fb_clear(FBDev *dev, uint32_t pixel) {
//	unsigned int x,y;
//	for (x = 0; x < dev->vinfo.xres; x++) {
//		for (y = 0; y < dev->vinfo.yres; y++) {
//			fb_draw(dev, x, y, pixel);
//		}
//	}

	memset(dev->bbuf, pixel, dev->ssize);
}

void fb_swap(FBDev *dev) {
	// Swap buffers
	if (dev->flags & DBL_BUF_NONE) {
		// Double buffering is not being used
		return;
	} else if (dev->flags & DBL_BUF_MEM) {
		// Copy the back buffer to the front buffer
		memcpy(dev->fbuf, dev->bbuf, dev->ssize);
	} else if (dev->flags & DBL_BUF_PAN) {
		// "Pan" to the back buffer
		if (dev->vinfo.yoffset == 0) {
			dev->vinfo.yoffset = dev->ssize;
		} else {
			dev->vinfo.yoffset = 0;
		}

		if (ioctl(dev->fd, FBIOPAN_DISPLAY, &(dev->vinfo)) < 0) {
			warn("failed to pan display: %s", strerror(errno));
		}

		// Update buffer pointers
		uint8_t *tmp;
		tmp = dev->fbuf;
		dev->fbuf = dev->bbuf;
		dev->bbuf = tmp;
		dev->bufp = dev->bbuf;
	}
}

void fb_close(FBDev *dev) {
	if (dev->flags & DBL_BUF_NONE) {
		// Unmap front buffer
		if (munmap(dev->fbuf, dev->ssize) < 0) {
			warn("failed to unmap front buffer: %s", strerror(errno));
		}
	} else if (dev->flags & DBL_BUF_MEM) {
		// Unmap front buffer and free back buffer
		if (munmap(dev->fbuf, dev->ssize) < 0) {
			warn("failed to unmap front buffer: %s", strerror(errno));
		}

		free(dev->bbuf);
	} else if (dev->flags & DBL_BUF_PAN) {
		// Unmap double buffer
		if (munmap(dev->fbuf, dev->ssize*2) < 0) {
			warn("failed to unmap double buffer: %s", strerror(errno));
		}
	}

	if (dev->kdsave >= 0) {
		// Reset kd mode
		set_kd_mode(dev->kdsave);
	}

	if (close(dev->fd) < 0) {
		warn("failed to close fb device: %s", strerror(errno));
	}
}
