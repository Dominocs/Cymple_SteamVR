#include <openvr_driver.h>
#include "../basic.h"
#include "eyeTracker.h"
#include "../driverlog.h"
using namespace vr;
template <typename T>
static void GetTrackerPosition(HmdMatrix34_t& matrix, const T *TrackerOffset, T *out)
{
    for (size_t idx = 0; idx < 4; idx++) {
        out[idx] = TrackerOffset[0] * matrix.m[idx][0] + TrackerOffset[1] * matrix.m[idx][1]
            + TrackerOffset[2] * matrix.m[idx][2] + matrix.m[idx][3];
    }
}

static HmdQuaternion_t GetTrackerRotation(HmdMatrix34_t& matrix, double yaw, double pitch) {
    HmdQuaternion_t q;
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

eyeTrackerClass::eyeTrackerClass()
{
	pose.poseIsValid = true;
	pose.result = TrackingResult_Running_OK;
	pose.deviceIsConnected = true;
	pose.poseTimeOffset = 0;
	pose.qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0);
	pose.qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);
	pose.qRotation.w = 1;
	pose.qRotation.x = 0;
	pose.qRotation.y = 0;
	pose.qRotation.z = 0;
    DriverLog("Creating Cymple Eye Tracker!\n");
}

EVRInitError eyeTrackerClass::Activate(uint32_t unObjectId)
{
	this->device_index = unObjectId;
	auto props = VRProperties()->TrackedDeviceToPropertyContainer(unObjectId);
	// Set some universe ID (Must be 2 or higher)
	VRProperties()->SetUint64Property(props, Prop_CurrentUniverseId_Uint64, this->device_index);
	// Set up a model "number" (not needed but good to have)
	VRProperties()->SetStringProperty(props, Prop_ModelNumber_String, "Cymple Eye Tracker");
	// Opt out of hand selection
	VRProperties()->SetInt32Property(props, Prop_ControllerRoleHint_Int32, ETrackedControllerRole::TrackedControllerRole_RightHand);
	// Set up a render model path
	VRProperties()->SetStringProperty(props, Prop_RenderModelName_String, "{htc}/rendermodels/vr_tracker_vive_1_0");
	//	 Set the icon
    VRDriverInput()->CreateBooleanComponent(props, "/input/trigger/click", &this->trigger_click_component_);

	DriverLog("Tracker:activate.\n");
	return VRInitError_None;
}

void eyeTrackerClass::Deactivate()
{
	this->device_index = k_unTrackedDeviceIndexInvalid;
	DriverLog("Tracker: Deactivate.\n");
}


void eyeTrackerClass::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
	if (unResponseBufferSize >= 1)
		pchResponseBuffer[0] = 0;
}

void eyeTrackerClass::updateHmdPose() {
    VRServerDriverHost()->GetRawTrackedDevicePoses(0, &hmdPose, 1);
}

int eyeTrackerClass::update(MSG_OPENVR_EYE_EULER_S* pdata, uint32_t length) {
    if (length < sizeof(MSG_OPENVR_EYE_EULER_S)) {
        return length;
    }
    double offset[4] = { pdata->offset_X, pdata->offset_Y, pdata->offset_Z, 1 };
    GetTrackerPosition(hmdPose.mDeviceToAbsoluteTracking, offset,  pose.vecPosition);
    pose.qRotation = GetTrackerRotation(hmdPose.mDeviceToAbsoluteTracking, pdata->yaw, pdata->pitch);
    //if (currentBindController != pdata->bindControllerIdx) {
    //    /* 回退原绑定的controller */
    //    if (vr::k_unTrackedDeviceIndexInvalid == currentBindController) {
    //        auto props = VRProperties()->TrackedDeviceToPropertyContainer(pdata->bindControllerIdx);
    //        VRProperties()->SetInt32Property(props, Prop_ControllerRoleHint_Int32, TrackedControllerRole_OptOut);
    //    }
    //    else {
    //        auto props = VRProperties()->TrackedDeviceToPropertyContainer(currentBindController);
    //        VRProperties()->SetInt32Property(props, Prop_ControllerRoleHint_Int32, currentControllerRole);
    //    }
    //    /* 绑定新的role */
    //    auto props = VRProperties()->TrackedDeviceToPropertyContainer(device_index);
    //    VRProperties()->SetInt32Property(props, Prop_ControllerRoleHint_Int32, pdata->bindControllerRole);
    //    currentControllerRole = pdata->bindControllerRole;
    //    currentBindController = pdata->bindControllerIdx;
    //}
    bool click = (0x8000 & GetAsyncKeyState('P'));
    vr::VRDriverInput()->UpdateBooleanComponent(trigger_click_component_, click, 0);
    VRServerDriverHost()->TrackedDevicePoseUpdated(device_index, pose, sizeof(DriverPose_t));
    return sizeof(MSG_OPENVR_EYE_EULER_S);
}