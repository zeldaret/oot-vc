#include "revolution/hbm/snd.hpp"
#include "revolution/hbm/ut.hpp"

namespace nw4hbm {
namespace snd {
namespace detail {

// See BankFile::Region
enum {
    DATATYPE_NONE = Util::DATATYPE_T0,
    DATATYPE_INSTPARAM = Util::DATATYPE_T1,
    DATATYPE_RANGETABLE = Util::DATATYPE_T2,
    DATATYPE_INDEXTABLE = Util::DATATYPE_T3,
};

bool BankFileReader::IsValidFileHeader(const void* pBankBin) {
    const ut::BinaryFileHeader* pFileHeader = static_cast<const ut::BinaryFileHeader*>(pBankBin);

    NW4HBMAssertMessage_Line(pFileHeader->signature == BankFile::SIGNATURE_FILE, 54,
                             "invalid file signature. bank data is not available.");

    if (pFileHeader->signature != BankFile::SIGNATURE_FILE) {
        return false;
    }

    NW4HBMAssertMessage_Line(pFileHeader->version >= NW4HBM_VERSION(1, 0), 62,
                             "bank file is not supported version.\n  please "
                             "reconvert file using new version tools.\n");

    if (pFileHeader->version < NW4HBM_VERSION(1, 0)) {
        return false;
    }

    NW4HBMAssertMessage_Line(pFileHeader->version <= NW4HBM_VERSION(1, 1), 68,
                             "bank file is not supported version.\n  please "
                             "reconvert file using new version tools.\n");

    if (pFileHeader->version > NW4HBM_VERSION(1, 1)) {
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

BankFile::InstParam const* BankFileReader::GetInstParam(int prgNo, int key, int velocity) const {
    NW4HBMAssertPointerNonnull_Line(mHeader, 135);

    if (!mHeader) {
        return nullptr;
    }

    if (prgNo < 0 || prgNo >= static_cast<int>(mDataBlock->instTable.count)) {
        return nullptr;
    }

    BankFile::DataRegion const* ref = &mDataBlock->instTable.items[prgNo];

    if (ref->dataType == Util::DATATYPE_INVALID) {
        return nullptr;
    }

    if (ref->dataType != DATATYPE_INSTPARAM) {
        ref = GetReferenceToSubRegion(ref, key);
        if (!ref) {
            return nullptr;
        }
    }

    if (ref->dataType == Util::DATATYPE_INVALID) {
        return nullptr;
    }

    if (ref->dataType != DATATYPE_INSTPARAM) {
        ref = GetReferenceToSubRegion(ref, velocity);
        if (!ref) {
            return nullptr;
        }
    }

    if (ref->dataType != DATATYPE_INSTPARAM) {
        return nullptr;
    }

    BankFile::InstParam const* instParam = Util::GetDataRefAddress1(*ref, &mDataBlock->instTable);

    return instParam;
}

bool BankFileReader::ReadInstInfo(InstInfo* instInfo, int prgNo, int key, int velocity) const {
    NW4HBMAssertPointerNonnull_Line(instInfo, 123);

    BankFile::InstParam const* instParam = GetInstParam(prgNo, key, velocity);
    if (instParam == nullptr) {
        return false;
    }

    if (instParam->waveIndex < 0) {
        return false;
    }

    instInfo->waveDataLocation.index = instParam->waveIndex;

    instInfo->attack = instParam->attack;
    instInfo->hold = instParam->hold;
    instInfo->decay = instParam->decay;
    instInfo->sustain = instParam->sustain;
    instInfo->release = instParam->release;
    instInfo->originalKey = instParam->originalKey;
    instInfo->pan = instParam->pan;

    if (mHeader->fileHeader.version >= NW4HBM_VERSION(1, 1)) {
        instInfo->volume = instParam->volume;
        instInfo->tune = instParam->tune;
    } else {
        instInfo->volume = 127;
        instInfo->tune = 1.0f;
    }

    return true;
}

BankFile::DataRegion const* BankFileReader::GetReferenceToSubRegion(BankFile::DataRegion const* ref,
                                                                    int splitKey) const {
    BankFile::DataRegion const* subRef = nullptr;

    switch (ref->dataType) {
        case 0:
            break;

        case 1:
            subRef = ref;
            break;

        case 2: {
            BankFile::RangeTable const* table = Util::GetDataRefAddress2(*ref, &mDataBlock->instTable);

            if (!table) {
                return nullptr;
            }

            int index = 0;
            while (splitKey > ReadByte(table->key + index)) {
                if (++index >= table->tableSize) {
                    return nullptr;
                }
            }

            u32 refOffset = sizeof(BankFile::DataRegion) * index + ut::RoundUp(table->tableSize + 1, 4);

            /* TODO: fake: how to properly match this call? (the arguments to
             * AddOffsetToPtr are supposed to be the other way around)
             */
            subRef = static_cast<BankFile::DataRegion const*>(
                ut::AddOffsetToPtr(reinterpret_cast<void const*>(refOffset), reinterpret_cast<u32>(table)));
        } break;

        case 3: {
            BankFile::IndexTable const* table = Util::GetDataRefAddress3(*ref, &mDataBlock->instTable);

            if (!table) {
                return nullptr;
            }

            if (splitKey < table->min || splitKey > table->max) {
                return nullptr;
            }

            subRef = reinterpret_cast<BankFile::DataRegion const*>(table->ref + (splitKey - table->min) *
                                                                                    sizeof(BankFile::DataRegion));
        } break;
    }

    return subRef;
}

bool BankFileReader::ReadWaveInfo(WaveInfo* waveParam, int waveIndex, const void* pWaveAddr) const {
    NW4HBMAssertPointerNonnull_Line(waveParam, 263);

    if (mHeader == nullptr) {
        return false;
    }

    if (mWaveBlock == nullptr) {
        return false;
    }

    if (waveIndex >= mWaveBlock->waveInfoTable.count) {
        return false;
    }

    const BankFile::WaveRegion* pRef = &mWaveBlock->waveInfoTable.items[waveIndex];

    const WaveFile::WaveInfo* pInfo = Util::GetDataRefAddress0(*pRef, &mWaveBlock->waveInfoTable);

    if (pInfo == nullptr) {
        return false;
    }

    WaveFileReader wfr(pInfo);
    return wfr.ReadWaveInfo(waveParam, pWaveAddr);
}

} // namespace detail
} // namespace snd
} // namespace nw4hbm
