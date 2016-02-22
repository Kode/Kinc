#include "pch.h"

#include <stdexcept>

namespace Kore { namespace Display {
    int count() {
        return 1;
    }

    int width( int index ) {
        throw std::runtime_error("TODO (DK) implement me");
    }

    int height( int index ) {
        throw std::runtime_error("TODO (DK) implement me");
    }
}}