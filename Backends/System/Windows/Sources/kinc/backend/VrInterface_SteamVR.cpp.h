#ifdef KORE_STEAMVR

#include <Kinc/graphics4/graphics.h>
#include <Kinc/graphics4/rendertarget.h>
#include <Kinc/math/quaternion.h>
#include <Kinc/math/vector.h>
#include <Kinc/vr/vrinterface.h>
#include <Kore/Input/Gamepad.h>
#include <Kore/Log.h>
// #include "Direct3D11.h"

#include <openvr.h>

#include <math.h>
#include <stdio.h>

using namespace Kore;

namespace {
	vr::IVRSystem *hmd;
	kinc_g4_render_target_t leftTexture;
	kinc_g4_render_target_t rightTexture;
	kinc_vr_sensor_state_t sensorStates[2];
	kinc_vr_pose_state_t controller[vr::k_unMaxTrackedDeviceCount];

	kinc_matrix4x4_t convert3x4(vr::HmdMatrix34_t &m) {
		kinc_matrix4x4_t mat;
		kinc_matrix4x4_set(&mat, 0, 0, m.m[0][0]);
		kinc_matrix4x4_set(&mat, 0, 1, m.m[0][1]);
		kinc_matrix4x4_set(&mat, 0, 2, m.m[0][2]);
		kinc_matrix4x4_set(&mat, 0, 3, m.m[0][3]);
		kinc_matrix4x4_set(&mat, 1, 0, m.m[1][0]);
		kinc_matrix4x4_set(&mat, 1, 1, m.m[1][1]);
		kinc_matrix4x4_set(&mat, 1, 2, m.m[1][2]);
		kinc_matrix4x4_set(&mat, 1, 3, m.m[1][3]);
		kinc_matrix4x4_set(&mat, 2, 0, m.m[2][0]);
		kinc_matrix4x4_set(&mat, 2, 1, m.m[2][1]);
		kinc_matrix4x4_set(&mat, 2, 2, m.m[2][2]);
		kinc_matrix4x4_set(&mat, 2, 3, m.m[2][3]);
		kinc_matrix4x4_set(&mat, 3, 0, 0);
		kinc_matrix4x4_set(&mat, 3, 1, 0);
		kinc_matrix4x4_set(&mat, 3, 2, 0);
		kinc_matrix4x4_set(&mat, 3, 3, 1);
		return mat;
	}

	kinc_matrix4x4_t convert4x4(vr::HmdMatrix44_t &m) {
		kinc_matrix4x4_t mat;
		kinc_matrix4x4_set(&mat, 0, 0, m.m[0][0]);
		kinc_matrix4x4_set(&mat, 0, 1, m.m[0][1]);
		kinc_matrix4x4_set(&mat, 0, 2, m.m[0][2]);
		kinc_matrix4x4_set(&mat, 0, 3, m.m[0][3]);
		kinc_matrix4x4_set(&mat, 1, 0, m.m[1][0]);
		kinc_matrix4x4_set(&mat, 1, 1, m.m[1][1]);
		kinc_matrix4x4_set(&mat, 1, 2, m.m[1][2]);
		kinc_matrix4x4_set(&mat, 1, 3, m.m[1][3]);
		kinc_matrix4x4_set(&mat, 2, 0, m.m[2][0]);
		kinc_matrix4x4_set(&mat, 2, 1, m.m[2][1]);
		kinc_matrix4x4_set(&mat, 2, 2, m.m[2][2]);
		kinc_matrix4x4_set(&mat, 2, 3, m.m[2][3]);
		kinc_matrix4x4_set(&mat, 3, 0, m.m[3][0]);
		kinc_matrix4x4_set(&mat, 3, 1, m.m[3][1]);
		kinc_matrix4x4_set(&mat, 3, 2, m.m[3][2]);
		kinc_matrix4x4_set(&mat, 3, 3, m.m[3][3]);
		return mat;
	}

	void getButtonEvent(const vr::VREvent_t &event) {
		Gamepad *gamepad = Gamepad::get(event.trackedDeviceIndex);
		switch (event.data.controller.button) {
		case vr::k_EButton_Grip:
			switch (event.eventType) {
			case vr::VREvent_ButtonPress:
				gamepad->Button(vr::k_EButton_Grip, 1);
				break;

			case vr::VREvent_ButtonUnpress:
				gamepad->Button(vr::k_EButton_Grip, 0);
				break;
			}
			break;

		case vr::k_EButton_SteamVR_Trigger:
			switch (event.eventType) {
			case vr::VREvent_ButtonPress:
				gamepad->Button(vr::k_EButton_SteamVR_Trigger, 1);
				break;

			case vr::VREvent_ButtonUnpress:
				gamepad->Button(vr::k_EButton_SteamVR_Trigger, 0);
				break;
			}
			break;

		case vr::k_EButton_SteamVR_Touchpad:
			// TODO: add axis
			switch (event.eventType) {
			case vr::VREvent_ButtonPress:
				gamepad->Button(vr::k_EButton_SteamVR_Touchpad, 1);
				break;

			case vr::VREvent_ButtonUnpress:
				gamepad->Button(vr::k_EButton_SteamVR_Touchpad, 0);
				break;

			case vr::VREvent_ButtonTouch:
				break;

			case vr::VREvent_ButtonUntouch:
				break;
			}
			break;

		case vr::k_EButton_ApplicationMenu:
			switch (event.eventType) {
			case vr::VREvent_ButtonPress:
				gamepad->Button(vr::k_EButton_ApplicationMenu, 1);
				break;

			case vr::VREvent_ButtonUnpress:
				gamepad->Button(vr::k_EButton_ApplicationMenu, 0);
				break;
			}
			break;
		}
	}

	void processVREvent(const vr::VREvent_t &event) {
		switch (event.eventType) {
		case vr::VREvent_None:
			// log(Info, "The event is invalid.");
			break;
		case vr::VREvent_TrackedDeviceActivated:
			// SetupRenderModelForTrackedDevice(event.trackedDeviceIndex);
			// dprintf("Device %u attached. Setting up render model.\n", event.trackedDeviceIndex);
			log(Info, "Device %u attached", event.trackedDeviceIndex);
			break;
		case vr::VREvent_TrackedDeviceDeactivated:
			log(Info, "Device %u detached.", event.trackedDeviceIndex);
			break;
		case vr::VREvent_TrackedDeviceUpdated:
			log(Info, "Device %u updated.", event.trackedDeviceIndex);
			break;
		}

		// Get buttons
		getButtonEvent(event);
	}

	void getPosition(const vr::HmdMatrix34_t *m, kinc_vector3_t *position) {
		position->x = m->m[0][3];
		position->y = m->m[1][3];
		position->z = m->m[2][3];
	}

	void getOrientation(const vr::HmdMatrix34_t *m, kinc_quaternion_t *orientation) {
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
					kinc_vr_pose_state_t poseState;
					poseState.linearVelocity.x = poses[device].vVelocity.v[0];
					poseState.linearVelocity.y = poses[device].vVelocity.v[1];
					poseState.linearVelocity.z = poses[device].vVelocity.v[2];
					poseState.angularVelocity.x = poses[device].vAngularVelocity.v[0];
					poseState.angularVelocity.y = poses[device].vAngularVelocity.v[1];
					poseState.angularVelocity.z = poses[device].vAngularVelocity.v[2];

					vr::HmdMatrix34_t m = poses[device].mDeviceToAbsoluteTracking;
					// log(Info, "x: %f y: %f z: %f", m.m[0][3], m.m[1][3], m.m[2][3]);

					getPosition(&m, &poseState.vrPose.position);
					getOrientation(&m, &poseState.vrPose.orientation);

					sensorStates[0].pose = poseState;
					sensorStates[1].pose = poseState;

					vr::HmdMatrix34_t leftEyeMatrix = hmd->GetEyeToHeadTransform(vr::Eye_Left);
					vr::HmdMatrix34_t rightEyeMatrix = hmd->GetEyeToHeadTransform(vr::Eye_Right);
					sensorStates[0].pose.vrPose.eye = convert3x4(leftEyeMatrix).Invert() * convert3x4(m).Invert();
					sensorStates[1].pose.vrPose.eye = convert3x4(rightEyeMatrix).Invert() * convert3x4(m).Invert();

					vr::HmdMatrix44_t leftProj = hmd->GetProjectionMatrix(vr::Eye_Left, 0.1f, 100.0f);
					vr::HmdMatrix44_t rightProj = hmd->GetProjectionMatrix(vr::Eye_Right, 0.1f, 100.0f);
					sensorStates[0].pose.vrPose.projection = convert4x4(leftProj);
					sensorStates[1].pose.vrPose.projection = convert4x4(rightProj);
				}
				else if (hmd->GetTrackedDeviceClass(device) == vr::TrackedDeviceClass_Controller ||
				         hmd->GetTrackedDeviceClass(device) == vr::TrackedDeviceClass_GenericTracker) {
					kinc_vr_pose_state_t poseState;
					poseState.linearVelocity.x = poses[device].vVelocity.v[0];
					poseState.linearVelocity.x = poses[device].vVelocity.v[1];
					poseState.linearVelocity.x = poses[device].vVelocity.v[2];
					poseState.angularVelocity.x = poses[device].vAngularVelocity.v[0];
					poseState.angularVelocity.y = poses[device].vAngularVelocity.v[1];
					poseState.angularVelocity.z = poses[device].vAngularVelocity.v[2];

					vr::HmdMatrix34_t m = poses[device].mDeviceToAbsoluteTracking;

					getPosition(&m, &poseState.vrPose.position);
					getOrientation(&m, &poseState.vrPose.orientation);

					// Kore::log(Kore::Info, "Pos of device %i %f %f %f", device, poseState.vrPose.position.x(), poseState.vrPose.position.y(),
					// poseState.vrPose.position.z());

					if (hmd->GetTrackedDeviceClass(device) == vr::TrackedDeviceClass_Controller)
						poseState.trackedDevice = KINC_TRACKED_DEVICE_CONTROLLER;
					else if (hmd->GetTrackedDeviceClass(device) == vr::TrackedDeviceClass_GenericTracker)
						poseState.trackedDevice = KINC_TRACKED_DEVICE_VIVE_TRACKER;
					controller[device] = poseState;
				}
			}
		}
	}
}

void *kinc_vr_interface_init(void *hinst, const char *title, const char *windowClassName) {
	vr::HmdError error;
	hmd = vr::VR_Init(&error, vr::VRApplication_Scene);
	// vr::IVRRenderModels* renderModels = (vr::IVRRenderModels*)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &error);
	vr::VRCompositor();

	uint32_t width, height;
	hmd->GetRecommendedRenderTargetSize(&width, &height);

	kinc_g4_render_target_init(&leftTexture, width, height, 0, false, KINC_G4_RENDER_TARGET_FORMAT_32BIT, 0, 0);
	kinc_g4_render_target_init(&rightTexture, width, height, 0, false, KINC_G4_RENDER_TARGET_FORMAT_32BIT, 0, 0);

	return nullptr;
}

void kinc_vr_interface_begin() {
	vr::VREvent_t event;
	while (hmd->PollNextEvent(&event, sizeof(event))) {
		processVREvent(event);
	}

	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; ++unDevice) {
		vr::VRControllerState_t state;
		if (hmd->GetControllerState(unDevice, &state, sizeof(state))) {
			// m_rbShowTrackedDevice[unDevice] = state.ulButtonPressed == 0;
		}
	}

	readSensorStates();
}

void kinc_vr_interface_begin_render(int eye) {
	if (eye == 0) {
		kinc_g4_render_target_t *renderTargets[1] = {&leftTexture};
		kinc_g4_set_render_targets(renderTargets, 1);
	}
	else {
		kinc_g4_render_target_t *renderTargets[1] = {&rightTexture};
		kinc_g4_set_render_targets(renderTargets, 1);
	}
}

void kinc_vr_interface_end_render(int eye) {}

kinc_vr_sensor_state_t kinc_vr_interface_get_sensor_state(int eye) {
	return sensorStates[eye];
}

kinc_vr_pose_state_t kinc_vr_interface_get_controller(int index) {
	return controller[index];
}

void kinc_vr_interface_warp_swap() {
#ifdef KORE_OPENGL
	vr::Texture_t leftEyeTexture = {(void *)(uintptr_t)leftTexture.impl._texture, vr::TextureType_OpenGL, vr::ColorSpace_Gamma};
	vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
	vr::Texture_t rightEyeTexture = {(void *)(uintptr_t)rightTexture.impl._texture, vr::TextureType_OpenGL, vr::ColorSpace_Gamma};
	vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
#else
	vr::Texture_t leftEyeTexture = {(void *)(uintptr_t)leftTexture.impl.textureRender, vr::TextureType_DirectX, vr::ColorSpace_Gamma};
	vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
	vr::Texture_t rightEyeTexture = {(void *)(uintptr_t)rightTexture.impl.textureRender, vr::TextureType_DirectX, vr::ColorSpace_Gamma};
	vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
#endif
}

void kinc_vr_interface_update_tracking_origin(kinc_tracking_origin_t origin) {}

void kinc_vr_interface_reset_hmd_pose() {}

void kinc_vr_interface_ovr_shutdown() {
	vr::VR_Shutdown();
}

#endif
