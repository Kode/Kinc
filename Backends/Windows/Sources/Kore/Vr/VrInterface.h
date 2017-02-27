#pragma once

//#include <kha/Image.h>
//#include <kha/math/Quaternion.h>
//#include <kha/vr/TimeWarpParms.h>

#include <Kore/Vr/SensorState.h>
#include "Kore/Graphics/Graphics.h"

#include "GL/CAPI_GLE.h"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI_GL.h"

enum TrackingOrigin { Stand, Sit };

namespace Kore {

	class OculusTexture {
	private:
		ovrSession				session;
		ovrTextureSwapChain		textureChain;
		OVR::Sizei				texSize;
		RenderTarget*			renderTarget;
	
	public:
		OculusTexture(ovrSession session, bool displayableOnHmd, OVR::Sizei size, int mipLevels, unsigned char * data, int sampleCount);
		~OculusTexture();
		OVR::Sizei getSize() const;
		void setAndClearRenderSurface();
		void commit();
		ovrTextureSwapChain getOculusTexture();

	};

	namespace VrInterface {
		void* init(void* hinst);
		SensorState* getSensorState();
		void warpSwap(/*kha::vr::TimeWarpParms_obj* parms*/);

		void ovrShutdown();
		void updateTrackingOrigin(TrackingOrigin origin);
		void resetHmdPose();
	}
}
