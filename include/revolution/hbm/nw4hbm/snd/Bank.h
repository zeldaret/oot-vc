#ifndef NW4R_SND_BANK_H
#define NW4R_SND_BANK_H
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/BankFile.h"
#include "revolution/hbm/nw4hbm/snd/NoteOnCallback.h"

namespace nw4hbm {
namespace snd {
namespace detail {

// Forward declarations
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
    BankFileReader mBankReader; // 0x00
    const void* mWaveDataAddress; // 0x0C
};

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
