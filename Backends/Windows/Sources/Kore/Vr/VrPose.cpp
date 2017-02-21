#include "pch.h"

#include "VrPose.h"

VrPose::VrPose() : orientation(Kore::Quaternion(0,0,0,0)), position(Kore::vec3(0,0,0)) {
}
