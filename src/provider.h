#ifndef _CPROVIDER_H_
#define _CPROVIDER_H_

#include <openvr_driver.h>
#include <thread>
#include <shared_mutex>
#include <WinSock2.h>
#include "eyeTracker/eyeTracker.h"
#define RX_BUFFER_LENGTH  1024
class ProviderClass : public vr::IServerTrackedDeviceProvider
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
	int dataLength = 0;
	SOCKET udpSocket;
	std::thread* threadUdpRW;
	char aucRxBuffer[RX_BUFFER_LENGTH];
	std::shared_mutex mutexUdpRW;
	void udpReadAndWrite();
	eyeTrackerClass* pEyeTracker = NULL;
	bool b_exit = false;
};

#endif

