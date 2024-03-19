//@Copyright:Copyright 2022 Dominocs
//@License:Apache-2.0
//@Birth:created by Dominocs on 2022-06-21
//@Content:驱动总入口
//@Version:0.1.1
//@Revision:
#include "driverlog.h"
#include "provider.h"

#include <string.h>

using namespace vr;

#if defined(_WIN32)
#define HMD_DLL_EXPORT extern "C" __declspec(dllexport)
#define HMD_DLL_IMPORT extern "C" __declspec(dllimport)
#elif defined(__GNUC__) || defined(COMPILER_GCC) || defined(__APPLE__)
#define HMD_DLL_EXPORT extern "C" __attribute__((visibility("default")))
#define HMD_DLL_IMPORT extern "C"
#else
#error "Unsupported Platform."
#endif

ProviderClass g_provider;

//==================================================================
//函 数 名：HmdDriverFactory
//功能描述：驱动注册入口，返回驱动提供器实例
//输入参数：onst char *pInterfaceName, int *pReturnCode)
//返 回 值：HMD_DLL_EXPORT
//作    者：Dominocs
//日    期：2022/06/21
//修改记录：
//==================================================================
HMD_DLL_EXPORT void *HmdDriverFactory(const char *pInterfaceName, int *pReturnCode)
{
    if (0 == strcmp(IServerTrackedDeviceProvider_Version, pInterfaceName)) {
        return &g_provider;
    }
    if (pReturnCode) {
        *pReturnCode = VRInitError_Init_InterfaceNotFound;
    }

    return NULL;
}

//==================================================================
//函 数 名：GetDriver
//功能描述：获取全局驱动provider
//输入参数：onst char *pInterfaceName, int *pReturnCode)
//返 回 值：HMD_DLL_EXPORT
//作    者：Dominocs
//日    期：2022/06/21
//修改记录：
//==================================================================
ProviderClass* GetDriverProvider()
{
    return &g_provider;
}
