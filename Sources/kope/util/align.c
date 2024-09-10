#include "align.h"

int align_pow2(int value, int pow2_alignment) {
	int mask = pow2_alignment - 1;
	return value + (-value & mask);
}
