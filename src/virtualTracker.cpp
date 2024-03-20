#include <openvr_driver.h>
#include "basic.h"
#include "virtualTracker.h"
#include "driverlog.h"
using namespace vr;
template <typename T>
static void GetTrackerPosition(vr::HmdMatrix34_t& matrix, const T *TrackerOffset, T *out)
{
    for (size_t idx = 0; idx < 4; idx++) {
        out[idx] = TrackerOffset[0] * matrix.m[idx][0] + TrackerOffset[1] * matrix.m[idx][1]
            + TrackerOffset[2] * matrix.m[idx][2] + matrix.m[idx][3];
    }
}


static vr::HmdQuaternion_t GetTrackerRotation(vr::HmdMatrix34_t& matrix, double yaw, double pitch) {
    vr::HmdQuaternion_t q;
    //Convert yaw, pitch, roll to quaternion
    double cosp = cos(pitch);
    double sinp = sin(pitch);
    double cosh = cos(yaw);
    double sinh = sin(yaw);
    double trackerMatrix[3][3] = { {cosh, sinp * sinh, cosp * sinh},{0, cosp, -sinp}, {-sinh, sinp * cosh, cosp * cosh}};
    double outRotateMatrix[3][3] = {};
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            outRotateMatrix[row][col] = matrix.m[row][0] * trackerMatrix[0][col] + matrix.m[row][1] * trackerMatrix[1][col] + matrix.m[row][2] * trackerMatrix[2][col];
        }
    }
    //Set head tracking rotation
    q.w = sqrt(fmax(0, 1 + outRotateMatrix[0][0] + outRotateMatrix[1][1] + outRotateMatrix[2][2])) / 2;
    q.x = sqrt(fmax(0, 1 + outRotateMatrix[0][0] - outRotateMatrix[1][1] - outRotateMatrix[2][2])) / 2;
    q.y = sqrt(fmax(0, 1 - outRotateMatrix[0][0] + outRotateMatrix[1][1] - outRotateMatrix[2][2])) / 2;
    q.z = sqrt(fmax(0, 1 - outRotateMatrix[0][0] - outRotateMatrix[1][1] + outRotateMatrix[2][2])) / 2;
    q.x = copysign(q.x, outRotateMatrix[2][1] - outRotateMatrix[1][2]);
    q.y = copysign(q.y, outRotateMatrix[0][2] - outRotateMatrix[2][0]);
    q.z = copysign(q.z, outRotateMatrix[1][0] - outRotateMatrix[0][1]);
    return q;
}

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
    vr::VRProperties()->SetInt32Property(props, vr::Prop_SupportedButtons_Uint64, 1);
	// Set up a render model path
	vr::VRProperties()->SetStringProperty(props, vr::Prop_RenderModelName_String, "{indexcontroller}/rendermodels/valve_controller_knu_ev2_0_right");
	//	 Set the icon
    vr::VRDriverInput()->CreateBooleanComponent(props, "/input/trigger/click", &this->trigger_click_component_);
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
	this->device_index = k_unTrackedDeviceIndexInvalid;
	DriverLog("Tracker: Deactivate.\n");
}


void VirtualTracker::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
	if (unResponseBufferSize >= 1)
		pchResponseBuffer[0] = 0;
}

void VirtualTracker::updatePose() {

    //Simple change yaw, pitch, roll with numpad keys
    if ((GetAsyncKeyState('R') & 0x8000) != 0) {
        yaw = pitch = 0;
    }

    if ((GetAsyncKeyState('J') & 0x8000) != 0) {
        yaw += 0.01;
    }
    if ((GetAsyncKeyState('L') & 0x8000) != 0) {
        yaw += -0.01;
    }

    if ((GetAsyncKeyState('I') & 0x8000) != 0) {
        pitch += 0.01;
    }
    if ((GetAsyncKeyState('K') & 0x8000) != 0) {
        pitch += -0.01;
    }

    bool click = (0x8000 & GetAsyncKeyState('P'));
    vr::TrackedDevicePose_t hmd_pose;
    vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0, &hmd_pose, 1);
    double offset[4] = { -0.25, 0, -0.5, 1 };
    GetTrackerPosition(hmd_pose.mDeviceToAbsoluteTracking, offset,  pose.vecPosition);
    pose.qRotation = GetTrackerRotation(hmd_pose.mDeviceToAbsoluteTracking, yaw, pitch);
    //if (itemId == 5) {
    //    auto props = vr::VRProperties()->TrackedDeviceToPropertyContainer(this->device_index);
    //    if (click) {
    //        vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerRoleHint_Int32, vr::ETrackedControllerRole::TrackedControllerRole_RightHand);
    //    }
    //    else {
    //        vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerRoleHint_Int32, vr::ETrackedControllerRole::TrackedControllerRole_OptOut);
    //    }
    //}
        
    //vr::VRDriverInput()->UpdateBooleanComponent(trigger_click_component_, click, 0);
    //
    //pose.vecPosition[0] = pX;
    //pose.vecPosition[1] = pY;
    //pose.vecPosition[2] = pZ;

    //Convert yaw, pitch, roll to quaternion
    //t0 = cos(roll * 0.5);
    //t1 = sin(roll * 0.5);
    //t2 = cos(pitch * 0.5);
    //t3 = sin(pitch * 0.5);
    //t4 = cos(yaw * 0.5);
    //t5 = sin(yaw * 0.5);

    //Set head tracking rotation
    //double w = t0 * t2 * t4 + t1 * t3 * t5;
    //double x = t0 * t3 * t4 + t1 * t2 * t5;
    //double y = t0 * t2 * t5 - t1 * t3 * t4;
    //double z = t1 * t2 * t4 - t0 * t3 * t5;
    //double off_x = 0.2 * (2 * x * y - 2 * w * z) - 0.15 * (2 * x * z + 2 * w * y);
    //double off_y = 0.2 * (1 - 2 * x * x - 2 * z * z) - 0.15 * (2 * y * z - 2 * w * x);
    //double off_z = 0.2 * (2 * y * z + 2 * w * x) - 0.15 * (1 - 2 * x * x - 2 * y * y);
    //pose.qRotation.w = w;
    //pose.qRotation.x = x;
    //pose.qRotation.y = y;
    //pose.qRotation.z = z;
    //pose.vecPosition[0] += off_x;
    //pose.vecPosition[1] += off_y;
    //pose.vecPosition[2] += off_z;
	vr::VRServerDriverHost()->TrackedDevicePoseUpdated(device_index, pose, sizeof(DriverPose_t));
}