#include "pch.h"
#include "System.h"

#ifndef SYS_HTML5
#ifndef SYS_ANDROID
double Kore::System::time() {
	return timestamp() / frequency();
}
#endif
#endif
