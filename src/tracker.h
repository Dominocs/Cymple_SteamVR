#ifndef _CLIGHTVRTRACKER_H_
#define _CLIGHTVRTRACKER_H_

#include<openvr_driver.h>
#include<iostream>
#include"trackerRoles.h"
#include "driverProto.hpp"

class CTracker : public vr::ITrackedDeviceServerDriver
{
	public:
		CTracker(uint16_t);
		virtual vr::EVRInitError Activate(uint32_t unObjectId);
		virtual void Deactivate();
		virtual void EnterStandby(){};
		virtual void* GetComponent(const char* pchComponentNameAndVersion) { return nullptr; };
		virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize);
		virtual vr::DriverPose_t GetPose() { return this->pose; };
		void UpdatePose(TRACKER_INFO_S* pstLocation);
		uint16_t GetRole() { return this->role; };
	private:
		vr::TrackedDeviceIndex_t device_index = vr::k_unTrackedDeviceIndexInvalid /* track tag id */;
		vr::DriverPose_t pose = { 0 };/* track tag pose */
		uint16_t role = TrackerRole::ROLE_INVALID;
};

#endif // !_CLIGHTVRTRACKER_H_