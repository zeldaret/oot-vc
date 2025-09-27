#ifndef NW4R_SND_CHANNEL_MANAGER_H
#define NW4R_SND_CHANNEL_MANAGER_H

#include "revolution/hbm/nw4hbm/snd/Channel.h"
#include "revolution/hbm/nw4hbm/snd/InstanceManager.h"
#include "revolution/hbm/nw4hbm/snd/types.h"

namespace nw4hbm {
namespace snd {
namespace detail {
class ChannelManager : public InstanceManager<Channel, 0> {
  public:
    static ChannelManager& GetInstance() {
        static ChannelManager instance;
        return instance;
    }
    ChannelManager() {}
};
} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_CHANNEL_MANAGER_H
