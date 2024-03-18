#ifndef HMDPOSE_HPP
#define HMDPOSE_HPP
#include "openvr_driver.h"
#include "driverProto.hpp"

static vr::HmdVector3_t GetHmdPosition(vr::HmdMatrix34_t& matrix)
{
    vr::HmdVector3_t vector;
    vector.v[0] = matrix.m[0][3];
    vector.v[1] = matrix.m[1][3];
    vector.v[2] = matrix.m[2][3];

    return vector;
}
static vr::HmdQuaternion_t GetHmdRotation(vr::HmdMatrix34_t& matrix) {
    vr::HmdQuaternion_t q;
    q.w = sqrt(fmax(0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2])) / 2;
    q.x = sqrt(fmax(0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2])) / 2;
    q.y = sqrt(fmax(0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2])) / 2;
    q.z = sqrt(fmax(0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2])) / 2;
    q.x = copysign(q.x, matrix.m[2][1] - matrix.m[1][2]);
    q.y = copysign(q.y, matrix.m[0][2] - matrix.m[2][0]);
    q.z = copysign(q.z, matrix.m[1][0] - matrix.m[0][1]);
    return q;
}

void updataHmdPose(TRACKER_POSE_S*pstMsg)
{
    vr::TrackedDevicePose_t hmd_pose;
    vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0, &hmd_pose, 1);
    vr::HmdQuaternion_t q = GetHmdRotation(hmd_pose.mDeviceToAbsoluteTracking);
    vr::HmdVector3_t pos = GetHmdPosition(hmd_pose.mDeviceToAbsoluteTracking);
    double* pdTmp = pstMsg->adPose;
    pdTmp[0] = pos.v[0];
    pdTmp[1] = pos.v[1];
    pdTmp[2] = pos.v[2];
    pdTmp = pstMsg->adQua;
    pdTmp[0] = q.w;
    pdTmp[1] = q.x;
    pdTmp[2] = q.y;
    pdTmp[3] = q.z;
}
#endif