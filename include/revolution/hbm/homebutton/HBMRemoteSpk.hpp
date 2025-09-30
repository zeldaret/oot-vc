#ifndef RVL_SDK_HBM_HOMEBUTTON_REMOTE_SPK_HPP
#define RVL_SDK_HBM_HOMEBUTTON_REMOTE_SPK_HPP

#include "revolution/arc/arc.h" // ARCHandle
#include "revolution/os/OSAlarm.h"
#include "revolution/types.h"
#include "revolution/wenc/wenc.h" // WENCInfo
#include "revolution/wpad/WPAD.h"

namespace homebutton {

class RemoteSpk {
    // nested types
  private:
    struct ChanInfo {
        /* 0x00 */ OSAlarm alarm;
        /* 0x30 */ WENCInfo wencinfo;
        /* 0x50 */ const s16* in_pcm;
        /* 0x54 */ int length;
        /* 0x58 */ int seId;
        /* 0x5C */ bool first;
        /* 0x5D */ s8 vol;
        /* 0x5E */ s8 cannotSendCnt;
        /* 1 byte padding */
        u16 pad; // is this padding??? it's not used
        /* 0x62 */ bool playReady;
        /* 5 bytes padding */
    }; // size 0x68

    // methods
  public:
    // cdtors
    RemoteSpk(void* spkSeBuf);
    /* virtual ~RemoteSpk(); */ // late declaration

    // methods
    bool isPlayReady(WPADChannel chan) const;
    bool isPlaying(WPADChannel chan) const;
    bool isPlayingId(WPADChannel chan, int seId) const;

    void GetPCMFromSeID(int in_ID, s16*& out_wave, int& out_length);
    void ClearPcm();
    void Start();
    void Stop();
    void Connect(WPADChannel chan);
    void Play(WPADChannel chan, int seID, s8 vol);

    // static methods
    static void SetInstance(RemoteSpk* pThis);
    static RemoteSpk* GetInstance();

  private:
    static void UpdateSpeaker(OSAlarm* alarm, OSContext* context);

    static void SpeakerOnCallback(WPADChannel chan, WPADResult result);
    static void DelaySpeakerOnCallback(OSAlarm* alarm, OSContext* context);

    static void SpeakerPlayCallback(WPADChannel chan, WPADResult result);
    static void DelaySpeakerPlayCallback(OSAlarm* alarm, OSContext* context);

    // members
  private:
    /* 0x000 */ ChanInfo info[WPAD_MAX_CONTROLLERS];
    /* 0x1A0 */ OSAlarm speakerAlarm;
    /* 0x1D0 */ ARCHandle handle;
    /* 0x1EC */ bool available;
    /* 3 bytes padding */
    /* vtable */ // size 0x004, offset 0x1f0
    /* 4 bytes padding */

    // late declarations
  public:
    virtual ~RemoteSpk();
}; // size 0x1f8
} // namespace homebutton

#endif // RVL_SDK_HBM_HOMEBUTTON_REMOTE_SPK_HPP
