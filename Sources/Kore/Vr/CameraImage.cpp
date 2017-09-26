#include "pch.h"

#include "CameraImage.h"

using namespace Kore;

CameraImage::CameraImage(int imageWidth, int imageHeight, int* imageBGRA8Data, mat4 cameraViewTransform,mat4 cameraProjectionTransform,Kore::vec2 focalLength)
	:imageWidth(imageWidth), imageHeight(imageHeight), imageBGRA8Data(imageBGRA8Data), cameraViewTransform(cameraViewTransform), cameraProjectionTransform(cameraProjectionTransform), focalLength(focalLength){
}

CameraImage::~CameraImage()
{
	delete[] imageBGRA8Data;
}

//see https://developer.microsoft.com/en-us/windows/mixed-reality/locatable_camera
vec2 CameraImage::getPixelForWorldPosition(vec3 worldPosition)
{
	//todo
	return vec2(0, 0);
}

static vec3 UnProjectVector(mat4 proj, vec3 to)
{
	vec3 from(0, 0, 0);
	vec3 axsX = vec3(proj.get(0,0), proj.get(0, 1), proj.get(0, 2)); // proj.GetRow(0); TRANSPOSE ??
	vec3 axsY = vec3(proj.get(1, 0), proj.get(1, 1), proj.get(1, 2)); // proj.GetRow(1); TRANSPOSE ??
	vec3 axsZ = vec3(proj.get(2, 0), proj.get(2, 1), proj.get(2, 2)); // proj.GetRow(2); TRANSPOSE ??

	float z = to.z() / axsZ.z();
	float y = (to.y() - (from.z() * axsY.z())) / axsY.y();
	float x = (to.x() - (from.z() * axsX.z())) / axsX.x();
	return vec3(x,y,z);
}

//see https://developer.microsoft.com/en-us/windows/mixed-reality/locatable_camera
void CameraImage::getWorldRayForPixelPosition(vec2 pixelPosition, vec3 &origin, vec3 &direction)
{
	vec2 ImagePosZeroToOne(pixelPosition.x() / imageWidth, 1.0f - (pixelPosition.y() / imageHeight));
	vec2 ImagePosProjected = ((ImagePosZeroToOne * 2.0f) - vec2(1, 1)); // -1 to 1 space
	vec3 CameraSpacePos = UnProjectVector(cameraProjectionTransform, vec3(ImagePosProjected, 1));
	mat4 CameraToWorld = cameraViewTransform; //.Invert(); //NOT INVERTED OR CORRECT ??
	vec3 WorldSpaceRayPoint1 = CameraToWorld*vec4(0, 0, 0, 1); // camera location in world space
	vec3 WorldSpaceRayPoint2 = CameraToWorld*vec4(CameraSpacePos,1); // ray point in world space
	origin = WorldSpaceRayPoint1;
	direction = (WorldSpaceRayPoint2 - WorldSpaceRayPoint1).normalize();
}