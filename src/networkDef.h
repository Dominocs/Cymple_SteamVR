#pragma once
#define CYMPLE_OPENVR_PORT 23304

enum {
    MSG_OPENVR_EYE_EULER_E,
    MSG_OPENVR_MAX_E
};
enum {
    EULER_IDX_NONE_E,
    EULER_IDX_LEFT_HAND_E,
    EULER_IDX_RIGHT_HAND_E
};
typedef struct {
    uint16_t uiType;	/* DRIVER_MSG_XXXX */
    uint16_t uiLength;
}TLV_S;

typedef struct {
    TLV_S stTlv;
    float pitch;
    float yaw;
    float offset_X;
    float offset_Y;
    float offset_Z;
    uint32_t bindControllerIdx;
    uint8_t bindControllerRole;
    uint8_t bTrigger;
}MSG_OPENVR_EYE_EULER_S;
