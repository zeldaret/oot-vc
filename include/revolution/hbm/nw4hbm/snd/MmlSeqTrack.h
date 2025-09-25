#ifndef NW4R_SND_MML_SEQ_TRACK_H
#define NW4R_SND_MML_SEQ_TRACK_H

#include "revolution/hbm/nw4hbm/snd/MmlParser.h"
#include "revolution/hbm/nw4hbm/snd/SeqTrack.h"

namespace nw4hbm {
namespace snd {
namespace detail {
class MmlSeqTrack : public SeqTrack {
  public:
    typedef struct MmlParserParam {
        bool cmpFlag; // 0x00
        bool noteWaitFlag; // 0x01
        bool tieFlag; // 0x02
        u8 loopCount[MmlParser::CALL_STACK_MAX_DEPTH]; // 0x03

        u8 callStackDepth; // 0x06
        const u8* callStack[MmlParser::CALL_STACK_MAX_DEPTH]; // 0x08
    } MmlParserParam;

  public:
    MmlSeqTrack();

    virtual ParseResult Parse(bool doNoteOn); // 0x0C

    void SetMmlParser(const MmlParser* pParser) { mParser = pParser; }
    MmlParserParam& GetMmlParserParam() { return mMmlParserParam; }

  private:
    const MmlParser* mParser; // 0xC0
    MmlParserParam mMmlParserParam; // 0xC4
};

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_MML_SEQ_TRACK_H
