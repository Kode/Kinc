#include "pch.h"
#include "Error.h"
#include <stdlib.h>

using namespace Kore;

void Kore::affirm(bool b) {
	if (!b) error();
}

void Kore::affirm(bool b, const char* message) {
	if (!b) error(message);
}

void Kore::error() {
	error("Unknown error");
}

void Kore::error(const char* message) {
	exit(EXIT_FAILURE);
}
