#include "pch.h"
#include "System.h"

#ifndef SYS_HTML5
double Kore::System::time() {
	return timestamp() / frequency();
}
#endif
