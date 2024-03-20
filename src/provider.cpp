#include <openvr_driver.h>
#include <math.h>
#include "provider.h"
#include "driverlog.h"
#include "basic.h"
#include "networkDef.h"
#include "network/socket.hpp"
#include "network/udp.hpp"
#ifdef _DEBUG
#include "virtualDevice/hmd.h"
#include "virtualTracker.h"
#endif // DEBUG

using namespace vr;
static bool b_exit = false;

VirtualHmd* m_pNullHmdLatest = NULL;
std::vector<VirtualTracker*> g_pVirtualTracker;
//==================================================================
//函 数 名：Init
//功能描述：驱动初始化入口
//输入参数：
//返 回 值：
//作    者：Dominocs
//日    期：2022/06/21
//修改记录：
//==================================================================
EVRInitError ProviderClass::Init(IVRDriverContext* pDriverContext)
{
    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
    InitDriverLog(vr::VRDriverLog());
    b_exit = false;
#ifdef _DEBUG
    m_pNullHmdLatest = new VirtualHmd();
    vr::VRServerDriverHost()->TrackedDeviceAdded(m_pNullHmdLatest->GetSerialNumber().c_str(), vr::TrackedDeviceClass_HMD, m_pNullHmdLatest);
    VirtualTracker* tmp = new VirtualTracker(4, vr::ETrackedControllerRole::TrackedControllerRole_Stylus);
    vr::VRServerDriverHost()->TrackedDeviceAdded("Test Tracker1", vr::TrackedDeviceClass_Controller, tmp);
    g_pVirtualTracker.push_back(tmp);
    //tmp = new VirtualTracker(5, vr::ETrackedControllerRole::TrackedControllerRole_OptOut);
    //vr::VRServerDriverHost()->TrackedDeviceAdded("Test Tracker2", vr::TrackedDeviceClass_Controller, tmp);
    //g_pVirtualTracker.push_back(tmp);
    //tmp = new VirtualTracker(5, vr::ETrackedControllerRole::TrackedControllerRole_LeftHand);
    //vr::VRServerDriverHost()->TrackedDeviceAdded("Test Tracker2", vr::TrackedDeviceClass_Controller, tmp);
    //g_pVirtualTracker.push_back(tmp);
    //tmp = new VirtualTracker(6, vr::ETrackedControllerRole::TrackedControllerRole_LeftHand);
    //vr::VRServerDriverHost()->TrackedDeviceAdded("Test Tracker3", vr::TrackedDeviceClass_Controller, tmp);
    //g_pVirtualTracker.push_back(tmp);
#endif
    
    udpSocket = UdpSocketClass::createUdp();
    if (INVALID_SOCKET == udpSocket) {
        DriverLog("Failed to create udp socket!\n");
        return VRInitError_Driver_Failed;
    }
    else {
        sockaddr_in serverAddr{};
        serverAddr.sin_family = PF_INET;  //IPV4
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);  //addr
        serverAddr.sin_port = htons(CYMPLE_STEAMVR_PORT);  //监听端口号
        if (0 != CSocket::bind(udpSocket, &serverAddr)) {
            DriverLog("Failed to bind udp !\n");
            closesocket(udpSocket);
            return VRInitError_Driver_Failed;
        }
    }
    threadUdpRW = new std::thread(&ProviderClass::udpReadAndWrite, this);
    if (NULL == threadUdpRW) {
        DriverLog("Failed to create udp read&write thread!\n");
        return VRInitError_Driver_Failed;
    }
    DriverLog("Init Cymple Provider Successful!\n");
    
	return VRInitError_None;
}
//==================================================================
//函 数 名：Cleanup
//功能描述：清除
//输入参数：
//返 回 值：
//作    者：Dominocs
//日    期：2022/06/21
//修改记录：
//==================================================================
void ProviderClass::Cleanup()
{
#ifdef _DEBUG
    if (NULL != m_pNullHmdLatest){
        delete(m_pNullHmdLatest);
        m_pNullHmdLatest = nullptr;
    }
    for (auto item : g_pVirtualTracker) {
        delete item;
    }
#endif
    if (NULL != threadUdpRW) {
        b_exit = true;
        threadUdpRW->join();
        threadUdpRW = NULL;
    }
    if (INVALID_SOCKET != udpSocket) {
        closesocket(udpSocket);
    }
}

void ProviderClass::RunFrame()
{
#ifdef _DEBUG
    if (NULL != m_pNullHmdLatest)
    {
        m_pNullHmdLatest->RunFrame();
    }
    for (auto item : g_pVirtualTracker) {
        item->updatePose();
    }
#endif
    TLV_S* pstHdr;
    mutexUdpRW.lock_shared();
    mutexUdpRW.unlock_shared();
}
void ProviderClass::EnterStandby()
{

}
void ProviderClass::LeaveStandby()
{

}
void ProviderClass::udpReadAndWrite() {
}
