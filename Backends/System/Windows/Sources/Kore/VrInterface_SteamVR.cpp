#include "pch.h"

#ifdef KORE_STEAMVR

#include <Kore/Vr/VrInterface.h>

#include <Kore/Graphics4/Graphics.h>
#include <Kore/Log.h>
//#include "Direct3D11.h"

#include <openvr.h>

#include <math.h>
#include <stdio.h>

using namespace Kore;

namespace {
	vr::IVRSystem* hmd;
	Graphics4::RenderTarget* leftTexture;
	Graphics4::RenderTarget* rightTexture;
	SensorState sensorStates[2];
	VrPoseState controller[vr::k_unMaxTrackedDeviceCount];

	mat4 convert(vr::HmdMatrix34_t& m) {
		mat4 mat;
		mat.Set(0, 0, m.m[0][0]); mat.Set(0, 1, m.m[0][1]); mat.Set(0, 2, m.m[0][2]); mat.Set(0, 3, m.m[0][3]);
		mat.Set(1, 0, m.m[1][0]); mat.Set(1, 1, m.m[1][1]); mat.Set(1, 2, m.m[1][2]); mat.Set(1, 3, m.m[1][3]);
		mat.Set(2, 0, m.m[2][0]); mat.Set(2, 1, m.m[2][1]); mat.Set(2, 2, m.m[2][2]); mat.Set(2, 3, m.m[2][3]);
		mat.Set(3, 0, 0); mat.Set(3, 1, 0); mat.Set(3, 2, 0); mat.Set(3, 3, 1);
		return mat;
	}

	mat4 convert(vr::HmdMatrix44_t& m) {
		mat4 mat;
		mat.Set(0, 0, m.m[0][0]); mat.Set(0, 1, m.m[0][1]); mat.Set(0, 2, m.m[0][2]); mat.Set(0, 3, m.m[0][3]);
		mat.Set(1, 0, m.m[1][0]); mat.Set(1, 1, m.m[1][1]); mat.Set(1, 2, m.m[1][2]); mat.Set(1, 3, m.m[1][3]);
		mat.Set(2, 0, m.m[2][0]); mat.Set(2, 1, m.m[2][1]); mat.Set(2, 2, m.m[2][2]); mat.Set(2, 3, m.m[2][3]);
		mat.Set(3, 0, m.m[3][0]); mat.Set(3, 1, m.m[3][1]); mat.Set(3, 2, m.m[3][2]); mat.Set(3, 3, m.m[3][3]);
		return mat;
	}

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
	
	void getPosition(const vr::HmdMatrix34_t* m, vec3* position) {
		position->x() = m->m[0][3];
		position->y() = m->m[1][3];
		position->z() = m->m[2][3];
	}

	void getOrientation(const vr::HmdMatrix34_t* m, Quaternion* orientation) {
		orientation->w = sqrt(fmax(0, 1 + m->m[0][0] + m->m[1][1] + m->m[2][2])) / 2;
		orientation->x = sqrt(fmax(0, 1 + m->m[0][0] - m->m[1][1] - m->m[2][2])) / 2;
		orientation->y = sqrt(fmax(0, 1 - m->m[0][0] + m->m[1][1] - m->m[2][2])) / 2;
		orientation->z = sqrt(fmax(0, 1 - m->m[0][0] - m->m[1][1] + m->m[2][2])) / 2;
		orientation->x = copysign(orientation->x, m->m[2][1] - m->m[1][2]);
		orientation->y = copysign(orientation->y, m->m[0][2] - m->m[2][0]);
		orientation->z = copysign(orientation->z, m->m[1][0] - m->m[0][1]);
	}

	void readSensorStates() {
		vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];
		vr::TrackedDevicePose_t predictedPoses[vr::k_unMaxTrackedDeviceCount];
		vr::VRCompositor()->WaitGetPoses(poses, vr::k_unMaxTrackedDeviceCount, predictedPoses, vr::k_unMaxTrackedDeviceCount);

		for (int device = 0; device < vr::k_unMaxTrackedDeviceCount; ++device) {
			if (poses[device].bPoseIsValid) {
				if (hmd->GetTrackedDeviceClass(device) == vr::TrackedDeviceClass_HMD) {
					VrPoseState poseState;
					poseState.linearVelocity = vec3(poses[device].vVelocity.v[0], poses[device].vVelocity.v[1], poses[device].vVelocity.v[2]);
					poseState.angularVelocity = vec3(poses[device].vAngularVelocity.v[0], poses[device].vAngularVelocity.v[1], poses[device].vAngularVelocity.v[2]);

					vr::HmdMatrix34_t m = poses[device].mDeviceToAbsoluteTracking;
					//log(Info, "x: %f y: %f z: %f", m.m[0][3], m.m[1][3], m.m[2][3]);

					getPosition(&m, &poseState.vrPose.position);
					getOrientation(&m, &poseState.vrPose.orientation);

					sensorStates[0].pose = poseState;
					sensorStates[1].pose = poseState;

					vr::HmdMatrix34_t leftEyeMatrix = hmd->GetEyeToHeadTransform(vr::Eye_Left);
					vr::HmdMatrix34_t rightEyeMatrix = hmd->GetEyeToHeadTransform(vr::Eye_Right);
					sensorStates[0].pose.vrPose.eye = convert(leftEyeMatrix).Invert() * convert(m).Invert();
					sensorStates[1].pose.vrPose.eye = convert(rightEyeMatrix).Invert() * convert(m).Invert();

					vr::HmdMatrix44_t leftProj = hmd->GetProjectionMatrix(vr::Eye_Left, 0.1f, 100.0f);
					vr::HmdMatrix44_t rightProj = hmd->GetProjectionMatrix(vr::Eye_Right, 0.1f, 100.0f);
					sensorStates[0].pose.vrPose.projection = convert(leftProj);
					sensorStates[1].pose.vrPose.projection = convert(rightProj);
				} else if (hmd->GetTrackedDeviceClass(device) == vr::TrackedDeviceClass_Controller ||
					hmd->GetTrackedDeviceClass(device) == vr::TrackedDeviceClass_GenericTracker) {
					VrPoseState poseState;
					poseState.linearVelocity = vec3(poses[device].vVelocity.v[0], poses[device].vVelocity.v[1], poses[device].vVelocity.v[2]);
					poseState.angularVelocity = vec3(poses[device].vAngularVelocity.v[0], poses[device].vAngularVelocity.v[1], poses[device].vAngularVelocity.v[2]);

					vr::HmdMatrix34_t m = poses[device].mDeviceToAbsoluteTracking;

					getPosition(&m, &poseState.vrPose.position);
					getOrientation(&m, &poseState.vrPose.orientation);

					//Kore::log(Kore::Info, "Pos of device %i %f %f %f", device, poseState.vrPose.position.x(), poseState.vrPose.position.y(), poseState.vrPose.position.z());

					if (hmd->GetTrackedDeviceClass(device) == vr::TrackedDeviceClass_Controller)
						poseState.trackedDevice = Controller;
					else if (hmd->GetTrackedDeviceClass(device) == vr::TrackedDeviceClass_GenericTracker)
						poseState.trackedDevice = ViveTracker;
					controller[device] = poseState;
				}
			}
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

	readSensorStates();
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

}

SensorState VrInterface::getSensorState(int eye) {
	return sensorStates[eye];
}

VrPoseState VrInterface::getController(int index) {
	return controller[index];
}

void VrInterface::warpSwap() {
#ifdef KORE_OPENGL
	vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)leftTexture->_texture, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
	vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)rightTexture->_texture, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
#else
	vr::Texture_t leftEyeTexture = { (void*)leftTexture->texture, vr::TextureType_DirectX, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
	vr::Texture_t rightEyeTexture = { (void*)rightTexture->texture, vr::TextureType_DirectX, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
#endif
}

void VrInterface::updateTrackingOrigin(TrackingOrigin origin) {

}

void VrInterface::resetHmdPose() {

}

void VrInterface::ovrShutdown() {
	vr::VR_Shutdown();
}

#endif
