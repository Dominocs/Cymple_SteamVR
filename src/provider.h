#ifndef _CPROVIDER_H_
#define _CPROVIDER_H_

#include <openvr_driver.h>
#include <thread>
#include <shared_mutex>
#include <WinSock2.h>
#include "virtualhmd/hmd.h"
#include "tracker.h"
#include "trackerRoles.h"
#define RX_BUFFER_LENGTH  4096
class CProvider : public vr::IServerTrackedDeviceProvider
{
public:
	virtual vr::EVRInitError Init(vr::IVRDriverContext* pDriverContext);
	virtual void Cleanup();
	virtual const char* const* GetInterfaceVersions() { return vr::k_InterfaceVersions; }
	virtual void RunFrame();
	virtual bool ShouldBlockStandbyMode() { return false; }
	virtual void EnterStandby();
	virtual void LeaveStandby();
	virtual vr::CVRPropertyHelpers* GetProperties() { return vr::VRProperties(); };

private:
	SOCKET udpSocket;
	std::thread* thUdpRW;
	char aucRxBuffer[RX_BUFFER_LENGTH];
	std::shared_mutex mutexUdpRW;
	int readsize = 0;
	CTracker* apstValidTracker[ROLE_MAX];
	std::vector<CTracker*>v_invalidTracker;
	void udpRW();
};

#endif

