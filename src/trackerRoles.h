#ifndef _TRACKERROLES_
#define _TRACKERROLES_

#include<iostream>

enum TrackerRole 
{
    ROLE_UNDEFINED = 0,
    ROLE_HEAD = 1,
    ROLE_CHEST = 2,
    ROLE_HIP = 3,
    ROLE_LEFT_KNEE = 4,
    ROLE_RIGHT_KNEE = 5,
    ROLE_LEFT_ELBOW = 6,
    ROLE_RIGHT_ELBOW = 7,
    ROLE_LEFT_SOLE = 8,
    ROLE_RIGHT_SOLE = 9,
    ROLE_LEFT_SHOULDER = 10,
    ROLE_RIGHT_SHOULDER = 11,
    ROLE_LEFT_HAND = 12,
    ROLE_RIGHT_HAND = 13,
    ROLE_MAX = 14,
    ROLE_INVALID = 0xFFFF,
};

//==================================================================
//函 数 名：getTrackerRole
//功能描述：获取对应vive角色
//输入参数：
//返 回 值：
//作    者：Dominocs
//日    期：2022/06/21
//修改记录：
//==================================================================
static std::string getTrackerRole(uint16_t role) {
    switch (role) {
    case TrackerRole::ROLE_HEAD:
        return "TrackerRole_Head";
    case TrackerRole::ROLE_CHEST:
        return "TrackerRole_Chest";
    case TrackerRole::ROLE_HIP:
        return "TrackerRole_Hip";
    case TrackerRole::ROLE_LEFT_KNEE:
        return "TrackerRole_LeftKnee";
    case TrackerRole::ROLE_RIGHT_KNEE:
        return "TrackerRole_RightKnee";
    case TrackerRole::ROLE_LEFT_ELBOW:
        return "TrackerRole_LeftElbow";
    case TrackerRole::ROLE_RIGHT_ELBOW:
        return "TrackerRole_RightElbow";
    case TrackerRole::ROLE_LEFT_SOLE:
        return "TrackerRole_LeftSole";
    case TrackerRole::ROLE_RIGHT_SOLE:
        return "TrackerRole_RightSole";
    case TrackerRole::ROLE_LEFT_SHOULDER:
        return "TrackerRole_LeftShoulder";
    case TrackerRole::ROLE_RIGHT_SHOULDER:
        return "TrackerRole_RightShoulder";
    case TrackerRole::ROLE_LEFT_HAND:
    case TrackerRole::ROLE_RIGHT_HAND:
        return "TrackerRole_Handed";

    }
    return "invalidRole";
}

#endif // !_TRACKERROLES_