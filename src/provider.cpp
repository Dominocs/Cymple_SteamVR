#include <openvr_driver.h>
#include <math.h>
#include "provider.h"
#include "driverlog.h"
#include "trackerRoles.h"
#include "basics.h"
#include "network/socket.hpp"
#include "network/udp.hpp"
#include "driverProto.hpp"
#include "hmdpose.hpp"
#ifdef _DEBUG
#include "virtualhmd/hmd.h"
#endif // DEBUG

using namespace vr;
static bool b_exit = false;

static CHmdDriver* m_pNullHmdLatest = NULL;
//==================================================================
//函 数 名：Init
//功能描述：驱动初始化入口
//输入参数：
//返 回 值：
//作    者：Dominocs
//日    期：2022/06/21
//修改记录：
//==================================================================
EVRInitError CProvider::Init(IVRDriverContext* pDriverContext)
{
    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
    InitDriverLog(vr::VRDriverLog());
    b_exit = false;
#ifdef _DEBUG
    m_pNullHmdLatest = new CHmdDriver();
    vr::VRServerDriverHost()->TrackedDeviceAdded(m_pNullHmdLatest->GetSerialNumber().c_str(), vr::TrackedDeviceClass_HMD, m_pNullHmdLatest);
#endif
    udpSocket = CUdp::createUdp();
    if (INVALID_SOCKET == udpSocket) {
        DriverLog("Failed to create udp socket!\n");
        return VRInitError_Driver_Failed;
    }
    else {
        sockaddr_in serverAddr{};
        serverAddr.sin_family = PF_INET;  //IPV4
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);  //addr
        serverAddr.sin_port = htons(DRIVER_PORT);  //监听端口号
        if (0 != CSocket::bind(udpSocket, &serverAddr)) {
            DriverLog("Failed to bind udp !\n");
            closesocket(udpSocket);
            return VRInitError_Driver_Failed;
        }
    }
    thUdpRW = new std::thread(&CProvider::udpRW, this); 
    if (NULL == thUdpRW) {
        DriverLog("Failed to create udp read&write thread!\n");
        return VRInitError_Driver_Failed;
    }
    memset(apstValidTracker, 0, sizeof(apstValidTracker));
    v_invalidTracker.clear();
    DriverLog("Init CyTag Provider Successful!\n");
    
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
void CProvider::Cleanup()
{
#ifdef _DEBUG
    if (NULL != m_pNullHmdLatest){
        delete(m_pNullHmdLatest);
        m_pNullHmdLatest = nullptr;
    }
#endif
    if (NULL != thUdpRW) {
        b_exit = true;
        thUdpRW->join();
        thUdpRW = NULL;
    }
    if (INVALID_SOCKET != udpSocket) {
        closesocket(udpSocket);
    }
}

void CProvider::RunFrame()
{
#ifdef _DEBUG
    if (NULL != m_pNullHmdLatest)
    {
        m_pNullHmdLatest->RunFrame();
    }
#endif
    TLV_S* pstHdr;
    mutexUdpRW.lock_shared();
    if (readsize >= sizeof(TLV_S)) {
        pstHdr = (TLV_S*)aucRxBuffer;
        uint8_t ucPart = 0;
        size_t invalidTagIndex = 0;
        if (pstHdr->uiLength == readsize) {
            switch (pstHdr->uiType) {
            case MSG_SERVER_POSE2DRIVER_E:
                for (TRACKER_INFO_S* pstTagData = (TRACKER_INFO_S*)(pstHdr + 1)
                    ; (char*)pstTagData < aucRxBuffer + readsize; pstTagData++) {
                    ucPart = pstTagData->ucPart;
                    if (ucPart < ROLE_MAX) {
                        if ((NULL == apstValidTracker[ucPart]) && (false != pstTagData->bEnable)) {
                            apstValidTracker[ucPart] = new CTracker(ucPart);
                            DriverLog("Create Tracker:%s", getTrackerRole(ucPart).c_str());
                            if (NULL == apstValidTracker[ucPart]) {
                                DriverLog("%s", "Failed to create tracker.", getTrackerRole(ucPart).c_str());
                                continue;
                            }
                            vr::VRServerDriverHost()->TrackedDeviceAdded(getTrackerRole(ucPart).c_str(),
                                vr::TrackedDeviceClass_GenericTracker, apstValidTracker[ucPart]);
                        }
                        if (NULL != apstValidTracker[ucPart]) {
                            apstValidTracker[ucPart]->UpdatePose(pstTagData);
                        }
                    }
                    //else {
                    //    if (invalidTagIndex < v_invalidTracker.size()) {
                    //        v_invalidTracker[ucPart]->UpdatePose(pstTagData);
                    //        ucPart++;
                    //    }
                    //    else {
                    //        CTracker* ptemp = new CTracker(ucPart);
                    //        if (NULL != ptemp) {
                    //            ptemp->UpdatePose(pstTagData);
                    //            v_invalidTracker.insert(v_invalidTracker.begin(), ptemp);
                    //            ucPart++;
                    //        }
                    //    }
                    //}
                }
                break;
            default:
                //DriverLog("Unrecognized msg type(%u) in Provider::RunFrame", pstHdr->uiType);
                break;
            }
        }
        else {
            DebugDriverLog("Received msg length length wrong(readsize:%d, uiLength:%u)!\n", readsize, pstHdr->uiLength);
        }
    }
    mutexUdpRW.unlock_shared();
}
void CProvider::EnterStandby()
{

}
void CProvider::LeaveStandby()
{

}
void CProvider::udpRW() {
    HMD_MSG_POSE_DATA_S stMsg{};
    sockaddr_in serverAddr{};
    const size_t msgsize = sizeof(HMD_MSG_POSE_DATA_S);
    serverAddr.sin_family = PF_INET;  //IPV4
    serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  //addr
    serverAddr.sin_port = htons(SERVER_PORT);  //Server端口号
    memset(aucRxBuffer, 0, RX_BUFFER_LENGTH);
    stMsg.stTlv.uiLength = sizeof(HMD_MSG_POSE_DATA_S);
    stMsg.stTlv.uiType = HMD_MSG_POSE_DATA;

    while (!b_exit) {
        mutexUdpRW.lock();
        readsize = CSocket::selectAndRecv(udpSocket, aucRxBuffer, RX_BUFFER_LENGTH);
        mutexUdpRW.unlock();
        updataHmdPose(&stMsg.stHmdPose);
        CSocket::sendto(udpSocket, (char*)&stMsg, msgsize, &serverAddr);
        Sleep(5);
    }
}
