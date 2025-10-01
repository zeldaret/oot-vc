#ifndef NW4R_SND_BANK_H
#define NW4R_SND_BANK_H
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/BankFile.h"
#include "revolution/hbm/nw4hbm/snd/NoteOnCallback.h"

namespace nw4hbm {
namespace snd {
namespace detail {

class Channel;

class Bank {
  public:
    explicit Bank(const void* bankData);
    ~Bank();

    Channel* NoteOn(const NoteOnInfo& noteOnInfo) const;

    void SetWaveDataAddress(const void* waveData) {
        NW4HBMAssertPointerNonnull_Line(waveData, 47);
        mWaveDataAddress = waveData;
    }

  private:
    /* 0x00 */ BankFileReader mBankReader;
    /* 0x0C */ const void* mWaveDataAddress;
};

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
