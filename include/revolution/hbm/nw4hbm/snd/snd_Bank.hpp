#ifndef NW4R_SND_BANK_H
#define NW4R_SND_BANK_H
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/snd_BankFile.hpp"
#include "revolution/hbm/nw4hbm/snd/snd_NoteOnCallback.hpp"

namespace nw4hbm {
namespace snd {
namespace detail {

// Forward declarations
class Channel;

class Bank {
  public:
    explicit Bank(const void* pBankBin);
    ~Bank();

    Channel* NoteOn(const NoteOnInfo& rInfo) const;

    void SetWaveDataAddress(const void* pData) { mWaveDataAddress = pData; }

  private:
    BankFileReader mBankReader; // at 0x0
    const void* mWaveDataAddress; // at 0xC
};

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
