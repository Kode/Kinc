#pragma once

#include <Kore/Math/Matrix.h>
#include <Kore/Graphics4/VertexStructure.h>
#include <Kore/LightImpl.h>

namespace Kore {
	class Light : public LightImpl {
	public:
		Light(LightType type = DirectionalLight);

        // Sets the new type of the lighting model.
        void setType(LightType type);
        
        // Sets all light colors. Default is: ambient(0, 0, 0, 1), diffuse(1, 1, 1, 1), specular(1, 1, 1, 1).
        void setColors(const vec4& ambient, const vec4& diffuse, const vec4& specular);
        
        // Sets the position for a point- or spot-light.
        void setPosition(const vec3& position);
        
        // Sets the direction for a directional- or spot-light.
        void setDirection(const vec3& direction);
        
        // Sets the spot-light parameters directly. 'exponent' must be in the rnage [0, 180], 'cutoff' must be in the range [0, 90] or 180.
        void setSpot(float exponent, float cutoff);
        
        // Sets the attenuation parameters directly.
        void setAttenuation(float constAttn, float linearAttn, float quadricAttn);
        
        // Sets the attenuation parameters by the specified light radius. This is equivalent to 'setAttenuationRadius(1, 1 / radius, 1 / radius)'.
        void setAttenuationRadius(float radius);

        // Do not call this directly, use Graphics3::setLight
        void _set(int num);
        
	};

}
