#include "pch.h"

#ifdef KORE_STEAMVR

#include <Kore/Vr/VrInterface.h>

#include <Kore/Graphics4/Graphics.h>
#include <Kore/Log.h>
#include "Direct3D11.h"

#include <openvr.h>

using namespace Kore;

namespace {
	vr::IVRSystem* hmd;
	Graphics4::RenderTarget* leftTexture;
	Graphics4::RenderTarget* rightTexture;

	void processVREvent(const vr::VREvent_t & event) {
		switch (event.eventType) {
		case vr::VREvent_TrackedDeviceActivated:
			//SetupRenderModelForTrackedDevice(event.trackedDeviceIndex);
			//dprintf("Device %u attached. Setting up render model.\n", event.trackedDeviceIndex);
			break;
		case vr::VREvent_TrackedDeviceDeactivated:
			printf("Device %u detached.\n", event.trackedDeviceIndex);
			break;
		case vr::VREvent_TrackedDeviceUpdated:
			printf("Device %u updated.\n", event.trackedDeviceIndex);
			break;
		}
	}
}

void* VrInterface::init(void* hinst, const char* title, const char* windowClassName) {
	vr::HmdError error;
	hmd = vr::VR_Init(&error, vr::VRApplication_Scene);
	//vr::IVRRenderModels* renderModels = (vr::IVRRenderModels*)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &error);
	vr::VRCompositor();

	uint32_t width, height;
	hmd->GetRecommendedRenderTargetSize(&width, &height);

	leftTexture = new Graphics4::RenderTarget(width, height, 16);
	rightTexture = new Graphics4::RenderTarget(width, height, 16);
	
	return nullptr;
}

void VrInterface::begin() {

}

void VrInterface::beginRender(int eye) {
	if (eye == 0) {
		Kore::Graphics4::setRenderTarget(leftTexture);
	}
	else {
		Kore::Graphics4::setRenderTarget(rightTexture);
	}
}

void VrInterface::endRender(int eye) {
	vr::Texture_t leftEyeTexture = { (void*)leftTexture->texture, vr::TextureType_DirectX, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
	vr::Texture_t rightEyeTexture = { (void*)rightTexture->texture, vr::TextureType_DirectX, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
}

SensorState* VrInterface::getSensorState(int eye) {
	SensorState* sensorState = new SensorState();

	VrPoseState* poseState = new VrPoseState();
	vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];
	vr::TrackedDevicePose_t predictedPoses[vr::k_unMaxTrackedDeviceCount];
	vr::VRCompositor()->WaitGetPoses(poses, vr::k_unMaxTrackedDeviceCount, predictedPoses, vr::k_unMaxTrackedDeviceCount);

	for (int device = 0; device < vr::k_unMaxTrackedDeviceCount; ++device) {
		if (poses[device].bPoseIsValid) {
			if (hmd->GetTrackedDeviceClass(device) == vr::TrackedDeviceClass_HMD) {
				poseState->linearVelocity = vec3(poses[device].vVelocity.v[0], poses[device].vVelocity.v[1], poses[device].vVelocity.v[2]);
				poseState->angularVelocity = vec3(poses[device].vAngularVelocity.v[0], poses[device].vAngularVelocity.v[1], poses[device].vAngularVelocity.v[2]);
				
				vr::HmdMatrix34_t m = poses[device].mDeviceToAbsoluteTracking;
				
				poseState->vrPose->position = vec3(m.m[0][3], m.m[1][3], m.m[2][3]);

				Quaternion q;
				q.w = sqrt(fmax(0, 1 + m.m[0][0] + m.m[1][1] + m.m[2][2])) / 2;
				q.x = sqrt(fmax(0, 1 + m.m[0][0] - m.m[1][1] - m.m[2][2])) / 2;
				q.y = sqrt(fmax(0, 1 - m.m[0][0] + m.m[1][1] - m.m[2][2])) / 2;
				q.z = sqrt(fmax(0, 1 - m.m[0][0] - m.m[1][1] + m.m[2][2])) / 2;
				q.x = copysign(q.x, m.m[2][1] - m.m[1][2]);
				q.y = copysign(q.y, m.m[0][2] - m.m[2][0]);
				q.z = copysign(q.z, m.m[1][0] - m.m[0][1]);
				poseState->vrPose->orientation = q;
			}
		}
	}

	VrPoseState* predictedPoseState = new VrPoseState();

	sensorState->pose = poseState;
	sensorState->predictedPose = predictedPoseState;

	return sensorState;
}

void VrInterface::warpSwap() {
	vr::VREvent_t event;
	while (hmd->PollNextEvent(&event, sizeof(event))) {
		processVREvent(event);
	}

	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; ++unDevice) {
		vr::VRControllerState_t state;
		if (hmd->GetControllerState(unDevice, &state, sizeof(state))) {
			//m_rbShowTrackedDevice[unDevice] = state.ulButtonPressed == 0;
		}
	}
}

void VrInterface::updateTrackingOrigin(TrackingOrigin origin) {

}

void VrInterface::resetHmdPose() {

}

void VrInterface::ovrShutdown() {
	vr::VR_Shutdown();
}

#endif
