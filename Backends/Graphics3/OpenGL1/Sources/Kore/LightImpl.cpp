#include "pch.h"
#include <Kore/Graphics3/Light.h>
#include <Kore/Math/Core.h>
#include <Kore/Graphics3/Graphics.h>
#include "ogl.h"

using namespace Kore;

// OpenGL man pages for "glLight" function:
// see https://www.opengl.org/sdk/docs/man2/xhtml/glLight.xml

LightImpl::LightImpl() :
    myType                  ( DirectionalLight ),
    myAmbient               ( 0, 0, 0, 1 ),
    myDiffuse               ( 1, 1, 1, 1 ),
    mySpecular              ( 1, 1, 1, 1 ),
    myPositionOrDirection   ( 0, 0, 1, 0 ),
    mySpotDirection         ( 0, 0, 1 ), // default point in +Z direction
    mySpotExponent          ( 0.0f ),
    mySpotCutoff            ( 180.0f ),
    myConstAttn             ( 1.0f ),
    myLinearAttn            ( 0.0f ),
    myQuadricAttn           ( 0.0f )
{
}

LightImpl::~LightImpl() {
}

void Light::setType(LightType type) {
    // Set new light type
    myType = type;

    // Update position/directior vector
    myPositionOrDirection[3] = (myType == DirectionalLight ? 0.0f : 1.0f);
}

void Light::setColors(const vec4& ambient, const vec4& diffuse, const vec4& specular) {
    // Store light colors
    myAmbient = ambient;
    myDiffuse = diffuse;
    mySpecular = specular;
}

void Light::setPosition(const vec3& position) {
    // Store position point (x, y, z, 1)
    myPositionOrDirection = vec4(position, 1);
}

void Light::setDirection(const vec3& direction) {
    if (myType == SpotLight) {
        // Store spot direction vector (x, y, z)
        mySpotDirection = direction;
    } else {
        // Store direction vector (x, y, z, 0)
        myPositionOrDirection = vec4(direction, 0);
    }
}

void Light::setSpot(float exponent, float cutoff) {
    mySpotExponent = exponent;
    mySpotCutoff = cutoff;
}

void Light::setAttenuation(float constAttn, float linearAttn, float quadricAttn) {
    myConstAttn = constAttn;
    myLinearAttn = linearAttn;
    myQuadricAttn = quadricAttn;
}

/*
                                    1
GL Light Intensity = -----------------------------------
                     constant + d*linear + d^2*quadratic
*/
void Light::setAttenuationRadius(float radius) {
    setAttenuation(1.0f, 1.0f / radius, 1.0f / radius);
}

void LightImpl::submitLightParamsToGL(GLenum lightID) const {
    // Submit colors
    glLightfv(lightID, GL_AMBIENT, myAmbient.values);
    glLightfv(lightID, GL_DIFFUSE, myDiffuse.values);
    glLightfv(lightID, GL_SPECULAR, mySpecular.values);

    // Submit attenuation
    glLightf(lightID, GL_CONSTANT_ATTENUATION, myConstAttn);
    glLightf(lightID, GL_LINEAR_ATTENUATION, myLinearAttn);
    glLightf(lightID, GL_QUADRATIC_ATTENUATION, myQuadricAttn);

    // Submit spot parameters
    glLightfv(lightID, GL_SPOT_DIRECTION, mySpotDirection.values);
    glLightf(lightID, GL_SPOT_EXPONENT, mySpotExponent);
    glLightf(lightID, GL_SPOT_CUTOFF, mySpotCutoff);
}

void LightImpl::submitLightTransformToGL(GLenum lightID) const {
    glLightfv(lightID, GL_POSITION, myPositionOrDirection.values);
}

Light::Light(LightType type) : LightImpl() {
    setType(type);
}

void Light::_set(int num) {
    GLenum lightID = GL_LIGHT0 + num;
    glEnable(lightID);
    submitLightParamsToGL(lightID);
    submitLightTransformToGL(lightID);
}
