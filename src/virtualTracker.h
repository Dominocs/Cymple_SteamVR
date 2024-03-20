#pragma once
#include <openvr_driver.h>
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
enum MyComponent
{
	MyComponent_a_touch,
	MyComponent_a_click,

	MyComponent_trigger_value,
	MyComponent_trigger_click,

	MyComponent_haptic,

	MyComponent_MAX
};


class VirtualTracker : public vr::ITrackedDeviceServerDriver
{
public:
	VirtualTracker(uint32_t,uint32_t);
	virtual vr::EVRInitError Activate(uint32_t unObjectId);
	virtual void Deactivate();
	virtual void EnterStandby() {};
	virtual void updatePose();
	virtual void* GetComponent(const char* pchComponentNameAndVersion) { return nullptr; };
	virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize);
	virtual vr::DriverPose_t GetPose() { return this->pose; };
private:
	vr::TrackedDeviceIndex_t device_index = vr::k_unTrackedDeviceIndexInvalid /* track tag id */;
	vr::DriverPose_t pose = { 0 };/* track tag pose */
	uint32_t itemId;
	uint32_t controllerId;
	vr::VRInputComponentHandle_t trigger_click_component_ = 0;
	double yaw = 0, pitch = 0;
};