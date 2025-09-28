#ifndef RVL_SDK_HBM_HOMEBUTTON_AX_SOUND_HPP
#define RVL_SDK_HBM_HOMEBUTTON_AX_SOUND_HPP

#include "revolution/hbm/nw4hbm/snd/mix.h" // HBMMIXSoundMode
#include "revolution/types.h"

namespace homebutton {
void PlaySeq(int num);
void InitAxSound(const void* soundData, void* mem, u32 memSize);
void ShutdownAxSound();
void AxSoundMain();
void StopAllSeq();
void SetVolumeAllSeq(f32 gain);
void SetSoundMode(HBMMIXSoundMode mode);
} // namespace homebutton

#endif // RVL_SDK_HBM_HOMEBUTTON_AX_SOUND_HPP
