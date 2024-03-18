#include <openvr_driver.h>
#include"trackerRoles.h"
#include "driverlog.h"
#include "DriverFactory.h"
#include "provider.h"
#include "tracker.h"
#include "basics.h"
#include <string>
#include "driverProto.hpp"

using namespace vr; 
CTracker::CTracker(uint16_t role)
{
	this->role = role;
	pose.poseIsValid = true;
	pose.result = vr::TrackingResult_Running_OK;
	pose.deviceIsConnected = true;
	pose.poseTimeOffset = 0;
	pose.qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0);
	pose.qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);
	pose.qRotation.w = 1;
	pose.qRotation.x = 0;
	pose.qRotation.y = 0;
	pose.qRotation.z = 0;
}

EVRInitError CTracker::Activate(uint32_t unObjectId)
{
	this->device_index = unObjectId;
	auto props = GetDriverProvider()->GetProperties()->TrackedDeviceToPropertyContainer(this->device_index);
	// Set some universe ID (Must be 2 or higher)
	GetDriverProvider()->GetProperties()->SetUint64Property(props, vr::Prop_CurrentUniverseId_Uint64, 4);
	// Set up a model "number" (not needed but good to have)
	GetDriverProvider()->GetProperties()->SetStringProperty(props, vr::Prop_ModelNumber_String, "LightVR Tracker");
	// Opt out of hand selection
	GetDriverProvider()->GetProperties()->SetInt32Property(props, vr::Prop_ControllerRoleHint_Int32, vr::ETrackedControllerRole::TrackedControllerRole_OptOut);
	vr::VRProperties()->SetInt32Property(props, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_GenericTracker);
	vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerHandSelectionPriority_Int32, -1);
	// Set up a render model path
	GetDriverProvider()->GetProperties()->SetStringProperty(props, vr::Prop_RenderModelName_String, "{htc}/rendermodels/vr_tracker_vive_1_0");
	// Automatically select vive tracker roles and set hints for games that need it (Beat Saber avatar mod, for example)
		// Set the icon
	vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceOff_String, "{lightvr_release}/icons/test.png");
	vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceReady_String, "{lightvr_release}/icons/test.png");
	vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceReadyAlert_String, "{lightvr_release}/icons/test.png");
	vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceNotReady_String, "{lightvr_release}/icons/test.png");
	vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceStandby_String, "{lightvr_release}/icons/test.png");
	vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceAlertLow_String, "{lightvr_release}/icons/test.png");
	auto roleHint = getTrackerRole(this->role);
	if (roleHint != "")
		GetDriverProvider()->GetProperties()->SetStringProperty(props, vr::Prop_ControllerType_String, roleHint.c_str());

	auto role = getTrackerRole(this->role);
	if (role != "")
		vr::VRSettings()->SetString(vr::k_pch_Trackers_Section,  "/ devices / slimevr / ", role.c_str());

	DriverLog("Tracker:%s activate.\n", getTrackerRole(this->role).c_str());
	return VRInitError_None;
}

void CTracker::Deactivate()
{
	this->role = TrackerRole::ROLE_INVALID;
	this->device_index = k_unTrackedDeviceIndexInvalid;
	DriverLog("Tracker:%s Deactivate.\n", getTrackerRole(this->role).c_str());
}

void CTracker::UpdatePose(TRACKER_INFO_S*pstLocation)
{
	if (pstLocation->bEnable) {
		pose.deviceIsConnected = true;
		memcpy(pose.vecPosition, pstLocation->adPose, sizeof(double) * 3);
		memcpy(&pose.qRotation.w, pstLocation->adQua, sizeof(double) * 4);
	}
	else {
		pose.deviceIsConnected = false;
	}
	vr::VRServerDriverHost()->TrackedDevicePoseUpdated(device_index, pose, sizeof(DriverPose_t));
}


void CTracker::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
	if (unResponseBufferSize >= 1)
		pchResponseBuffer[0] = 0;
}