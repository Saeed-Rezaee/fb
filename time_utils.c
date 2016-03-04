#include <time.h>
#include <math.h>

unsigned int get_current_ms() {
	struct timespec tspec;
	clock_gettime(CLOCK_MONOTONIC, &tspec);
	return (tspec.tv_sec * 1000) + round(tspec.tv_nsec / 1000000); // Convert seconds and nanoseconds to milliseconds
}

void sleep_ms(unsigned int ms) {
	struct timespec tspec;
	tspec.tv_sec = (unsigned int)(ms / 1000); // Extract seconds
	tspec.tv_nsec = (long)((ms % 1000) * 1000000); // Extract sub-second portion
	nanosleep(&tspec, NULL);
}

void sleep_us(unsigned int us) {
	struct timespec tspec;
	tspec.tv_sec = (unsigned int)(us / 1000000); // Extract seconds
	tspec.tv_nsec = (long)((us % 1000000) * 1000); // Extract sub-second portion
	nanosleep(&tspec, NULL);
}

void sleep_ns(unsigned int ns) {
	struct timespec tspec;
	tspec.tv_sec = (int)(ns / 1000000000); // Extract seconds
	tspec.tv_nsec = (long)(ns % 1000000000); // Extract sub-second portion
	nanosleep(&tspec, NULL);
}
