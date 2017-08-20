#include "pch.h"

#include "CameraImage.h"

using namespace Kore;

CameraImage::CameraImage(int imageHeight, int imageWidth, u8* imageBGRA8Data, mat4 cameraViewTransform,mat4 cameraProjectionTransform)
	:imageHeight(imageHeight), imageWidth(imageWidth),imageBGRA8Data(imageBGRA8Data), cameraViewTransform(cameraViewTransform), cameraProjectionTransform(cameraProjectionTransform){
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
	auto axsX = proj.GetRow(0);
	auto axsY = proj.GetRow(1);
	auto axsZ = proj.GetRow(2);
	from.z = to.z / axsZ.z;
	from.y = (to.y - (from.z * axsY.z)) / axsY.y;
	from.x = (to.x - (from.z * axsX.z)) / axsX.x;
	return from;
}

//see https://developer.microsoft.com/en-us/windows/mixed-reality/locatable_camera
void CameraImage::getWorldRayForPixelPosition(vec2 pixelPosition, vec3 &origin, vec3 &direction)
{
	vec2 ImagePosZeroToOne(pixelPosition.x / imageWidth, 1.0 - (pixelPosition.y / imageHeight));
	vec2 ImagePosProjected = ((ImagePosZeroToOne * 2.0f) - vec2(1, 1)); // -1 to 1 space
	vec3 CameraSpacePos = UnProjectVector(cameraProjectionTransform, vec3(ImagePosProjected, 1));
	vec3 WorldSpaceRayPoint1 = mul(CameraToWorld, float4(0, 0, 0, 1)); // camera location in world space
	vec3 WorldSpaceRayPoint2 = mul(CameraToWorld, CameraSpacePos); // ray point in world space
}