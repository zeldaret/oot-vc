#ifndef NW4R_SND_CHANNEL_MANAGER_H
#define NW4R_SND_CHANNEL_MANAGER_H

#include "revolution/hbm/nw4hbm/snd/snd_Types.hpp"
#include "revolution/hbm/nw4hbm/snd/Channel.h"
#include "revolution/hbm/nw4hbm/snd/InstanceManager.h"

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
        }
    }
}

#endif // NW4R_SND_CHANNEL_MANAGER_H
