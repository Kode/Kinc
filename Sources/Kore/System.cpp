#include "pch.h"
#include "System.h"

#ifndef SYS_HTML5
double Kore::System::getTime() {
	return static_cast<double>(getTimestamp() / getFrequency());
}
#endif

