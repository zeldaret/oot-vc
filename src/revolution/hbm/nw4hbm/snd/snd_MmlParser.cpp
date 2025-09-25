#include "revolution/hbm/nw4hbm/snd/MmlParser.h"
#include "revolution/hbm/nw4hbm/snd/MmlSeqTrack.h"
#include "revolution/hbm/nw4hbm/snd/SeqPlayer.h"
#include "revolution/hbm/nw4hbm/snd/Util.h" // Util::CalcRandom

namespace nw4hbm {
namespace snd {
namespace detail {

bool MmlParser::mPrintVarEnabledFlag = false;

ParseResult MmlParser::Parse(MmlSeqTrack* track, bool doNoteOn) const {
    NW4HBMAssertPointerNonnull_Line(track, 49);

    SeqPlayer* player = track->GetSeqPlayer();
    NW4HBMAssertPointerNonnull_Line(player, 51);

    SeqTrack::ParserTrackParam& rTrackParam = track->GetParserTrackParam();
    MmlSeqTrack::MmlParserParam& rMmlParam = track->GetMmlParserParam();

    SeqPlayer::ParserPlayerParam& rPlayerParam = player->GetParserPlayerParam();

    SeqArgType argType;
    bool useArgType = false;
    bool doExecCommand = true;

    u32 cmd = ReadByte(&rTrackParam.currentAddr);

    if (cmd == MML_EXECIF) {
        cmd = ReadByte(&rTrackParam.currentAddr);
        doExecCommand = rMmlParam.cmpFlag != false;
    }

    if (cmd == MML_RNDPARAM) {
        cmd = ReadByte(&rTrackParam.currentAddr);
        argType = SEQ_ARG_RANDOM;
        useArgType = true;
    }

    if (cmd == MML_INDPARAM) {
        cmd = ReadByte(&rTrackParam.currentAddr);
        argType = SEQ_ARG_VARIABLE;
        useArgType = true;
    }

    // MML note data, not a command
    if (!(cmd & MML_CMD_MASK)) {
        u8 velocity = ReadByte(&rTrackParam.currentAddr);

        s32 length = ReadArg(&rTrackParam.currentAddr, player, track, useArgType ? argType : SEQ_ARG_VMIDI);

        int key = cmd + rTrackParam.transpose;

        if (!doExecCommand) {
            return PARSE_RESULT_CONTINUE;
        }

        key = ut::Clamp(key, 0, 127);

        if (!rTrackParam.muteFlag && doNoteOn) {
            NoteOnCommandProc(track, key, velocity, length > 0 ? length : -1, rMmlParam.tieFlag);
        }

        if (rMmlParam.noteWaitFlag) {
            rTrackParam.wait = length;

            if (length == 0) {
                rTrackParam.noteFinishWait = true;
            }
        }
    }
    // MML command
    else {
        s32 arg1 = 0;
        s32 arg2 = 0;

        switch (cmd & MML_CMD_SET_MASK) {
            case 0x80: {
                switch (cmd) {
                    case MML_WAIT: {
                        s32 time =
                            ReadArg(&rTrackParam.currentAddr, player, track, useArgType ? argType : SEQ_ARG_VMIDI);

                        if (doExecCommand) {
                            rTrackParam.wait = time;
                        }
                        break;
                    }

                    case MML_SET_PRGNO: {
                        arg1 = ReadArg(&rTrackParam.currentAddr, player, track, useArgType ? argType : SEQ_ARG_VMIDI);

                        if (doExecCommand) {
                            CommandProc(track, cmd, arg1, arg2);
                        }
                        break;
                    }

                    case MML_OPENTRACK: {
                        u8 trackNo = ReadByte(&rTrackParam.currentAddr);
                        u32 offset = Read24(&rTrackParam.currentAddr);

                        if (doExecCommand) {
                            arg1 = trackNo;
                            arg2 = offset;
                            CommandProc(track, cmd, arg1, arg2);
                        }
                        break;
                    }

                    case MML_JUMP: {
                        u32 offset = Read24(&rTrackParam.currentAddr);

                        if (doExecCommand) {
                            arg1 = offset;
                            CommandProc(track, cmd, arg1, arg2);
                        }
                        break;
                    }

                    case MML_CALL: {
                        u32 offset = Read24(&rTrackParam.currentAddr);

                        if (doExecCommand) {
                            arg1 = offset;
                            CommandProc(track, cmd, arg1, arg2);
                        }
                        break;
                    }
                }
                break;
            }
            case 0xC0:
            case 0xD0: {
                u8 arg = ReadArg(&rTrackParam.currentAddr, player, track, useArgType ? argType : SEQ_ARG_U8);

                if (!doExecCommand) {
                    break;
                }

                switch (cmd) {
                    case MML_SET_TRANSPOSE:
                    case MML_SET_PITCHBEND: {
                        arg1 = *reinterpret_cast<s8*>(&arg);
                        break;
                    }
                    default: {
                        arg1 = arg;
                        break;
                    }
                }

                CommandProc(track, cmd, arg1, arg2);
                break;
            }

            case 0xB0:
            case 0x90: {
                if (doExecCommand) {
                    CommandProc(track, cmd, arg1, arg2);
                }
                break;
            }

            case 0xE0: {
                arg1 = static_cast<s16>(
                    ReadArg(&rTrackParam.currentAddr, player, track, useArgType ? argType : SEQ_ARG_S16));

                if (doExecCommand) {
                    CommandProc(track, cmd, arg1, arg2);
                }
                break;
            }

            case 0xF0: {
                switch (cmd) {
                    case MML_ALLOCTRACK: {
                        // Read16(&rTrackParam.currentAddr);
                        NW4HBMPanicMessage_Line(261, "seq: must use alloctrack in startup code");
                        break;
                    }

                    case MML_EOF: {
                        return PARSE_RESULT_FINISH;
                    }

                    case MML_EX_COMMAND: {
                        u32 cmdex = ReadByte(&rTrackParam.currentAddr);

                        switch (cmdex & MML_CMD_SET_MASK) {
                            case MML_EX_USERPROC: {
                                arg1 = static_cast<s16>(ReadArg(&rTrackParam.currentAddr, player, track,
                                                                useArgType ? argType : SEQ_ARG_S16));

                                if (doExecCommand) {
                                    CommandProc(track, (cmd << 8) + cmdex, arg1, arg2);
                                }
                                break;
                            }

                            case MML_EX_ARITHMETIC:
                            case MML_EX_LOGIC: {
                                arg1 = ReadByte(&rTrackParam.currentAddr);
                                arg2 = static_cast<s16>(ReadArg(&rTrackParam.currentAddr, player, track,
                                                                useArgType ? argType : SEQ_ARG_S16));

                                if (doExecCommand) {
                                    CommandProc(track, (cmd << 8) + cmdex, arg1, arg2);
                                }
                                break;
                            }
                        }

                        // FALLTHROUGH (assume no arguments?)
                    }

                    default: {
                        if (doExecCommand) {
                            CommandProc(track, cmd, arg1, arg2);
                        }
                        break;
                    }
                }

                break;
            }
        }
    }

    return PARSE_RESULT_CONTINUE;
}

void MmlParser::CommandProc(MmlSeqTrack* track, u32 command, s32 varNo, s32 arg2) const {
    NW4HBMAssertPointerNonnull_Line(track, 354);

    SeqPlayer* player = track->GetSeqPlayer();
    NW4HBMAssertPointerNonnull_Line(player, 356);

    SeqTrack::ParserTrackParam& rTrackParam = track->GetParserTrackParam();
    MmlSeqTrack::MmlParserParam& rMmlParam = track->GetMmlParserParam();

    SeqPlayer::ParserPlayerParam& rPlayerParam = player->GetParserPlayerParam();

    if (command <= MML_CMD_MAX) {
        switch (command) {
            case MML_SET_TEMPO: {
                rPlayerParam.tempo = varNo;
                break;
            }
            case MML_SET_PRGNO: {
                if (varNo < SeqTrack::PRGNO_MAX + 1) {
                    rTrackParam.prgNo = varNo & SeqTrack::PRGNO_MAX;
                } else {
                    NW4HBMWarningMessage_Line(377, "nw4hbm::snd::MmlParser: too large prg No. %d", varNo);
                }
                break;
            }

            case MML_SET_MUTE: {
                track->SetMute(static_cast<SeqMute>(varNo));
                break;
            }

            case MML_SET_TRACK_VOLUME: {
                rTrackParam.volume = varNo;
                break;
            }
            case MML_SET_TRACK_VOLUME2: {
                rTrackParam.volume2 = varNo;
                break;
            }
            case MML_SET_PLAYER_VOLUME: {
                rPlayerParam.volume = varNo;
                break;
            }

            case MML_SET_TRANSPOSE: {
                rTrackParam.transpose = varNo;
                break;
            }

            case MML_SET_PITCHBEND: {
                rTrackParam.pitchBend = varNo;
                break;
            }
            case MML_SET_BENDRANGE: {
                rTrackParam.bendRange = varNo;
                break;
            }
            case MML_SET_PAN: {
                if (varNo <= 1) {
                    varNo++;
                }
                rTrackParam.pan = varNo - 64;
                break;
            }
            case MML_SET_INITPAN: {
                if (varNo <= 1) {
                    varNo++;
                }
                rTrackParam.initPan = varNo - 64;
                break;
            }
            case MML_SET_SURROUNDPAN: {
                if (varNo <= 1) {
                    varNo++;
                }
                rTrackParam.surroundPan = varNo;
                break;
            }
            case MML_SET_PRIORITY: {
                rTrackParam.priority = varNo;
                break;
            }

            case MML_SET_NOTEWAIT: {
                rMmlParam.noteWaitFlag = varNo;
                break;
            }

            case MML_SET_PORTATIME: {
                rTrackParam.portaTime = varNo;
                break;
            }

            case MML_SET_LFODEPTH: {
                rTrackParam.lfoParam.depth = static_cast<u8>(varNo) / 128.0f;
                break;
            }
            case MML_SET_LFOSPEED: {
                rTrackParam.lfoParam.speed = static_cast<u8>(varNo) * (100.0f / 256.0f);
                break;
            }
            case MML_SET_LFOTARGET: {
                rTrackParam.lfoTarget = varNo;
                break;
            }
            case MML_SET_LFORANGE: {
                rTrackParam.lfoParam.range = varNo;
                break;
            }
            case MML_SET_LFODELAY: {
                rTrackParam.lfoParam.delay = varNo * 5;
                break;
            }

            case MML_SET_SWEEPPITCH: {
                rTrackParam.sweepPitch = varNo / 32.0f;
                break;
            }

            case MML_SET_ATTACK: {
                rTrackParam.attack = varNo;
                break;
            }
            case MML_SET_DECAY: {
                rTrackParam.decay = varNo;
                break;
            }
            case MML_SET_SUSTAIN: {
                rTrackParam.sustain = varNo;
                break;
            }
            case MML_SET_RELEASE: {
                rTrackParam.release = varNo;
                break;
            }
            case MML_SET_TIE: {
                rMmlParam.tieFlag = varNo;
                track->ReleaseAllChannel(-1);
                track->FreeAllChannel();
                break;
            }

            case MML_SET_PORTAMENTO: {
                rTrackParam.portaKey = varNo + rTrackParam.transpose;
                rTrackParam.portaFlag = true;
                break;
            }
            case MML_SET_PORTASPEED: {
                rTrackParam.portaFlag = varNo != 0;
                break;
            }
            case MML_SET_LPFFREQ: {
                rTrackParam.lpfFreq = varNo;
                break;
            }

            case MML_SET_FXSEND_A: {
                rTrackParam.fxSend[AUX_A] = varNo;
                break;
            }
            case MML_SET_FXSEND_B: {
                rTrackParam.fxSend[AUX_B] = varNo;
                break;
            }
            case MML_SET_FXSEND_C: {
                rTrackParam.fxSend[AUX_C] = varNo;
                break;
            }
            case MML_SET_MAINSEND: {
                rTrackParam.mainSend = varNo;
                break;
            }

            case MML_PRINTVAR: {
                if (!mPrintVarEnabledFlag) {
                    break;
                }

                const vs16* pVar = GetVariablePtr(player, track, varNo);

                // clang-format off

                // Convert absolute index into variable type-relative index
                int _varNo = varNo >= SeqPlayer::VARIABLE_NUM       ? varNo - SeqPlayer::VARIABLE_NUM
                        : (varNo >= SeqPlayer::LOCAL_VARIABLE_NUM ? varNo - SeqPlayer::LOCAL_VARIABLE_NUM : varNo);

                // Determine variable type prefix from absolute index
                // 'T' = Track variable, 'G' = Global (player) variable
                const char* pVarType = varNo >= SeqPlayer::VARIABLE_NUM       ? "T"
                                    : (varNo >= SeqPlayer::LOCAL_VARIABLE_NUM ? "G" : "");

                // clang-format on

                OSReport("#%08x[%d]: printvar %sVAR_%d(%d) = %d\n", player, track->GetPlayerTrackNo(), pVarType, _varNo,
                         varNo, *pVar);
                break;
            }

            case MML_OPENTRACK: {
                SeqTrack* pNewTrack = player->GetPlayerTrack(varNo);

                if (pNewTrack == nullptr) {
                    NW4HBMWarningMessage_Line(537, "nw4hbm::snd::MmlParser: opentrack for not allocated track");
                } else if (pNewTrack == track) {
                    NW4HBMWarningMessage_Line(542, "nw4hbm::snd::MmlParser: opentrack for self track");
                } else {
                    pNewTrack->Close();
                    pNewTrack->SetSeqData(rTrackParam.baseAddr, arg2);
                }
                break;
            }

            case MML_JUMP: {
                rTrackParam.currentAddr = rTrackParam.baseAddr + varNo;
                break;
            }

            case MML_CALL: {
                if (rMmlParam.callStackDepth >= CALL_STACK_MAX_DEPTH) {
                    NW4HBMWarningMessage_Line(556, "nw4hbm::snd::MmlParser: cannot 'call' because already too deep");
                    break;
                }

                rMmlParam.callStack[rMmlParam.callStackDepth] = rTrackParam.currentAddr;

                rMmlParam.callStackDepth++;
                rTrackParam.currentAddr = rTrackParam.baseAddr + varNo;
                break;
            }

            case MML_RET: {
                if (rMmlParam.callStackDepth == 0) {
                    NW4HBMWarningMessage_Line(567, "nw4hbm::snd::MmlParser: unmatched sequence command 'ret'");
                    break;
                }

                rMmlParam.callStackDepth--;

                rTrackParam.currentAddr = rMmlParam.callStack[rMmlParam.callStackDepth];
                break;
            }

            case MML_LOOP_START: {
                if (rMmlParam.callStackDepth >= CALL_STACK_MAX_DEPTH) {
                    NW4HBMWarningMessage_Line(577,
                                              "nw4hbm::snd::MmlParser: cannot 'loop_start' because already too deep");
                    break;
                }

                rMmlParam.callStack[rMmlParam.callStackDepth] = rTrackParam.currentAddr;

                rMmlParam.loopCount[rMmlParam.callStackDepth] = varNo;
                rMmlParam.callStackDepth++;
                break;
            }

            case MML_LOOP_END: {
                if (rMmlParam.callStackDepth == 0) {
                    NW4HBMWarningMessage_Line(589, "nw4hbm::snd::MmlParser: unmatched sequence command 'loop_end'");
                    break;
                }

                u8 count = rMmlParam.loopCount[rMmlParam.callStackDepth - 1];

                if (count != 0 && --count == 0) {
                    rMmlParam.callStackDepth--;
                } else {
                    rMmlParam.loopCount[rMmlParam.callStackDepth - 1] = count;

                    rTrackParam.currentAddr = rMmlParam.callStack[rMmlParam.callStackDepth - 1];
                }
                break;
            }
        }
    } else if (command <= MML_EX_CMD_MAX) {
        u32 cmd = command & 0xFF;

        if ((command >> 8) != 0xF0) {
            NW4HBMPanicMessage_Line(612, "NW4HBM:Failed assertion cmd == MML_EX_COMMAND");
        }

        vs16* pVar = nullptr;

        if ((cmd & 0xF0) == MML_EX_ARITHMETIC || (cmd & 0xF0) == MML_EX_LOGIC) {
            pVar = GetVariablePtr(player, track, varNo);
            if (pVar == NULL) {
                return;
            }
        }

        switch (cmd) {
            case MML_EX_SET: {
                *pVar = arg2;
                break;
            }

            case MML_EX_APL: {
                *pVar += arg2;
                break;
            }

            case MML_EX_AMI: {
                *pVar -= arg2;
                break;
            }

            case MML_EX_AMU: {
                *pVar *= arg2;
                break;
            }

            case MML_EX_ADV: {
                if (arg2 != 0) {
                    *pVar /= arg2;
                }
                break;
            }

            case MML_EX_ALS: {
                if (arg2 >= 0) {
                    *pVar <<= arg2;

                } else {
                    *pVar >>= -arg2;
                }
                break;
            }

            case MML_EX_RND: {
                bool minus = false;

                if (arg2 < 0) {
                    minus = true;
                    arg2 = static_cast<s16>(-arg2);
                }

                s32 rand = Util::CalcRandom();
                rand *= arg2 + 1;
                rand >>= 16;

                if (minus) {
                    rand = -rand;
                }

                *pVar = rand;
                break;
            }

            case MML_EX_AAD: {
                *pVar &= arg2;
                break;
            }

            case MML_EX_AOR: {
                *pVar |= arg2;
                break;
            }

            case MML_EX_AER: {
                *pVar ^= arg2;
                break;
            }

            case MML_EX_ACO: {
                *pVar = ~static_cast<u16>(arg2);
                break;
            }

            case MML_EX_AMD: {
                if (arg2 != 0) {
                    *pVar %= arg2;
                }
                break;
            }

            case MML_EX_EQ: {
                rMmlParam.cmpFlag = *pVar == arg2;
                break;
            }

            case MML_EX_GE: {
                rMmlParam.cmpFlag = *pVar >= arg2;
                break;
            }

            case MML_EX_GT: {
                rMmlParam.cmpFlag = *pVar > arg2;
                break;
            }

            case MML_EX_LE: {
                rMmlParam.cmpFlag = *pVar <= arg2;
                break;
            }

            case MML_EX_LT: {
                rMmlParam.cmpFlag = *pVar < arg2;
                break;
            }

            case MML_EX_NE: {
                rMmlParam.cmpFlag = *pVar != arg2;
                break;
            }

            case MML_EX_USERPROC: {
                break;
            }
        }
    }
}

Channel* MmlParser::NoteOnCommandProc(MmlSeqTrack* track, int key, int velocity, s32 length, bool tie) const {
    return track->NoteOn(key, velocity, length, tie);
}

u16 MmlParser::Read16(const u8** ppData) const {
    u16 val = ReadByte(ppData);

    val <<= 8;
    val |= ReadByte(ppData);

    return val;
}

u32 MmlParser::Read24(const u8** ppData) const {
    u32 val = ReadByte(ppData);

    val <<= 8;
    val |= ReadByte(ppData);

    val <<= 8;
    val |= ReadByte(ppData);

    return val;
}

s32 MmlParser::ReadVar(const u8** ppData) const {
    u32 val = 0;
    int i = 0;
    u8 b;

    while (1) {
        NW4HBMAssert_Line(i < 4, 806);
        b = ReadByte(ppData);
        val <<= 7;
        val |= b & 0x7F;
        if (!(b & 0x80)) {
            break;
        }
        i++;
    }

    return val;
}

s32 MmlParser::ReadArg(const u8** ppData, SeqPlayer* seqPlayer, SeqTrack* track, SeqArgType type) const {
    s32 arg;

    switch (type) {
        case SEQ_ARG_U8: {
            arg = ReadByte(ppData);
            break;
        }
        case SEQ_ARG_S16: {
            arg = Read16(ppData);
            break;
        }
        case SEQ_ARG_VMIDI: {
            arg = ReadVar(ppData);
            break;
        }
        case SEQ_ARG_VARIABLE: {
            u8 varNo = ReadByte(ppData);

            const vs16* pVar = GetVariablePtr(seqPlayer, track, varNo);
            if (pVar != NULL) {
                arg = *pVar;
            }
            break;
        }
        case SEQ_ARG_RANDOM: {
            s16 min = Read16(ppData);
            s16 max = Read16(ppData);

            s32 rand = Util::CalcRandom();
            rand *= (max - min) + 1;
            rand >>= 16;
            rand += min;

            arg = rand;
            break;
        }
    }

    return arg;
}

vs16* MmlParser::GetVariablePtr(SeqPlayer* seqPlayer, SeqTrack* track, int varNo) const {
    NW4HBMAssertHeaderClampedLRValue_Line(varNo, 0, SeqPlayer::VARIABLE_NUM + SeqTrack::VARIABLE_NUM, 880);

    if (varNo < SeqPlayer::VARIABLE_NUM) {
        return seqPlayer->GetVariablePtr(varNo);
    }

    if (varNo < SeqPlayer::VARIABLE_NUM + SeqTrack::VARIABLE_NUM) {
        return track->GetVariablePtr(varNo - SeqPlayer::VARIABLE_NUM);
    }

    return nullptr;
}

} // namespace detail
} // namespace snd
} // namespace nw4hbm
