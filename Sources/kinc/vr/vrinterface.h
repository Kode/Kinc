#pragma once

#include <kinc/global.h>

#include <kinc/math/matrix.h>
#include <kinc/math/quaternion.h>
#include <kinc/math/vector.h>

/*! \file vrinterface.h
    \brief The C-API for VR is currently deactivated and needs some work. Please use the Kore/C++-API in the meantime or send pull-requests.
*/

#ifdef KINC_VR

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { KINC_TRACKING_ORIGIN_STAND, KINC_TRACKING_ORIGIN_SIT } kinc_tracking_origin_t;

typedef enum { KINC_TRACKED_DEVICE_HMD, KINC_TRACKED_DEVICE_CONTROLLER, KINC_TRACKED_DEVICE_VIVE_TRACKER } kinc_tracked_device_t;

typedef struct kinc_vr_pose {
	kinc_quaternion_t orientation;
	kinc_vector3_t position;

	kinc_matrix4x4_t eye;
	kinc_matrix4x4_t projection;

	// fov
	float left;
	float right;
	float bottom;
	float top;
} kinc_vr_pose_t;

typedef struct kinc_vr_pose_state {
	kinc_vr_pose_t vrPose;
	kinc_vector3_t angularVelocity;     // Angular velocity in radians per second
	kinc_vector3_t linearVelocity;      // Velocity in meters per second
	kinc_vector3_t angularAcceleration; // Angular acceleration in radians per second per second
	kinc_vector3_t linearAcceleration;  // Acceleration in meters per second per second

	kinc_tracked_device_t trackedDevice;

	// Sensor status
	bool isVisible;
	bool hmdPresenting;
	bool hmdMounted;
	bool displayLost;
	bool shouldQuit;
	bool shouldRecenter;
} kinc_vr_pose_state_t;

typedef struct kinc_vr_sensor_state {
	kinc_vr_pose_state_t pose;
} kinc_vr_sensor_state_t;

void *kinc_vr_interface_init(void *hinst, const char *title, const char *windowClassName);
void kinc_vr_interface_begin();
void kinc_vr_interface_begin_render(int eye);
void kinc_vr_interface_end_render(int eye);
kinc_vr_sensor_state_t kinc_vr_interface_get_sensor_state(int eye);
kinc_vr_pose_state_t kinc_vr_interface_get_controller(int index);
void kinc_vr_interface_warp_swap();
void kinc_vr_interface_update_tracking_origin(kinc_tracking_origin_t origin);
void kinc_vr_interface_reset_hmd_pose();
void kinc_vr_interface_ovr_shutdown();

#ifdef __cplusplus
}
#endif

#endif
