#include "revolution/hbm/nw4hbm/snd/WsdFile.h"
#include "revolution/hbm/ut.hpp"

namespace nw4hbm {
namespace snd {
namespace detail {

bool WsdFileReader::IsValidFileHeader(const void* wsdData) {
    const ut::BinaryFileHeader* fileHeader = static_cast<const ut::BinaryFileHeader*>(wsdData);
    NW4HBMAssert_Line(fileHeader->signature == WsdFile::SIGNATURE_FILE, 55);

    if (fileHeader->signature != WsdFile::SIGNATURE_FILE) {
        return false;
    }

    NW4HBMAssertMessage_Line(fileHeader->version >= NW4HBM_VERSION(1, 0), 63,
                             "wsd file is not supported version.\n  please reconvert file using new version tools.\n");
    if (fileHeader->version < NW4HBM_VERSION(1, 0)) {
        return false;
    }

    NW4HBMAssertMessage_Line(fileHeader->version <= WsdFile::FILE_VERSION, 69,
                             "wsd file is not supported version.\n  please reconvert file using new version tools.\n");
    if (fileHeader->version > WsdFile::FILE_VERSION) {
        return false;
    }

    return true;
}

WsdFileReader::WsdFileReader(const void* wsdData) : mHeader(nullptr), mDataBlock(nullptr), mWaveBlock(nullptr) {
    NW4HBMAssertPointerNonnull_Line(wsdData, 89);

    if (!IsValidFileHeader(wsdData)) {
        return;
    }

    mHeader = static_cast<const WsdFile::Header*>(wsdData);

    mDataBlock = static_cast<const WsdFile::DataBlock*>(ut::AddOffsetToPtr(mHeader, mHeader->dataBlockOffset));
    NW4HBMAssert_Line(mDataBlock->blockHeader.kind == WsdFile::SIGNATURE_DATA_BLOCK, 98);

    mWaveBlock = static_cast<const WsdFile::WaveBlock*>(ut::AddOffsetToPtr(mHeader, mHeader->waveBlockOffset));
    NW4HBMAssert_Line(mWaveBlock->blockHeader.kind == WsdFile::SIGNATURE_WAVE_BLOCK, 102);
}

bool WsdFileReader::ReadWaveSoundInfo(WaveSoundInfo* soundInfo, int id) const {
    const WsdFile::Wsd* pWsd = Util::GetDataRefAddress0(mDataBlock->refWsd[id], &mDataBlock->wsdCount);
    const WsdFile::WsdInfo* pWsdInfo = Util::GetDataRefAddress0(pWsd->refWsdInfo, &mDataBlock->wsdCount);

    if (mHeader->fileHeader.version == NW4R_VERSION(1, 2)) {
        soundInfo->pitch = pWsdInfo->pitch;
        soundInfo->pan = pWsdInfo->pan;
        soundInfo->surroundPan = pWsdInfo->surroundPan;
        soundInfo->fxSendA = pWsdInfo->fxSendA;
        soundInfo->fxSendB = pWsdInfo->fxSendB;
        soundInfo->fxSendC = pWsdInfo->fxSendC;
        soundInfo->mainSend = pWsdInfo->mainSend;
    } else if (mHeader->fileHeader.version == NW4R_VERSION(1, 1)) {
        soundInfo->pitch = pWsdInfo->pitch;
        soundInfo->pan = pWsdInfo->pan;
        soundInfo->surroundPan = pWsdInfo->surroundPan;
        soundInfo->fxSendA = 0;
        soundInfo->fxSendB = 0;
        soundInfo->fxSendC = 0;
        soundInfo->mainSend = 127;
    } else {
        soundInfo->pitch = 1.0f;
        soundInfo->pan = 64;
        soundInfo->surroundPan = 0;
        soundInfo->fxSendA = 0;
        soundInfo->fxSendB = 0;
        soundInfo->fxSendC = 0;
        soundInfo->mainSend = 127;
    }

    return true;
}

bool WsdFileReader::ReadWaveSoundNoteInfo(WaveSoundNoteInfo* soundNoteInfo, int id, int note) const {
    const WsdFile::Wsd* pWsd = Util::GetDataRefAddress0(mDataBlock->refWsd[id], &mDataBlock->wsdCount);
    const WsdFile::NoteTable* pTable = Util::GetDataRefAddress0(pWsd->refNoteTable, &mDataBlock->wsdCount);
    const WsdFile::NoteInfo* noteInfo = Util::GetDataRefAddress0(pTable->items[note], &mDataBlock->wsdCount);

    soundNoteInfo->waveIndex = noteInfo->waveIndex;
    soundNoteInfo->attack = noteInfo->attack;
    soundNoteInfo->decay = noteInfo->decay;
    soundNoteInfo->sustain = noteInfo->sustain;
    soundNoteInfo->release = noteInfo->release;
    soundNoteInfo->originalKey = noteInfo->originalKey;
    soundNoteInfo->volume = noteInfo->volume;

    if (mHeader->fileHeader.version >= NW4R_VERSION(1, 1)) {
        soundNoteInfo->pan = noteInfo->pan;
        soundNoteInfo->surroundPan = noteInfo->surroundPan;
        soundNoteInfo->pitch = noteInfo->pitch;
    } else {
        soundNoteInfo->pan = 64;
        soundNoteInfo->surroundPan = 0;
        soundNoteInfo->pitch = 1.0f;
    }

    return true;
}

bool WsdFileReader::ReadWaveParam(int id, WaveData* waveData, const void* waveAddr) const {
    const WaveFile::WaveInfo* waveInfo;

    if (mHeader->fileHeader.version == NW4R_VERSION(1, 0)) {
        const WsdFile::WaveBlockOld* waveBlockOld = reinterpret_cast<const WsdFile::WaveBlockOld*>(mWaveBlock);
        waveInfo =
            static_cast<const WaveFile::WaveInfo*>(ut::AddOffsetToPtr(waveBlockOld, waveBlockOld->offsetTable[id]));
    } else {
        if (id >= mWaveBlock->waveCount) {
            return false;
        }

        waveInfo = static_cast<const WaveFile::WaveInfo*>(ut::AddOffsetToPtr(mWaveBlock, mWaveBlock->offsetTable[id]));
    }

    WaveFileReader reader(waveInfo);
    return reader.ReadWaveParam(waveData, waveAddr);
}

} // namespace detail
} // namespace snd
} // namespace nw4hbm
