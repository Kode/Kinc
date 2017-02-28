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

		GLuint					texId; // TODO: delete
		GLuint					fboId;
	
	public:
		OculusTexture(ovrSession session, bool displayableOnHmd, OVR::Sizei size, int mipLevels, unsigned char * data, int sampleCount);
		~OculusTexture();
		OVR::Sizei getSize() const;
		void setAndClearRenderSurface(int texId);
		void unsetRenderSurface();
		void commit();
		ovrTextureSwapChain getOculusTexture();

	};

	namespace VrInterface {
		void* init(void* hinst);
		void begin(int eye);
		void end(int eye);
		SensorState* getSensorState(int eye);
		void warpSwap(/*kha::vr::TimeWarpParms_obj* parms*/);

		void updateTrackingOrigin(TrackingOrigin origin);
		void resetHmdPose();
		void ovrShutdown();
	}
}
