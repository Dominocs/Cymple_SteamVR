//@Copyright:Copyright 2022 Dominocs
//@License:Apache-2.0
//@Birth:created by Dominocs on 2022-06-21
//@Content:���������
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
//�� �� ����HmdDriverFactory
//��������������ע����ڣ����������ṩ��ʵ��
//���������onst char *pInterfaceName, int *pReturnCode)
//�� �� ֵ��HMD_DLL_EXPORT
//��    �ߣ�Dominocs
//��    �ڣ�2022/06/21
//�޸ļ�¼��
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
//�� �� ����GetDriver
//������������ȡȫ������provider
//���������onst char *pInterfaceName, int *pReturnCode)
//�� �� ֵ��HMD_DLL_EXPORT
//��    �ߣ�Dominocs
//��    �ڣ�2022/06/21
//�޸ļ�¼��
//==================================================================
ProviderClass* GetDriverProvider()
{
    return &g_provider;
}
