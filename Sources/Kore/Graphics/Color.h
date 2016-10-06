#pragma once

namespace Kore {
    class Color {
        
    private:
        void getColorFromHex(uint color, float& red, float& green, float& blue, float& alpha);
        
    public:
        Color(uint color);
        
        float R;
        float G;
        float B;
        float A;
    };
}
