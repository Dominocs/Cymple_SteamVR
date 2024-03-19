#ifndef _CPROVIDER_H_
#define _CPROVIDER_H_

#include <openvr_driver.h>
#include <thread>
#include <shared_mutex>
#include <WinSock2.h>
#define RX_BUFFER_LENGTH  4096
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
	SOCKET udpSocket;
	std::thread* threadUdpRW;
	char aucRxBuffer[RX_BUFFER_LENGTH];
	std::shared_mutex mutexUdpRW;
	void udpReadAndWrite();
};

#endif

