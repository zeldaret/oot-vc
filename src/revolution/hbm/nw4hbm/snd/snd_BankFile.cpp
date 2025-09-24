#include "revolution/hbm/snd.hpp"
#include "revolution/hbm/ut.hpp"

namespace nw4hbm {
namespace snd {
namespace detail {
enum {
    DATATYPE_NONE = Util::DATATYPE_T0,
    DATATYPE_INSTPARAM = Util::DATATYPE_T1,
    DATATYPE_RANGETABLE = Util::DATATYPE_T2,
    DATATYPE_INDEXTABLE = Util::DATATYPE_T3,
};

bool BankFileReader::IsValidFileHeader(const void* bankData) {
    const ut::BinaryFileHeader* fileHeader = static_cast<const ut::BinaryFileHeader*>(bankData);
    NW4HBMAssert_Line(fileHeader->signature == BankFile::SIGNATURE_FILE, 54);

    if (fileHeader->signature != BankFile::SIGNATURE_FILE) {
        return false;
    }

    NW4HBMAssertMessage_Line(fileHeader->version >= NW4HBM_VERSION(1, 0), 62,
                             "bank file is not supported version.\n  please reconvert file using new version tools.\n");
    if (fileHeader->version < NW4R_VERSION(1, 0)) {
        return false;
    }

    NW4HBMAssertMessage_Line(fileHeader->version <= FILE_VERSION, 68,
                             "bank file is not supported version.\n  please reconvert file using new version tools.\n");
    if (fileHeader->version > FILE_VERSION) {
        return false;
    }

    return true;
}

BankFileReader::BankFileReader(const void* bankData) : mHeader(nullptr), mDataBlock(nullptr), mWaveBlock(nullptr) {
    NW4HBMAssertPointerNonnull_Line(bankData, 88);

    if (!IsValidFileHeader(bankData)) {
        return;
    }

    mHeader = static_cast<const BankFile::Header*>(bankData);

    mDataBlock = static_cast<const BankFile::DataBlock*>(ut::AddOffsetToPtr(mHeader, mHeader->dataBlockOffset));
    NW4HBMAssert_Line(mDataBlock->blockHeader.kind == BankFile::SIGNATURE_DATA_BLOCK, 97);

    mWaveBlock = static_cast<const BankFile::WaveBlock*>(ut::AddOffsetToPtr(mHeader, mHeader->waveBlockOffset));
    NW4HBMAssert_Line(mWaveBlock->blockHeader.kind == BankFile::SIGNATURE_WAVE_BLOCK, 101);
}

bool BankFileReader::ReadInstInfo(InstInfo* instInfo, int prgNo, int key, int velocity) const {
    NW4HBMAssertPointerNonnull_Line(instInfo, 123);
    NW4HBMAssertPointerNonnull_Line(mHeader, 124);

    if (mHeader == nullptr) {
        return false;
    }

    if (prgNo < 0 || prgNo >= static_cast<int>(mDataBlock->instTable.count)) {
        return false;
    }

    const BankFile::DataRegion* ref = &mDataBlock->instTable.items[prgNo];
    if (ref->dataType == Util::DATATYPE_INVALID) {
        return false;
    }

    if (ref->dataType != DATATYPE_INSTPARAM) {
        ref = GetReferenceToSubRegion(ref, key);
        if (ref == nullptr) {
            return false;
        }
    }

    if (ref->dataType == Util::DATATYPE_INVALID) {
        return false;
    }

    if (ref->dataType != DATATYPE_INSTPARAM) {
        ref = GetReferenceToSubRegion(ref, velocity);
        if (ref == nullptr) {
            return false;
        }
    }

    if (ref->dataType != DATATYPE_INSTPARAM) {
        return false;
    }

    const BankFile::InstParam* param = Util::GetDataRefAddress1(*ref, &mDataBlock->instTable);

    if (param == nullptr) {
        return false;
    }

    if (param->waveIndex < 0) {
        return false;
    }

    instInfo->waveIndex = param->waveIndex;
    instInfo->attack = param->attack;
    instInfo->decay = param->decay;
    instInfo->sustain = param->sustain;
    instInfo->release = param->release;
    instInfo->originalKey = param->originalKey;
    instInfo->pan = param->pan;

    if (mHeader->fileHeader.version == FILE_VERSION) {
        instInfo->volume = param->volume;
        instInfo->tune = param->tune;
    } else {
        instInfo->volume = 127;
        instInfo->tune = 1.0f;
    }

    return true;
}

const BankFile::DataRegion* BankFileReader::GetReferenceToSubRegion(const BankFile::DataRegion* ref,
                                                                    int splitKey) const {
    const BankFile::DataRegion* pSub = nullptr;

    switch (ref->dataType) {
        case DATATYPE_NONE: {
            break;
        }
        case DATATYPE_INSTPARAM: {
            pSub = ref;
            break;
        }
        case DATATYPE_RANGETABLE: {
            const BankFile::RangeTable* pRangeTable = Util::GetDataRefAddress2(*ref, &mDataBlock->instTable);

            if (pRangeTable == nullptr) {
                return nullptr;
            }

            int i = 0;
            while (splitKey > ReadByte(pRangeTable->key + i)) {
                if (++i >= pRangeTable->tableSize) {
                    return nullptr;
                }
            }

            const u8* base = reinterpret_cast<const u8*>(pRangeTable);
            u32 refOffset = i * sizeof(BankFile::DataRegion);
            u32 refStart = ut::RoundUp<u32>(pRangeTable->tableSize + 1, 4);

            pSub = reinterpret_cast<const BankFile::DataRegion*>(base + refOffset + refStart);
            break;
        }
        case DATATYPE_INDEXTABLE: {
            const BankFile::IndexTable* pIndexTable = Util::GetDataRefAddress3(*ref, &mDataBlock->instTable);

            if (pIndexTable == nullptr) {
                return nullptr;
            }

            if (splitKey < pIndexTable->min || splitKey > pIndexTable->max) {
                return nullptr;
            }

            pSub = reinterpret_cast<const BankFile::DataRegion*>(pIndexTable->ref + (splitKey - pIndexTable->min));
            break;
        }
    }

    return pSub;
}

bool BankFileReader::ReadWaveParam(WaveData* waveParam, int waveIndex, const void* waveAddr) const {
    NW4HBMAssertPointerNonnull_Line(mHeader, 263);

    if (mHeader == nullptr) {
        return false;
    }

    if (mWaveBlock == nullptr) {
        return false;
    }

    if (waveIndex >= mWaveBlock->waveInfoTable.count) {
        return false;
    }

    const BankFile::WaveRegion* ref = &mWaveBlock->waveInfoTable.items[waveIndex];
    const WaveFile::WaveInfo* info = Util::GetDataRefAddress0(*ref, &mWaveBlock->waveInfoTable);

    if (info == nullptr) {
        return false;
    }

    WaveFileReader wfr(info);
    return wfr.ReadWaveParam(waveParam, waveAddr);
}
} // namespace detail
} // namespace snd
} // namespace nw4hbm
