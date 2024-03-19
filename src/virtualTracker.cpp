#include <openvr_driver.h>
#include "basic.h"
#include "virtualTracker.h"
#include "driverlog.h"
using namespace vr;
static double yaw = 0, pitch = -3.1415926 / 2.0, roll = 0;
static double pX = 0, pY = 1.55, pZ = 0;
static double t0, t1, t2, t3, t4, t5;
VirtualTracker::VirtualTracker(uint32_t itemId, uint32_t controllerId)
{
    this->itemId = itemId;
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

EVRInitError VirtualTracker::Activate(uint32_t unObjectId)
{
	this->device_index = unObjectId;
	auto props = vr::VRProperties()->TrackedDeviceToPropertyContainer(unObjectId);
	// Set some universe ID (Must be 2 or higher)
	vr::VRProperties()->SetUint64Property(props, vr::Prop_CurrentUniverseId_Uint64, this->itemId);
	// Set up a model "number" (not needed but good to have)
	vr::VRProperties()->SetStringProperty(props, vr::Prop_ModelNumber_String, "Test Tracker");
	// Opt out of hand selection
	vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerRoleHint_Int32, this->controllerId);
	vr::VRProperties()->SetInt32Property(props, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_Controller);
	vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerHandSelectionPriority_Int32, 1); 
    vr::VRProperties()->SetUint64Property(props, vr::Prop_SupportedButtons_Uint64, 1); 
    vr::VRProperties()->SetBoolProperty(props, vr::Prop_HasControllerComponent_Bool, true);
	// Set up a render model path
	vr::VRProperties()->SetStringProperty(props, vr::Prop_RenderModelName_String, "{indexcontroller}/rendermodels/valve_controller_knu_ev2_0_right");
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

void VirtualTracker::Deactivate()
{
	//this->device_index = k_unTrackedDeviceIndexInvalid;
	//DriverLog("Tracker: Deactivate.\n");
}


void VirtualTracker::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
	//if (unResponseBufferSize >= 1)
	//	pchResponseBuffer[0] = 0;
}

void VirtualTracker::updatePose() {
    if (itemId != 6) {
        return;
    }
    vr::DriverPose_t pose = { 0 };
    pose.poseIsValid = true;
    pose.result = vr::TrackingResult_Running_OK;
    pose.deviceIsConnected = true;

    pose.qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0);
    pose.qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);

    //Simple change yaw, pitch, roll with numpad keys
    if ((GetAsyncKeyState('R') & 0x8000) != 0) {
        roll = pitch = yaw = 0;
        pX = 0;
        pZ = -0.5;
        pY = 1.55;
    }
    if ((GetAsyncKeyState('H') & 0x8000) != 0) {
        yaw += 0.01;
    }
    if ((GetAsyncKeyState('K') & 0x8000) != 0) {
        yaw += -0.01;
    }
    if ((GetAsyncKeyState('U') & 0x8000) != 0) {
        pitch += 0.01;
    }
    if ((GetAsyncKeyState('J') & 0x8000) != 0) {
        pitch += -0.01;
    }

    if ((GetAsyncKeyState('Y') & 0x8000) != 0) {
        roll += -0.01;
    }
    if ((GetAsyncKeyState('D') & 0x8000) != 0) {
        pX += 0.01;
    }
    if ((GetAsyncKeyState('A') & 0x8000) != 0) {
        pX += -0.01;
    }
    if ((GetAsyncKeyState('S') & 0x8000) != 0) {
        pZ += 0.01;
    }
    if ((GetAsyncKeyState('W') & 0x8000) != 0) {
        pZ += -0.01;
    }
    if ((GetAsyncKeyState('E') & 0x8000) != 0) {
        pY += 0.01;
    }
    if ((GetAsyncKeyState('Q') & 0x8000) != 0) {
        pY += -0.01;
    }

    pose.vecPosition[0] = pX;
    pose.vecPosition[1] = pY;
    pose.vecPosition[2] = pZ;

    //Convert yaw, pitch, roll to quaternion
    t0 = cos(roll * 0.5);
    t1 = sin(roll * 0.5);
    t2 = cos(pitch * 0.5);
    t3 = sin(pitch * 0.5);
    t4 = cos(yaw * 0.5);
    t5 = sin(yaw * 0.5);

    //Set head tracking rotation
    double w = t0 * t2 * t4 + t1 * t3 * t5;
    double x = t0 * t3 * t4 + t1 * t2 * t5;
    double y = t0 * t2 * t5 - t1 * t3 * t4;
    double z = t1 * t2 * t4 - t0 * t3 * t5;
    double off_x = 0.2 * (2 * x * y - 2 * w * z) - 0.15 * (2 * x * z + 2 * w * y);
    double off_y = 0.2 * (1 - 2 * x * x - 2 * z * z) - 0.15 * (2 * y * z - 2 * w * x);
    double off_z = 0.2 * (2 * y * z + 2 * w * x) - 0.15 * (1 - 2 * x * x - 2 * y * y);
    pose.qRotation.w = w;
    pose.qRotation.x = x;
    pose.qRotation.y = y;
    pose.qRotation.z = z;
    pose.vecPosition[0] += off_x;
    pose.vecPosition[1] += off_y;
    pose.vecPosition[2] += off_z;
	vr::VRServerDriverHost()->TrackedDevicePoseUpdated(device_index, pose, sizeof(DriverPose_t));
}