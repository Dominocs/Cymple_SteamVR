#include <openvr_driver.h>
#include "../basic.h"
#include "DebugController.h"
#include "../driverlog.h"
using namespace vr;
template <typename T>
static void GetTrackerPosition(vr::HmdMatrix34_t& matrix, const T* TrackerOffset, T* out)
{
    for (size_t idx = 0; idx < 4; idx++) {
        out[idx] = TrackerOffset[0] * matrix.m[idx][0] + TrackerOffset[1] * matrix.m[idx][1]
            + TrackerOffset[2] * matrix.m[idx][2] + matrix.m[idx][3];
    }
}


static vr::HmdQuaternion_t GetTrackerRotation(vr::HmdMatrix34_t& matrix) {
    vr::HmdQuaternion_t q;
    //Set head tracking rotation
    q.w = sqrt(fmax(0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2])) / 2;
    q.x = sqrt(fmax(0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2])) / 2;
    q.y = sqrt(fmax(0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2])) / 2;
    q.z = sqrt(fmax(0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2])) / 2;
    q.x = copysign(q.x, matrix.m[2][1] - matrix.m[1][2]);
    q.y = copysign(q.y, matrix.m[0][2] - matrix.m[2][0]);
    q.z = copysign(q.z, matrix.m[1][0] - matrix.m[0][1]);
    return q;
}

DebugControllerClass::DebugControllerClass(uint32_t controllerId)
{
    this->controllerId = controllerId;
    pose.poseIsValid = true;
    pose.result = vr::TrackingResult_Running_OK;
    pose.deviceIsConnected = true;
    pose.poseTimeOffset = 0;
    pose.qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0);
    pose.qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);
    pose.qRotation.w = 1;
    pose.qRotation.x = 0;
    pose.qRotation.y = 0;
    pose.qRotation.z = 0;
}

EVRInitError DebugControllerClass::Activate(uint32_t unObjectId)
{
    this->device_index = unObjectId;
    auto props = vr::VRProperties()->TrackedDeviceToPropertyContainer(unObjectId);
    // Set some universe ID (Must be 2 or higher)
    vr::VRProperties()->SetUint64Property(props, vr::Prop_CurrentUniverseId_Uint64, this->device_index);
    // Set up a model "number" (not needed but good to have)
    vr::VRProperties()->SetStringProperty(props, vr::Prop_ModelNumber_String, "Test Controller");
    // Opt out of hand selection
    vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerRoleHint_Int32, this->controllerId);
    vr::VRProperties()->SetInt32Property(props, vr::Prop_SupportedButtons_Uint64, 1);
    // Set up a render model path
    vr::VRProperties()->SetStringProperty(props, vr::Prop_RenderModelName_String, "{indexcontroller}/rendermodels/valve_controller_knu_ev2_0_right");

    vr::VRDriverInput()->CreateBooleanComponent(props, "/input/trigger/click", &this->trigger_click_component_);
    //	 Set the icon
    //vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceOff_String, "{lightvr_release}/icons/test.png");
    //vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceReady_String, "{lightvr_release}/icons/test.png");
    //vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceReadyAlert_String, "{lightvr_release}/icons/test.png");
    //vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceNotReady_String, "{lightvr_release}/icons/test.png");
    //vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceStandby_String, "{lightvr_release}/icons/test.png");
    //vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceAlertLow_String, "{lightvr_release}/icons/test.png");

    DriverLog("Tracker:activate.\n");
    return VRInitError_None;
}

void DebugControllerClass::Deactivate()
{
    this->device_index = k_unTrackedDeviceIndexInvalid;
    DriverLog("Tracker: Deactivate.\n");
}


void DebugControllerClass::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
    if (unResponseBufferSize >= 1)
        pchResponseBuffer[0] = 0;
}

void DebugControllerClass::RunFrame() {
    //Simple change yaw, pitch, roll with numpad keys
    bool click = (0x8000 & GetAsyncKeyState('P'));
    vr::VRDriverInput()->UpdateBooleanComponent(trigger_click_component_, click, 0);
    vr::TrackedDevicePose_t hmd_pose;
    vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0, &hmd_pose, 1);
    double offset[4] = { 0, -0.1, -0.2};
    GetTrackerPosition(hmd_pose.mDeviceToAbsoluteTracking, offset, pose.vecPosition);
    pose.qRotation = GetTrackerRotation(hmd_pose.mDeviceToAbsoluteTracking);
    vr::VRServerDriverHost()->TrackedDevicePoseUpdated(device_index, pose, sizeof(DriverPose_t));
}