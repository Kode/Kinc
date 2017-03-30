#pragma once

#include "ogl.h"
#include <Kore/Math/Vector.h>

namespace Kore {
    enum LightType {
        DirectionalLight, PointLight, SpotLight
    };

	class LightImpl {
	public:
		LightImpl();
		virtual ~LightImpl();

    protected:
        // Submit light parameters to OpenGL (lightID is GL_LIGHT<n> where <n> is in the range [0, 7])
        void submitLightParamsToGL(GLenum lightID) const;

        // Submit light transformation to OpenGL (lightID is GL_LIGHT<n> where <n> is in the range [0, 7]).
        void submitLightTransformToGL(GLenum lightID) const;

        LightType   myType;
        vec4        myAmbient;
        vec4        myDiffuse;
        vec4        mySpecular;
        vec4        myPositionOrDirection;
        vec3        mySpotDirection;
        float       mySpotExponent;
        float       mySpotCutoff;
        float       myConstAttn;
        float       myLinearAttn;
        float       myQuadricAttn;
	};
}
