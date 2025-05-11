#include "revolution/hbm/homebutton/HBMController.hpp"

/*******************************************************************************
 * headers
 */

#include "macros.h"
#include "revolution/hbm/homebutton/HBMRemoteSpk.hpp"
#include "revolution/kpad/KPAD.h"
#include "revolution/os/OSTime.h"
#include "revolution/types.h"
#include "revolution/wpad/WPAD.h"

/*******************************************************************************
 * variables
 */

namespace homebutton {
// .bss
bool Controller::sBatteryFlag[WPAD_MAX_CONTROLLERS];
OSAlarm Controller::sAlarm[WPAD_MAX_CONTROLLERS];
OSAlarm Controller::sAlarmSoundOff[WPAD_MAX_CONTROLLERS];
Controller* Controller::sThis[WPAD_MAX_CONTROLLERS];
bool Controller::sSetInfoAsync[WPAD_MAX_CONTROLLERS];
RemoteSpk* Controller::sPInstance;
s32 Controller::lbl_8025DBBC;
} // namespace homebutton

/*******************************************************************************
 * functions
 */

namespace homebutton {

void Controller::wpadConnectCallback(WPADChannel chan, WPADResult result) {
    switch (result) {
        case WPAD_ESUCCESS:
            if (!sThis[chan]->mCallbackFlag) {
                sThis[chan]->mOldExtensionCallback = WPADSetExtensionCallback(chan, &wpadExtensionCallback);
                sThis[chan]->mCallbackFlag = true;
            }

            WPADControlSpeaker(chan, WPAD_SPEAKER_OFF, nullptr);
            break;

        case WPAD_ENODEV:
            WPADSetExtensionCallback(chan, sThis[chan]->mOldExtensionCallback);
            sThis[chan]->mOldExtensionCallback = nullptr;
            sThis[chan]->mCallbackFlag = false;
            sThis[chan]->mCheckSoundTimeFlag = false;
            sThis[chan]->mCheckSoundIntervalFlag = false;
            break;
    }

    if (sThis[chan]->mOldConnectCallback) {
        (*sThis[chan]->mOldConnectCallback)(chan, result);
    }
}

void Controller::wpadExtensionCallback(WPADChannel chan, s32 result) {
    switch (result) {
        case WPAD_DEV_INITIALIZING:
            sThis[chan]->soundOff(1000);
            break;
    }

    if (sThis[chan]->mOldExtensionCallback) {
        (*sThis[chan]->mOldExtensionCallback)(chan, result);
    }
}

void Controller::soundOnCallback(OSAlarm* alm, OSContext*) {
    int chan = OSGetAlarmUserDataAny(int, alm);
    sThis[chan]->soundOn();
}

Controller::Controller(int chan, RemoteSpk* spk) {
    mHBController.chan = chan;
    mHBController.rumble = false;
    mHBController.spVol = 1.0f;

    remotespk = spk;
    mOldConnectCallback = nullptr;
    mOldExtensionCallback = nullptr;
    mCallbackFlag = false;
    mSoundOffFlag = false;

    if (chan < WPAD_MAX_CONTROLLERS) {
        sBatteryFlag[chan] = false;
        OSCreateAlarm(&sAlarm[chan]);
        OSCreateAlarm(&sAlarmSoundOff[chan]);
        sThis[chan] = this;
    }
}

Controller::~Controller() {
    OSCancelAlarm(&sAlarm[mHBController.chan]);
    OSCancelAlarm(&sAlarmSoundOff[mHBController.chan]);
}

void Controller::initCallback() {
    WPADDeviceType type;

    mOldConnectCallback = WPADSetConnectCallback(mHBController.chan, &wpadConnectCallback);

    switch (WPADProbe(mHBController.chan, &type)) {
        case WPAD_ESUCCESS:
            mOldExtensionCallback = WPADSetExtensionCallback(mHBController.chan, &wpadExtensionCallback);
            mCallbackFlag = true;
            break;
        case WPAD_ENODEV:
            mCallbackFlag = false;
            break;
    }
}

void Controller::clearCallback() {
    WPADSetConnectCallback(mHBController.chan, mOldConnectCallback);
    mOldConnectCallback = nullptr;

    WPADSetExtensionCallback(mHBController.chan, mOldExtensionCallback);
    mOldExtensionCallback = nullptr;
}

void Controller::setKpad(const HBMKPadData* con, bool updatePos) {
    if (!con->kpad) {
        return;
    }

    if (updatePos) {
        if (con->kpad->dev_type == WPAD_DEV_CLASSIC && con->use_devtype == WPAD_DEV_CLASSIC) {
            mHBController.x = con->pos.x;
            mHBController.y = con->pos.y;
        } else {
            mHBController.x = con->kpad->pos.x;
            mHBController.y = con->kpad->pos.y;
        }
    }

    mHBController.trig = con->kpad->trig;
    mHBController.hold = con->kpad->hold;
    mHBController.release = con->kpad->release;

    if (con->kpad->dev_type == WPAD_DEV_CLASSIC) {
        u32 h = con->kpad->ex_status.cl.hold;
        u32 t = con->kpad->ex_status.cl.trig;
        u32 r = con->kpad->ex_status.cl.release;

        if (h & WPAD_BUTTON_CL_A) {
            mHBController.hold |= WPAD_BUTTON_A;
        }
        if (t & WPAD_BUTTON_CL_A) {
            mHBController.trig |= WPAD_BUTTON_A;
        }
        if (r & WPAD_BUTTON_CL_A) {
            mHBController.release |= WPAD_BUTTON_A;
        }

        if (h & WPAD_BUTTON_CL_PLUS) {
            mHBController.hold |= WPAD_BUTTON_PLUS;
        }
        if (t & WPAD_BUTTON_CL_PLUS) {
            mHBController.trig |= WPAD_BUTTON_PLUS;
        }
        if (r & WPAD_BUTTON_CL_PLUS) {
            mHBController.release |= WPAD_BUTTON_PLUS;
        }

        if (h & WPAD_BUTTON_CL_MINUS) {
            mHBController.hold |= WPAD_BUTTON_MINUS;
        }
        if (t & WPAD_BUTTON_CL_MINUS) {
            mHBController.trig |= WPAD_BUTTON_MINUS;
        }
        if (r & WPAD_BUTTON_CL_MINUS) {
            mHBController.release |= WPAD_BUTTON_MINUS;
        }

        if (h & WPAD_BUTTON_CL_HOME) {
            mHBController.hold |= WPAD_BUTTON_HOME;
        }
        if (t & WPAD_BUTTON_CL_HOME) {
            mHBController.trig |= WPAD_BUTTON_HOME;
        }
        if (r & WPAD_BUTTON_CL_HOME) {
            mHBController.release |= WPAD_BUTTON_HOME;
        }
    }
}

void Controller::clrKpadButton() {
    mHBController.trig = 0;
    mHBController.hold = 0;
    mHBController.release = 0;
}

void Controller::setInValidPos() {
    mHBController.x = -10000.0f;
    mHBController.y = -10000.0f;
}

int Controller::getChan() const { return mHBController.chan; }

void Controller::connect() { getRemoteSpk()->Connect(getChan()); }

void Controller::disconnect() { /* ... */ }

void Controller::setSpeakerVol(f32 vol) { mHBController.spVol = vol; }

f32 Controller::getSpeakerVol() const { return mHBController.spVol; }

void Controller::playSound(int arg1, int id) {
    if (!mSoundOffFlag) {
        getRemoteSpk()->Play(getChan(), id, getSpeakerVol() * 10.0f);

        if (WPADIsSpeakerEnabled(getChan())) {
            if (!mCheckSoundTimeFlag) {
                mPlaySoundTime = OSGetTime();
            }

            mCheckSoundTimeFlag = true;
            mCheckSoundIntervalFlag = false;
        }
    }
}

bool Controller::isPlayingSound() const { return getRemoteSpk()->isPlaying(getChan()); }

bool Controller::isPlayingSoundId(int id) const {
    if (!isPlayingSound()) {
        return false;
    }

    if (!getRemoteSpk()->isPlayingId(getChan(), id)) {
        return false;
    }

    return true;
}

void Controller::initSound() {
    mCheckSoundTimeFlag = false;
    mCheckSoundIntervalFlag = false;
}

void Controller::updateSound() {
    int chan = getChan();

    if (!isPlayingSound()) {
        if (mCheckSoundTimeFlag) {
            if (!mCheckSoundIntervalFlag) {
                mStopSoundTime = OSGetTime();
                mCheckSoundIntervalFlag = true;
            } else {
                OSTime time = OSGetTime();
                if (OSTicksToMilliseconds(OSDiffTick(time, mStopSoundTime)) >= 1000) {
                    mCheckSoundTimeFlag = false;
                    mCheckSoundIntervalFlag = false;
                }
            }
        }

        return;
    } else {
        if (mCheckSoundTimeFlag) {
            mCheckSoundIntervalFlag = false;

            OSTime time = OSGetTime();
            if (OSTicksToMilliseconds(OSDiffTick(time, mPlaySoundTime)) >= 480000) {
                mCheckSoundTimeFlag = false;
                mCheckSoundIntervalFlag = false;
                soundOff(1000);
                return;
            }
        }

        // Average radio sensitivity is 80 (see __wpadCalcRadioQuality)
        if (!mSoundOffFlag && WPADGetRadioSensitivity(chan) <= 85) {
            soundOff(1000);
        }
    }
}

void Controller::soundOff(int msec) {
    int chan = getChan();

    if (!WPADIsSpeakerEnabled(chan)) {
        return;
    }

    WPADControlSpeaker(chan, WPAD_SPEAKER_MUTE, nullptr);

    OSSetAlarmUserDataAny(&sAlarmSoundOff[chan], chan);
    OSCancelAlarm(&sAlarmSoundOff[chan]);
    OSSetAlarm(&sAlarmSoundOff[chan], OSMillisecondsToTicks(msec), &soundOnCallback);

    mSoundOffFlag = true;
}

void Controller::soundOn() {
    int chan = getChan();

    if (WPADIsSpeakerEnabled(chan)) {
        WPADControlSpeaker(chan, WPAD_SPEAKER_UNMUTE, nullptr);
    }

    mSoundOffFlag = false;
}

bool Controller::isPlayReady() const { return getRemoteSpk()->isPlayReady(getChan()); }

HBController* Controller::getController() { return &mHBController; }

void Controller::startMotor() {
    if (getChan() < WPAD_MAX_CONTROLLERS && !isPlayingSound()) {
        setRumble();
        WPADControlMotor(getChan(), WPAD_MOTOR_RUMBLE);
    }
}

void Controller::stopMotor() {
    if (getChan() < WPAD_MAX_CONTROLLERS && isRumbling()) {
        clrRumble();
        WPADControlMotor(getChan(), WPAD_MOTOR_STOP);
    }
}

/* not sure what the return value here is? doesn't seem like WPADResult and
 * it's never used
 */
s32 Controller::getInfoAsync(WPADInfo* info) {
    if (getChan() >= WPAD_MAX_CONTROLLERS) {
        return -2;
    }

    if (isPlayingSound() || isRumbling()) {
        return -2;
    }

    return WPADGetInfoAsync(getChan(), info, &ControllerCallback);
}

void Controller::ControllerCallback(WPADChannel chan, WPADResult result) {
    if (result == WPAD_ESUCCESS && chan < WPAD_MAX_CONTROLLERS) {
        sBatteryFlag[chan] = true;
    }
}

bool Controller::getBatteryFlag() const {
    if (getChan() >= WPAD_MAX_CONTROLLERS) {
        return false;
    }

    return sBatteryFlag[getChan()];
}

void Controller::clrBatteryFlag() {
    if (getChan() >= WPAD_MAX_CONTROLLERS) {
        return;
    }

    sBatteryFlag[getChan()] = false;
}

} // namespace homebutton
