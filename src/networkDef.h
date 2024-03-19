#pragma once
#define CYMPLE_STEAMVR_PORT 23304

typedef struct {
	uint16_t uiType;	/* DRIVER_MSG_XXXX */
	uint16_t uiLength;
}TLV_S;