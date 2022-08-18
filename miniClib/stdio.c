#include "stdio.h"

FILE *stdout = NULL, *stderr = NULL;

int fprintf(FILE *stream, const char *format, ...) {

}

int vsnprintf(char *s, size_t n, const char *format, va_list arg) {

}

size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream) {
	return 0;
}

int fclose(FILE *stream) {
	return 0;
}

long int ftell(FILE *stream) {
	return 0;
}

int fseek(FILE *stream, long int offset, int origin) {

}

size_t fread(void *ptr, size_t size, size_t count, FILE *stream) {
	return 0;
}
