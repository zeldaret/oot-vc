#ifndef NW4R_SND_ADPCM_H
#define NW4R_SND_ADPCM_H
#include "revolution/types.h"

#include "revolution/ax.h"

namespace nw4hbm {
namespace snd {
namespace detail {

s16 DecodeDspAdpcm(AXPBADPCM* pAdpcm, u8 bits);

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
