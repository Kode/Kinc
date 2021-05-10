#ifdef KORE_HOLOLENS

#include "Hololens.winrt.h"
#include <Kinc/vr/vrinterface.h>

using namespace Kore;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;

using namespace Windows::Media::Capture;
using namespace Windows::Media::Capture::Frames;
using namespace Windows::Media::MediaProperties;

void *kinc_vr_interface_init(void *hinst, const char *title, const char *windowClassName) {
	return nullptr;
}

void kinc_vr_interface_begin() {
	holographicFrameController->begin();
}

void kinc_vr_interface_begin_render(int eye) {
	holographicFrameController->beginRender(eye);
}

kinc_vr_sensor_state_t kinc_vr_interface_get_sensor_state(int eye) {
	return holographicFrameController->getSensorState(eye);
}

void kinc_vr_interface_end_render(int eye) {
	holographicFrameController->endRender(eye);
}

void kinc_vr_interface_warp_swap() {
	holographicFrameController->warpSwap();
}

void kinc_vr_interface_ovr_shutdown() {
	holographicFrameController.reset();
	videoFrameProcessor.reset();
}

void kinc_vr_interface_reset_hmd_pose() {
	// not supported for hololens
}

void kinc_vr_interface_update_tracking_origin(TrackingOrigin origin) {
	// could be implemented by setting up the tracking coordinate system with an offset
}

CameraImage *VrInterface::getCurrentCameraImage() {
	if (videoFrameProcessor == nullptr) {
		return nullptr;
	}
	return videoFrameProcessor->getCurrentCameraImage(holographicFrameController->getCurrentWorldCoordinateSystem());
}

#endif