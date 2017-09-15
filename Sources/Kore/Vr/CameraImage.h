#pragma once

#include <Kore/Math/Quaternion.h>
#include <Kore/Math/Vector.h>

class CameraImage {
public:
	CameraImage(int imageHeight, int imageWidth,int* imageBGRA8Data, Kore::mat4 cameraViewTransform, Kore::mat4 cameraProjectionTransform, Kore::vec2 focalLength);
	~CameraImage();
	int* imageBGRA8Data;
	int imageHeight, imageWidth;

	//in world (root) coordinate  system
	Kore::mat4 cameraViewTransform;
	Kore::mat4 cameraProjectionTransform;
	Kore::vec2 focalLength;

	void getWorldRayForPixelPosition(Kore::vec2 pixelPosition, Kore::vec3 &origin, Kore::vec3 &direction); //origin at camera pos
	Kore::vec2 getPixelForWorldPosition(Kore::vec3 worldPos);
};

