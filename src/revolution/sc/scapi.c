#include "revolution/sc.h"

s8 SCGetDisplayOffsetH(void) {
    s8 item;

    if (!SCFindS8Item(&item, SC_ITEM_IPL_DH)) {
        item = 0;
    } else if (item < -32) {
        item = -32;
    } else if (item > 32) {
        item = 32;
    }

    return item & ~1;
}

void SCGetIdleMode(SCIdleModeInfo* mode) { SCFindByteArrayItem(mode, sizeof(SCIdleModeInfo), SC_ITEM_IPL_IDL); }

u8 SCGetLanguage(void) {
    u8 item;

    if (!SCFindU8Item(&item, SC_ITEM_IPL_LNG)) {
        if (SCGetProductArea() == SC_AREA_JPN) {
            item = SC_LANG_JP;
        } else {
            item = SC_LANG_EN;
        }
    } else if (item > SC_LANG_KR) {
        item = SC_LANG_EN;
    }

    return item;
}

u8 SCGetProgressiveMode(void) {
    u8 item;

    if (!SCFindU8Item(&item, SC_ITEM_IPL_PGS)) {
        item = false;
    } else if (item != true) {
        item = false;
    }

    return item;
}

u8 SCGetScreenSaverMode(void) {
    u8 item;

    if (!SCFindU8Item(&item, SC_ITEM_IPL_SSV)) {
        item = true;
    } else if (item != true) {
        item = false;
    }

    return item;
}

u8 SCGetSoundMode(void) {
    u8 item;

    if (!SCFindU8Item(&item, SC_ITEM_IPL_SND)) {
        item = SC_SND_STEREO;
    } else if (item != SC_SND_MONO && item != SC_SND_STEREO && item != SC_SND_SURROUND) {
        item = SC_SND_STEREO;
    }

    return item;
}

void SCGetBtDeviceInfoArray(SCBtDeviceInfoArray* info) {
    SCFindByteArrayItem(info, sizeof(SCBtDeviceInfoArray), SC_ITEM_BT_DINF);
}

void SCSetBtDeviceInfoArray(const SCBtDeviceInfoArray* info) {
    SCReplaceByteArrayItem(info, sizeof(SCBtDeviceInfoArray), SC_ITEM_BT_DINF);
}

u32 SCGetBtDpdSensibility(void) {
    u32 item;

    if (!SCFindU32Item(&item, SC_ITEM_BT_SENS)) {
        item = 5;
    } else if (item < 1) {
        item = 1;
    } else if (item > 5) {
        item = 5;
    }

    return item;
}

u8 SCGetWpadMotorMode(void) {
    u8 item;

    if (!SCFindU8Item(&item, SC_ITEM_BT_MOT)) {
        item = true;
    } else if (item != true) {
        item = false;
    }

    return item;
}

void SCSetWpadMotorMode(u8 mode) { SCReplaceU8Item(mode, SC_ITEM_BT_MOT); }

u8 SCGetWpadSensorBarPosition(void) {
    u8 item;

    if (!SCFindU8Item(&item, SC_ITEM_BT_BAR)) {
        item = SC_SENSOR_BAR_BOTTOM;
    } else if (item != SC_SENSOR_BAR_TOP) {
        item = SC_SENSOR_BAR_BOTTOM;
    }

    return item;
}

u8 SCGetWpadSpeakerVolume(void) {
    u8 item;

    if (!SCFindU8Item(&item, SC_ITEM_BT_SPKV)) {
        item = 127;
    } else if (item > 127) {
        item = 127;
    }

    return item;
}

void SCSetWpadSpeakerVolume(u8 vol) { SCReplaceU8Item(vol, SC_ITEM_BT_SPKV); }
