#include <openvr_driver.h>
#include <math.h>
#include "provider.h"
#include "driverlog.h"
#include "basic.h"
#include "networkDef.h"
#include "network/socket.hpp"
#include "network/udp.hpp"
#ifdef _DEBUG
#include "debugDevice/debugHmd.h"
#include "debugDevice/debugController.h"
VirtualHmd* m_pNullHmdLatest = NULL;
DebugControllerClass* pDebugController_L = NULL;
DebugControllerClass* pDebugController_R = NULL;
#endif // DEBUG
#include "eyeTracker/eyeTracker.h"
using namespace vr;

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
#ifdef _DEBUG
    m_pNullHmdLatest = new VirtualHmd();
    vr::VRServerDriverHost()->TrackedDeviceAdded(m_pNullHmdLatest->GetSerialNumber().c_str(), vr::TrackedDeviceClass_HMD, m_pNullHmdLatest);
    pDebugController_L = new DebugControllerClass(vr::ETrackedControllerRole::TrackedControllerRole_RightHand);
    vr::VRServerDriverHost()->TrackedDeviceAdded("Debug Controller_R", vr::TrackedDeviceClass_Controller, pDebugController_L);
    pDebugController_R = new DebugControllerClass(vr::ETrackedControllerRole::TrackedControllerRole_LeftHand);
    vr::VRServerDriverHost()->TrackedDeviceAdded("Debug Controller_L", vr::TrackedDeviceClass_Controller, pDebugController_R);
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
        serverAddr.sin_port = htons(CYMPLE_OPENVR_PORT);  //监听端口号
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
    if (NULL != pDebugController_L) {
        delete(pDebugController_L);
        pDebugController_L = nullptr;
    }
    if (NULL != pDebugController_R) {
        delete(pDebugController_R);
        pDebugController_R = nullptr;
    }
#endif
    b_exit = true;
    if (NULL != pEyeTracker) {
        delete(pEyeTracker);
        pEyeTracker = nullptr;
    }
    if (NULL != threadUdpRW) {
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
    if (NULL != pDebugController_L) {
        pDebugController_L->RunFrame();
    }
    if (NULL != pDebugController_R) {
        pDebugController_R->RunFrame();
    }
#endif
    TLV_S* pstHdr;
    char* pData = aucRxBuffer;
    if (dataLength >= sizeof(TLV_S)){
        mutexUdpRW.lock_shared();
        while (dataLength >= sizeof(TLV_S)) {
            pstHdr = (TLV_S*)pData;
            int tmp = 0;
            switch(pstHdr->uiType){
                case MSG_OPENVR_EYE_EULER_E:
                    if (NULL == pEyeTracker) {
                        pEyeTracker = new eyeTrackerClass();
                        if (!vr::VRServerDriverHost()->TrackedDeviceAdded("Cymple Eye Tracker", vr::TrackedDeviceClass_Controller, pEyeTracker)) {
                            DriverLog("Failed to add Cymple Eye Tracker!\n");
                        }
                    }
                    if (NULL != pEyeTracker){
                        pEyeTracker->updateHmdPose();
                        tmp = pEyeTracker->update((MSG_OPENVR_EYE_EULER_S*)pData, dataLength);
                    }
                    else {
                        return;
                    }
                    break;
                default:
                    tmp = dataLength;
            }
            pData += tmp;
            dataLength -= tmp;
        }
        mutexUdpRW.unlock_shared();
    }
}
void ProviderClass::EnterStandby()
{

}
void ProviderClass::LeaveStandby()
{

}
void ProviderClass::udpReadAndWrite(){
    while (!b_exit) {
        mutexUdpRW.lock();
        int tmpLength =  CSocket::selectAndRecv(udpSocket, aucRxBuffer + dataLength, RX_BUFFER_LENGTH - dataLength);
        if (tmpLength > 0) {
            dataLength += tmpLength;
        }
        mutexUdpRW.unlock();
        Sleep(10);
    }
}
