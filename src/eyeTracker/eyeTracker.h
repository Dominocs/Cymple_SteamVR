#pragma once
#include <openvr_driver.h>
#include "../networkDef.h"
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

class eyeTrackerClass : public vr::ITrackedDeviceServerDriver
{
public:
	eyeTrackerClass();
	virtual vr::EVRInitError Activate(uint32_t unObjectId);
	virtual void Deactivate();
	virtual void EnterStandby() {};
	void updateHmdPose();
	int update(MSG_OPENVR_EYE_EULER_S*pdata, uint32_t length);
	virtual void* GetComponent(const char* pchComponentNameAndVersion) { return nullptr; };
	virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize);
	virtual vr::DriverPose_t GetPose() { return this->pose; };
private:
	uint32_t currentBindController = vr::k_unTrackedDeviceIndexInvalid;
	uint8_t currentControllerRole = vr::TrackedControllerRole_OptOut;
	vr::TrackedDeviceIndex_t device_index = vr::k_unTrackedDeviceIndexInvalid /* track tag id */;
	vr::DriverPose_t pose = { 0 };/* track tag pose */
	vr::VRInputComponentHandle_t trigger_click_component_ = 0;
	vr::TrackedDevicePose_t hmdPose = { 0 };
};