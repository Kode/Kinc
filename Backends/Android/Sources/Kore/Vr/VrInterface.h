#pragma once

#ifdef ANDROID
	#include <jni.h>
#endif 

#include <kha/vr/SensorState.h>
#include <kha/vr/TimeWarpParms.h>

namespace Kore {

	namespace VrInterface {

#ifdef ANDROID
	// Save the JVM. Must be called before Initialize().
	// TODO: Can this be handled better?
	void SetJVM(JavaVM* jvm);
#endif

	// Calls ovr_enterVrMode
	void Initialize();

	void WarpSwapBlack();

	void WarpSwapLoadingIcon();

	
	kha::vr::SensorState_obj* GetSensorState();
	
	kha::vr::SensorState_obj* GetPredictedSensorState(float time);

	double GetTimeInSeconds();
	
	void WarpSwap(kha::vr::TimeWarpParms_obj* parms);
	
	
		
		
	};
}

