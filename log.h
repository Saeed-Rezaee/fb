#ifndef _LOG_H_
#define _LOG_H_

#include <string.h>
#include <errno.h>

void warn(const char *fmt, ...);

#define WARN(msg, ...) \
	warn(msg ": %s", ##__VA_ARGS__, strerror(errno))

#define DEBUG(msg, ...) \
	warn("DEBUG %s:%s:%d: " msg, \
			__FILE__, __func__, __LINE__, ##__VA_ARGS__)

#endif /* _LOG_H_ */
