#include "stdio.h"

FILE *stdout = NULL, *stderr = NULL;

int fprintf(FILE *stream, const char *format, ...) {
	return 0;
}

int vsnprintf(char *s, size_t n, const char *format, va_list arg) {
	return 0;
}

size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream) {
	return 0;
}

FILE *fopen(const char *filename, const char *mode) {
	return NULL;
}

int fclose(FILE *stream) {
	return 0;
}

long int ftell(FILE *stream) {
	return 0;
}

int fseek(FILE *stream, long int offset, int origin) {
	return 0;
}

size_t fread(void *ptr, size_t size, size_t count, FILE *stream) {
	return 0;
}

int fputs(const char *str, FILE *stream) {
	return 0;
}
