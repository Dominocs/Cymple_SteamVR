#ifndef DRIVERPROTO_HPP
#define DRIVERPROTO_HPP
#include<iostream>

#define DRIVER_PORT 23300
#define SERVER_PORT 23301
enum {
    MSG_SERVER_HEARTBEAT_E,
    MSG_IMU_DATA_E,
    MSG_SERVER_IMU_CONFIG_E,
    MSG_IMU_LOG_E,
    HMD_MSG_POSE_DATA,
    MSG_SERVER_POSE2DRIVER_E,
};

typedef struct {
	uint16_t uiType;	/* DRIVER_MSG_XXXX */
	uint16_t uiLength;
}TLV_S;

typedef struct {
    double adQua[4];
    double adPose[3];
}TRACKER_POSE_S;
typedef struct {
    double adPose[3];
    double adQua[4];
    uint8_t ucPart;
    uint8_t bEnable;
    uint8_t aucReserved[2];
}TRACKER_INFO_S;
/* Õ∑œ‘–≈œ¢ */
typedef struct {
	TLV_S stTlv;
	TRACKER_POSE_S stHmdPose;
}HMD_MSG_POSE_DATA_S;

#endif // !DRIVERPROTO_HPP
