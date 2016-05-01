#include "pch.h"

#include <Kore/Log.h>

#include <stdexcept>

namespace Kore { namespace Display {
    int count() {
        return 1;
    }

    int width( int index ) {
        log(Warning, "TODO (DK) Kore::Display::width(int) implement me");
        return -1;
    }

    int height( int index ) {
        log(Warning, "TODO (DK) Kore::Display::height(int) implement me");
        return -1;
    }

    int widthPrimary() {
        log(Warning, "TODO (DK) Kore::Display::widthPrimary() implement me");
        return -1;
    }

    int heightPrimary() {
        log(Warning, "TODO (DK) Kore::Display::heightPrimary() implement me");
        return -1;
    }
}}
