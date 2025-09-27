#include "revolution/hbm/nw4hbm/snd/SoundArchivePlayer.h"

#include "revolution/hbm/nw4hbm/snd/SoundArchive.h"
#include "revolution/hbm/nw4hbm/snd/SoundArchiveFile.h"
#include "revolution/hbm/nw4hbm/snd/SoundArchiveLoader.h"
#include "revolution/hbm/nw4hbm/snd/SoundHandle.h"
#include "revolution/hbm/nw4hbm/snd/SoundPlayer.h"
#include "revolution/hbm/nw4hbm/snd/SoundThread.h"

#include "revolution/hbm/nw4hbm/snd/Bank.h"
#include "revolution/hbm/nw4hbm/snd/MmlSeqTrack.h"
#include "revolution/hbm/nw4hbm/snd/SeqFile.h"

#include "revolution/hbm/nw4hbm/snd/DisposeCallbackManager.h"

#include "revolution/hbm/nw4hbm/snd/TaskManager.h"
#include "revolution/hbm/nw4hbm/snd/TaskThread.h"

#include "revolution/hbm/HBMAssert.hpp"

#include "decomp.h"
#include <stddef.h>

namespace nw4hbm {
namespace snd {

SoundArchivePlayer::SoundArchivePlayer()
    : mSoundArchive(nullptr), mSeqLoadCallback(*this), mSeqCallback(*this), mStrmCallback(*this), mWsdCallback(*this),
      mGroupTable(nullptr), mFileManager(nullptr), mSoundPlayerCount(0), mSoundPlayers(nullptr),
      mSetupBufferAddress(nullptr), mSetupBufferSize(0), mMmlSeqTrackAllocator(&mMmlParser),
      mSeqTrackAllocator(&mMmlSeqTrackAllocator) {
    detail::DisposeCallbackManager::GetInstance().RegisterDisposeCallback(this);
}

SoundArchivePlayer::~SoundArchivePlayer() {
    detail::DisposeCallbackManager::GetInstance().UnregisterDisposeCallback(this);
}

bool SoundArchivePlayer::IsAvailable() const {
    if (mSoundArchive == nullptr) {
        return false;
    }

    return mSoundArchive->IsAvailable();
}

bool SoundArchivePlayer::Setup(const SoundArchive* arc, void* buffer, u32 mramBufferSize, void* strmBuffer,
                               u32 strmBufferSize) {
    // clang-format off
    NW4HBMAssertPointerNonnull_Line(arc, 162);
    NW4HBMAssertPointerNonnull_Line(buffer, 163);
    NW4HBMAssertPointerNonnull_Line(strmBuffer, 164);
    NW4HBMAssert_Line(strmBufferSize >= GetRequiredStrmBufferSize( arc ), 165);
    // clang-format on

    if (!SetupMram(arc, buffer, mramBufferSize)) {
        return false;
    }

    if (!SetupStrmBuffer(arc, strmBuffer, strmBufferSize)) {
        return false;
    }

    return true;
}

void SoundArchivePlayer::Shutdown() {
    mSoundArchive = nullptr;
    mGroupTable = nullptr;
    mFileManager = nullptr;

    for (int i = 0; i < mSoundPlayerCount; i++) {
        mSoundPlayers[i].~SoundPlayer();
    }

    mSoundPlayerCount = 0;
    mSoundPlayers = nullptr;

    mStrmBufferPool.Shutdown();

    mSeqSoundInstanceManager.Destroy(mSetupBufferAddress, mSetupBufferSize);
    mStrmSoundInstanceManager.Destroy(mSetupBufferAddress, mSetupBufferSize);
    mWaveSoundInstanceManager.Destroy(mSetupBufferAddress, mSetupBufferSize);
    mMmlSeqTrackAllocator.Destroy(mSetupBufferAddress, mSetupBufferSize);

    mSetupBufferAddress = nullptr;
    mSetupBufferSize = 0;
}

u32 SoundArchivePlayer::GetRequiredMemSize(const SoundArchive* arc) {
    NW4HBMAssertPointerNonnull_Line(arc, 220);
    u32 size = 0;

    u32 playerCount = arc->GetPlayerCount();
    size += ut::RoundUp(playerCount * sizeof(SoundPlayer), 4);

    for (u32 i = 0; i < playerCount; i++) {
        SoundArchive::PlayerInfo info;
        if (!arc->ReadPlayerInfo(i, &info)) {
            continue;
        }

        for (int j = 0; j < info.playableSoundCount; j++) {
            if (info.heapSize == 0) {
                continue;
            }

            size += ut::RoundUp(info.heapSize + sizeof(detail::PlayerHeap), 4);
        }
    }

    size += ut::RoundUp(arc->GetGroupCount() * sizeof(Group) + (sizeof(GroupTable) - sizeof(Group)), 4);

    SoundArchive::SoundArchivePlayerInfo info;
    if (arc->ReadSoundArchivePlayerInfo(&info)) {
        size += ut::RoundUp(info.seqSoundCount * sizeof(detail::SeqSound), 4);
        size += ut::RoundUp(info.strmSoundCount * sizeof(detail::StrmSound), 4);
        size += ut::RoundUp(info.waveSoundCount * sizeof(detail::WaveSound), 4);
        size += ut::RoundUp(info.seqTrackCount * sizeof(detail::MmlSeqTrack), 4);
    }

    return size;
}

u32 SoundArchivePlayer::GetRequiredStrmBufferSize(const SoundArchive* arc) {
    NW4HBMAssertPointerNonnull_Line(arc, 281);
    int strmChannels = 0;

    SoundArchive::SoundArchivePlayerInfo info;
    if (arc->ReadSoundArchivePlayerInfo(&info)) {
        strmChannels = info.strmChannelCount;
    }

    // TODO: How is this calculated?
    return strmChannels * 0xA000;
}

bool SoundArchivePlayer::SetupMram(const SoundArchive* arc, void* buffer, u32 size) {
    // clang-format off
    NW4HBMAssertPointerNonnull_Line(arc, 313);
    NW4HBMAssertPointerNonnull_Line(buffer, 314);
    NW4HBMAssertAligned_Line(315, buffer, 4);
    NW4HBMAssert_Line(size >= GetRequiredMemSize( arc ), 316);
    // clang-format on

    void* pEndPtr = static_cast<u8*>(buffer) + size;
    void* buf = buffer;

    if (!SetupSoundPlayer(arc, &buf, pEndPtr)) {
        return false;
    }

    if (!CreateGroupAddressTable(arc, &buf, pEndPtr)) {
        return false;
    }

    SoundArchive::SoundArchivePlayerInfo info;
    if (arc->ReadSoundArchivePlayerInfo(&info)) {
        if (!SetupSeqSound(arc, info.seqSoundCount, &buf, pEndPtr)) {
            return false;
        }

        if (!SetupStrmSound(arc, info.strmSoundCount, &buf, pEndPtr)) {
            return false;
        }

        if (!SetupWaveSound(arc, info.waveSoundCount, &buf, pEndPtr)) {
            return false;
        }

        if (!SetupSeqTrack(arc, info.seqTrackCount, &buf, pEndPtr)) {
            return false;
        }
    }

    // clang-format off
    NW4HBMAssert_Line(static_cast<char*>(buf) - static_cast<char*>(buffer) == GetRequiredMemSize( arc ), 0x163);
    // clang-format on

    mSoundArchive = arc;
    mSetupBufferAddress = buffer;
    mSetupBufferSize = size;

    return true;
}

detail::PlayerHeap* SoundArchivePlayer::CreatePlayerHeap(void* buffer, u32 bufferSize) {
    detail::PlayerHeap* pHeap = new (buffer) detail::PlayerHeap();

    buffer = ut::AddOffsetToPtr(buffer, sizeof(detail::PlayerHeap));

    if (!pHeap->Create(buffer, bufferSize)) {
        return nullptr;
    }

    return pHeap;
}

bool SoundArchivePlayer::SetupSoundPlayer(const SoundArchive* arc, void** buffer, void* end) {
    u32 playerCount = arc->GetPlayerCount();
    u32 requireSize = playerCount * sizeof(SoundPlayer);

    void* pPlayerEnd = ut::RoundUp(ut::AddOffsetToPtr(*buffer, requireSize), 4);

    if (ut::ComparePtr(pPlayerEnd, end) > 0) {
        return false;
    }

    void* pPlayerBuffer = *buffer;
    *buffer = pPlayerEnd;

    mSoundPlayers = static_cast<SoundPlayer*>(pPlayerBuffer);
    mSoundPlayerCount = playerCount;

    u8* pPtr = static_cast<u8*>(pPlayerBuffer);

    for (u32 i = 0; i < playerCount; i++, pPtr += sizeof(SoundPlayer)) {
        SoundPlayer* player = new (pPtr) SoundPlayer();

        SoundArchive::PlayerInfo info;
        if (!arc->ReadPlayerInfo(i, &info)) {
            continue;
        }

        player->SetPlayableSoundCount(info.playableSoundCount);
        player->detail_SetPlayableSoundLimit(info.playableSoundCount);

        if (info.heapSize == 0) {
            continue;
        }

        for (int j = 0; j < info.playableSoundCount; j++) {
            u32 requireSize = sizeof(detail::PlayerHeap) + info.heapSize;

            void* pHeapEnd = ut::RoundUp(ut::AddOffsetToPtr(*buffer, requireSize), 4);

            if (ut::ComparePtr(pHeapEnd, end) > 0) {
                return false;
            }

            void* pHeapBuffer = *buffer;
            *buffer = pHeapEnd;

            detail::PlayerHeap* pHeap = CreatePlayerHeap(pHeapBuffer, info.heapSize);

            if (pHeap == nullptr) {
                return false;
            }

            player->detail_AppendPlayerHeap(pHeap);
        }
    }

    return true;
}

bool SoundArchivePlayer::CreateGroupAddressTable(const SoundArchive* arc, void** buffer, void* end) {
    u32 requireSize = arc->GetGroupCount() * sizeof(Group) + (sizeof(GroupTable) - sizeof(Group));

    void* pTableEnd = ut::RoundUp(ut::AddOffsetToPtr(*buffer, requireSize), 4);

    if (ut::ComparePtr(pTableEnd, end) > 0) {
        return false;
    }

    mGroupTable = static_cast<GroupTable*>(*buffer);
    *buffer = pTableEnd;

    mGroupTable->count = arc->GetGroupCount();

    for (int i = 0; i < mGroupTable->count; i++) {
        mGroupTable->items[i].address = nullptr;
        mGroupTable->items[i].waveDataAddress = nullptr;
    }

    return true;
}

bool SoundArchivePlayer::SetupSeqSound(const SoundArchive* arc, int numSounds, void** buffer, void* end) {
    u32 soundsSize = numSounds * sizeof(detail::SeqSound);

    void* soundEnd = ut::RoundUp(ut::AddOffsetToPtr(*buffer, soundsSize), 4);

    if (ut::ComparePtr(soundEnd, end) > 0) {
        return false;
    }

    u32 createNum = mSeqSoundInstanceManager.Create(*buffer, soundsSize);
    NW4HBMAssert_Line(createNum == numSounds, 524);
    *buffer = soundEnd;

    return true;
}

bool SoundArchivePlayer::SetupWaveSound(const SoundArchive* arc, int numSounds, void** buffer, void* end) {
    u32 soundsSize = numSounds * sizeof(detail::WaveSound);

    void* soundEnd = ut::RoundUp(ut::AddOffsetToPtr(*buffer, soundsSize), 4);

    if (ut::ComparePtr(soundEnd, end) > 0) {
        return false;
    }

    u32 createNum = mWaveSoundInstanceManager.Create(*buffer, soundsSize);
    NW4HBMAssert_Line(createNum == numSounds, 556);
    *buffer = soundEnd;

    return true;
}

bool SoundArchivePlayer::SetupStrmSound(const SoundArchive* arc, int numSounds, void** buffer, void* end) {
    u32 soundsSize = numSounds * sizeof(detail::StrmSound);

    void* soundEnd = ut::RoundUp(ut::AddOffsetToPtr(*buffer, soundsSize), 4);

    if (ut::ComparePtr(soundEnd, end) > 0) {
        return false;
    }

    u32 createNum = mStrmSoundInstanceManager.Create(*buffer, soundsSize);
    NW4HBMAssert_Line(createNum == numSounds, 588);
    *buffer = soundEnd;

    return true;
}

bool SoundArchivePlayer::SetupSeqTrack(const SoundArchive* arc, int numTracks, void** buffer, void* end) {
    u32 tracksSize = numTracks * sizeof(detail::MmlSeqTrack);

    void* pTrackEnd = ut::RoundUp(ut::AddOffsetToPtr(*buffer, tracksSize), 4);

    if (ut::ComparePtr(pTrackEnd, end) > 0) {
        return false;
    }

    u32 createNum = mMmlSeqTrackAllocator.Create(*buffer, tracksSize);
    NW4HBMAssert_Line(createNum == numTracks, 620);
    *buffer = pTrackEnd;

    return true;
}

bool SoundArchivePlayer::SetupStrmBuffer(const SoundArchive* arc, void* buffer, u32 bufferSize) {
    if (bufferSize < GetRequiredStrmBufferSize(arc)) {
        return false;
    }

    int strmChannels = 0;

    SoundArchive::SoundArchivePlayerInfo info;
    if (arc->ReadSoundArchivePlayerInfo(&info)) {
        strmChannels = info.strmChannelCount;
    }

    mStrmBufferPool.Setup(buffer, bufferSize, strmChannels);
    return true;
}

void SoundArchivePlayer::Update() {
    ut::AutoMutexLock autoMutex(detail::SoundThread::GetInstance().GetSoundMutex());

    for (u32 i = 0; i < mSoundPlayerCount; i++) {
        GetSoundPlayer(i).Update();
    }

    mSeqSoundInstanceManager.SortPriorityList();
    mStrmSoundInstanceManager.SortPriorityList();
    mWaveSoundInstanceManager.SortPriorityList();
}

const SoundArchive& SoundArchivePlayer::GetSoundArchive() const {
    NW4HBMAssertMessage_Line(mSoundArchive != nullptr, 684, "Setup is not completed.");
    return *mSoundArchive;
}

DECOMP_FORCE(NW4HBMAssertPointerNonnull_String(mSoundArchive));

SoundPlayer& SoundArchivePlayer::GetSoundPlayer(u32 playerId) {
    NW4HBMAssertHeaderClampedLValue_Line(playerId, 0, mSoundPlayerCount, 690);
    return mSoundPlayers[playerId];
}

const void* SoundArchivePlayer::detail_GetFileAddress(u32 id) const {
    const void* pAddr = mSoundArchive->detail_GetFileAddress(id);
    if (pAddr != nullptr) {
        return pAddr;
    }

    if (mFileManager != nullptr) {
        const void* pAddr = mFileManager->GetFileAddress(id);
        if (pAddr != nullptr) {
            return pAddr;
        }
    }

    SoundArchive::FileInfo file;
    if (!mSoundArchive->detail_ReadFileInfo(id, &file)) {
        return nullptr;
    }

    for (u32 i = 0; i < file.filePosCount; i++) {
        SoundArchive::FilePos pos;
        if (!mSoundArchive->detail_ReadFilePos(id, i, &pos)) {
            continue;
        }

        const void* pGroup = GetGroupAddress(pos.groupId);
        if (pGroup == nullptr) {
            continue;
        }

        SoundArchive::GroupItemInfo item;
        if (mSoundArchive->detail_ReadGroupItemInfo(pos.groupId, pos.index, &item)) {
            return static_cast<const u8*>(pGroup) + item.offset;
        }
    }

    return nullptr;
}

const void* SoundArchivePlayer::detail_GetFileWaveDataAddress(u32 id) const {
    const void* pAddr = mSoundArchive->detail_GetWaveDataFileAddress(id);
    if (pAddr != nullptr) {
        return pAddr;
    }

    if (mFileManager != nullptr) {
        const void* pAddr = mFileManager->GetFileWaveDataAddress(id);
        if (pAddr != nullptr) {
            return pAddr;
        }
    }

    SoundArchive::FileInfo file;
    if (!mSoundArchive->detail_ReadFileInfo(id, &file)) {
        return nullptr;
    }

    for (u32 i = 0; i < file.filePosCount; i++) {
        SoundArchive::FilePos pos;
        if (!mSoundArchive->detail_ReadFilePos(id, i, &pos)) {
            continue;
        }

        const void* pGroup = GetGroupWaveDataAddress(pos.groupId);
        if (pGroup == nullptr) {
            continue;
        }

        SoundArchive::GroupItemInfo item;
        if (mSoundArchive->detail_ReadGroupItemInfo(pos.groupId, pos.index, &item)) {
            return static_cast<const u8*>(pGroup) + item.waveDataOffset;
        }
    }

    return nullptr;
}

const void* SoundArchivePlayer::GetGroupAddress(u32 groupId) const {
    if (mGroupTable == nullptr) {
        NW4HBMAssertWarningMessage_Line(
            mGroupTable != nullptr, 819,
            "Failed to SoundArchivePlayer::GetGroupAddress because group table is not allocated.\n");
        return nullptr;
    }

    if (groupId >= mGroupTable->count) {
        return nullptr;
    }

    return mGroupTable->items[groupId].address;
}

void SoundArchivePlayer::SetGroupAddress(u32 groupId, const void* pAddr) {
    if (mGroupTable == nullptr) {
        NW4HBMAssertWarningMessage_Line(
            mGroupTable != nullptr, 845,
            "Failed to SoundArchivePlayer::SetGroupAddress because group table is not allocated.\n");
        return;
    }

    NW4HBMAssertHeaderClampedLValue_Line(groupId, 0, mGroupTable->count, 849);
    mGroupTable->items[groupId].address = pAddr;
}

const void* SoundArchivePlayer::GetGroupWaveDataAddress(u32 groupId) const {
    if (mGroupTable == nullptr) {
        NW4HBMAssertWarningMessage_Line(
            mGroupTable != nullptr, 870,
            "Failed to SoundArchivePlayer::GetGroupWaveDataAddress because group table is not allocated.\n");
        return nullptr;
    }

    if (groupId >= mGroupTable->count) {
        return nullptr;
    }

    return mGroupTable->items[groupId].waveDataAddress;
}

void SoundArchivePlayer::SetGroupWaveDataAddress(u32 groupId, const void* pAddr) {
    if (mGroupTable == nullptr) {
        NW4HBMAssertWarningMessage_Line(
            mGroupTable != nullptr, 896,
            "Failed to SoundArchivePlayer::SetGroupWaveDataAddress because group table is not allocated.\n");
        return;
    }

    NW4HBMAssertHeaderClampedLValue_Line(groupId, 0, mGroupTable->count, 900);
    mGroupTable->items[groupId].waveDataAddress = pAddr;
}

SoundStartable::StartResult SoundArchivePlayer::detail_SetupSound(SoundHandle* handle, u32 soundId,
                                                                  detail::BasicSound::AmbientArgInfo* ambientArgInfo,
                                                                  detail::ExternalSoundPlayer* extPlayer, bool holdFlag,
                                                                  const StartInfo* startInfo) {
    u8 streamBuffer[STREAM_BUFFER_SIZE];

    NW4HBMAssertPointerNonnull_Line(handle, 927);
    if (!IsAvailable()) {
        return SoundStartable::START_ERR_NOT_AVAILABLE;
    }

    if (handle->IsAttachedSound()) {
        handle->DetachSound();
    }

    SoundArchive::SoundInfo sndInfo;
    if (!mSoundArchive->ReadSoundInfo(soundId, &sndInfo)) {
        return SoundStartable::START_ERR_INVALID_SOUNDID;
    }

    u32 playerId = sndInfo.playerId;
    int playerPriority = sndInfo.playerPriority;

    SoundStartable::StartInfo::StartOffsetType startOffsetType = SoundStartable::StartInfo::START_OFFSET_TYPE_MILLISEC;

    int startOffset = 0;
    int voices = 1;

    if (startInfo != nullptr) {
        if (startInfo->playerId != SoundArchive::INVALID_ID) {
            playerId = startInfo->playerId;
        }

        if (startInfo->playerPriority >= 0) {
            playerPriority = startInfo->playerPriority;
        }

        startOffsetType = startInfo->startOffsetType;
        startOffset = startInfo->startOffset;
        voices = startInfo->voiceOutCount;
    }

    int playerPriorityStart = holdFlag ? playerPriority - 1 : playerPriority;

    SoundPlayer& rPlayer = GetSoundPlayer(playerId);
    detail::BasicSound* sound = nullptr;

    switch (mSoundArchive->GetSoundType(soundId)) {
        case SOUND_TYPE_SEQ: {
            SoundArchive::SeqSoundInfo seqInfo;
            if (!mSoundArchive->detail_ReadSeqSoundInfo(soundId, &seqInfo)) {
                return SoundStartable::START_ERR_INVALID_SOUNDID;
            }

            detail::SeqSound* seqSound = rPlayer.detail_AllocSeqSound(
                playerPriority, playerPriorityStart, ambientArgInfo, extPlayer, soundId, &mSeqSoundInstanceManager);

            if (seqSound == nullptr) {
                return SoundStartable::START_ERR_LOW_PRIORITY;
            }

            seqSound->SetId(soundId);

            SoundStartable::StartResult result = PrepareSeqImpl(seqSound, &sndInfo, &seqInfo, voices);

            if (result != SoundStartable::START_SUCCESS) {
                seqSound->Shutdown();
                return result;
            }

            sound = seqSound;
            break;
        }
        case SOUND_TYPE_STRM: {
            SoundArchive::StrmSoundInfo strmInfo;
            if (!mSoundArchive->detail_ReadStrmSoundInfo(soundId, &strmInfo)) {
                return SoundStartable::START_ERR_INVALID_SOUNDID;
            }

            ut::FileStream* stream =
                mSoundArchive->detail_OpenFileStream(sndInfo.fileId, streamBuffer, sizeof(streamBuffer));
            if (stream == nullptr) {
                return START_ERR_CANNOT_OPEN_FILE;
            }
            stream->Close();

            detail::StrmSound* pStrmSound = rPlayer.detail_AllocStrmSound(
                playerPriority, playerPriorityStart, ambientArgInfo, extPlayer, soundId, &mStrmSoundInstanceManager);

            if (pStrmSound == nullptr) {
                return SoundStartable::START_ERR_LOW_PRIORITY;
            }

            pStrmSound->SetId(soundId);

            SoundStartable::StartResult result =
                PrepareStrmImpl(pStrmSound, &sndInfo, &strmInfo, startOffsetType, startOffset, voices);

            if (result != SoundStartable::START_SUCCESS) {
                pStrmSound->Shutdown();
                return result;
            }

            sound = pStrmSound;
            break;
        }
        case SOUND_TYPE_WAVE: {
            SoundArchive::WaveSoundInfo waveInfo;
            if (!mSoundArchive->detail_ReadWaveSoundInfo(soundId, &waveInfo)) {
                return SoundStartable::START_ERR_INVALID_SOUNDID;
            }

            detail::WaveSound* waveSound = rPlayer.detail_AllocWaveSound(
                playerPriority, playerPriorityStart, ambientArgInfo, extPlayer, soundId, &mWaveSoundInstanceManager);

            if (waveSound == nullptr) {
                return SoundStartable::START_ERR_LOW_PRIORITY;
            }

            waveSound->SetId(soundId);

            SoundStartable::StartResult result = PrepareWaveSoundImpl(waveSound, &sndInfo, &waveInfo, voices);

            if (result != SoundStartable::START_SUCCESS) {
                waveSound->Shutdown();
                return result;
            }

            sound = waveSound;
            break;
        }
        default: {
            return SoundStartable::START_ERR_INVALID_SOUNDID;
        }
    }

    handle->detail_AttachSound(sound);
    return SoundStartable::START_SUCCESS;
}

SoundStartable::StartResult SoundArchivePlayer::PrepareSeqImpl(detail::SeqSound* sound,
                                                               const SoundArchive::SoundInfo* soundInfo,
                                                               const SoundArchive::SeqSoundInfo* info, int voices) {
    NW4HBMAssertPointerNonnull_Line(info, 1129);
    u8 streamBuffer[STREAM_BUFFER_SIZE];

    const void* seqData = detail_GetFileAddress(soundInfo->fileId);

    if (seqData == nullptr) {
        detail::PlayerHeap* pHeap = sound->GetPlayerHeap();
        if (pHeap == nullptr) {
            return SoundStartable::START_ERR_NOT_DATA_LOADED;
        }

        detail::FileStreamHandle pFileStream =
            mSoundArchive->detail_OpenFileStream(soundInfo->fileId, streamBuffer, sizeof(streamBuffer));

        if (pHeap->GetFreeSize() < pFileStream->GetSize()) {
            return SoundStartable::START_ERR_NOT_ENOUGH_PLAYER_HEAP;
        }
    }

    detail::SeqPlayer::SetupResult result = sound->Setup(mSeqTrackAllocator, info->allocTrack, voices, &mSeqCallback);

    if (result != detail::SeqPlayer::SETUP_SUCCESS) {
        if (result == detail::SeqPlayer::SETUP_ERR_CANNOT_ALLOCATE_TRACK) {
            return SoundStartable::START_ERR_CANNOT_ALLOCATE_TRACK;
        } else {
            return SoundStartable::START_ERR_UNKNOWN;
        }
    }

    sound->SetInitialVolume(soundInfo->volume / 127.0f);
    sound->SetChannelPriority(info->channelPriority);

    if (seqData != nullptr) {
        detail::SeqFileReader reader(seqData);
        sound->Prepare(reader.GetBaseAddress(), info->dataOffset);
    } else {
        sound->Prepare(&mSeqLoadCallback, reinterpret_cast<u32>(sound));
    }

    return SoundStartable::START_SUCCESS;
}

SoundStartable::StartResult SoundArchivePlayer::PrepareStrmImpl(
    detail::StrmSound* sound, const SoundArchive::SoundInfo* soundInfo, const SoundArchive::StrmSoundInfo* info,
    SoundStartable::StartInfo::StartOffsetType startOffsetType, int startOffset, int voices) {
    detail::StrmPlayer::StartOffsetType strmOffsetType = detail::StrmPlayer::START_OFFSET_TYPE_SAMPLE;

    if (startOffsetType == SoundStartable::StartInfo::START_OFFSET_TYPE_MILLISEC) {
        strmOffsetType = detail::StrmPlayer::START_OFFSET_TYPE_MILLISEC;
    }

    if (!sound->Prepare(&mStrmBufferPool, strmOffsetType, startOffset, voices, &mStrmCallback, soundInfo->fileId)) {
        return SoundStartable::START_ERR_UNKNOWN;
    }

    sound->SetInitialVolume(soundInfo->volume / 127.0f);

    return SoundStartable::START_SUCCESS;
}

SoundStartable::StartResult SoundArchivePlayer::PrepareWaveSoundImpl(detail::WaveSound* sound,
                                                                     const SoundArchive::SoundInfo* soundInfo,
                                                                     const SoundArchive::WaveSoundInfo* info,
                                                                     int voices) {
    NW4HBMAssertPointerNonnull_Line(info, 1261);
    const void* waveData = detail_GetFileAddress(soundInfo->fileId);
    if (waveData == nullptr) {
        return SoundStartable::START_ERR_NOT_DATA_LOADED;
    }

    if (!sound->Prepare(waveData, info->subNo, voices, &mWsdCallback, soundInfo->fileId)) {
        return SoundStartable::START_ERR_UNKNOWN;
    }

    sound->SetInitialVolume(soundInfo->volume / 127.0f);
    sound->SetChannelPriority(info->channelPriority);

    return SoundStartable::START_SUCCESS;
}

bool SoundArchivePlayer::LoadGroup(u32 id, SoundMemoryAllocatable* allocatable, u32 loadBlockSize) {
    NW4HBMAssertPointerNonnull_Line(mSoundArchive, 1304);
    NW4HBMAssertAligned_Line(1305, loadBlockSize, 32);
    if (!IsAvailable()) {
        return false;
    }

    if (id >= mSoundArchive->GetGroupCount()) {
        return false;
    }

    if (GetGroupAddress(id) != nullptr) {
        return true;
    }

    if (allocatable == nullptr) {
        return false;
    }

    detail::SoundArchiveLoader loader(*mSoundArchive);

    void* waveBuffer;
    const void* pGroup = loader.LoadGroup(id, allocatable, &waveBuffer, loadBlockSize);

    if (pGroup == nullptr) {
        return nullptr;
    }

    SetGroupAddress(id, pGroup);
    SetGroupWaveDataAddress(id, waveBuffer);

    return true;
}

bool SoundArchivePlayer::LoadGroup(const char* label, SoundMemoryAllocatable* allocatable, u32 loadBlockSize) {
    u32 id = mSoundArchive->ConvertLabelStringToGroupId(label);
    if (id == SoundArchive::INVALID_ID) {
        return false;
    }

    return LoadGroup(id, allocatable, loadBlockSize);
}

void SoundArchivePlayer::InvalidateData(const void* start, const void* end) {
    if (mGroupTable == nullptr) {
        return;
    }

    for (int i = 0; i < mGroupTable->count; i++) {
        const void* pAddr = mGroupTable->items[i].address;

        if (start <= pAddr && pAddr <= end) {
            mGroupTable->items[i].address = nullptr;
        }
    }
}

void SoundArchivePlayer::InvalidateWaveData(const void* start, const void* end) {
    if (mGroupTable == nullptr) {
        return;
    }

    for (int i = 0; i < mGroupTable->count; i++) {
        const void* pAddr = mGroupTable->items[i].waveDataAddress;

        if (start <= pAddr && pAddr <= end) {
            mGroupTable->items[i].waveDataAddress = nullptr;
        }
    }
}

SoundArchivePlayer::SeqLoadCallback::SeqLoadCallback(const SoundArchivePlayer& player) : mSoundArchivePlayer(player) {
    OSInitMutex(&mMutex);
}

detail::SeqSound::SeqLoadCallback::Result
SoundArchivePlayer::SeqLoadCallback::LoadData(detail::SeqSound::NotifyAsyncEndCallback callback, void* callbackArg,
                                              u32 userData) const {
    if (!mSoundArchivePlayer.IsAvailable()) {
        return RESULT_FAILED;
    }

    const SoundArchive& arc = mSoundArchivePlayer.GetSoundArchive();
    detail::SeqSound* sound = reinterpret_cast<detail::SeqSound*>(userData);
    u32 soundId = sound->GetId();

    SoundArchive::SoundInfo soundInfo;
    if (!arc.ReadSoundInfo(soundId, &soundInfo)) {
        return RESULT_FAILED;
    }
    SoundArchive::SeqSoundInfo info;
    if (!arc.detail_ReadSeqSoundInfo(soundId, &info)) {
        return RESULT_FAILED;
    }

    detail::PlayerHeap* playerHeap = sound->GetPlayerHeap();
    if (playerHeap == nullptr) {
        return RESULT_FAILED;
    }

    NW4HBMAssert_Line(detail::TaskManager::GetInstance().GetTaskBufferSize() >= sizeof(SeqLoadTask), 1445);
    void* addr = detail::TaskManager::GetInstance().Alloc();
    NW4HBMAssertPointerNonnull_Line(addr, 1447);

    SeqLoadTask* task = new (addr)
        SeqLoadTask(callback, callbackArg, arc, soundInfo.fileId, info.dataOffset, *playerHeap, userData, mMutex);

    detail::TaskManager::GetInstance().AppendTask(task, detail::TaskManager::PRIORITY_MIDDLE);
    detail::TaskThread::GetInstance().SendWakeupMessage();

    return RESULT_ASYNC;
}

void SoundArchivePlayer::SeqLoadCallback::CancelLoading(u32 userData) const {
    detail::TaskManager::GetInstance().CancelByTaskId(userData);
}

detail::Channel* SoundArchivePlayer::SeqNoteOnCallback::NoteOn(detail::SeqPlayer* seqPlayer, int bankNo,
                                                               const detail::NoteOnInfo& noteOnInfo) {
    if (!mSoundArchivePlayer.IsAvailable()) {
        return nullptr;
    }

    const SoundArchive& arc = mSoundArchivePlayer.GetSoundArchive();
    u32 soundId = seqPlayer->GetId();

    SoundArchive::SeqSoundInfo seqInfo;
    if (!arc.detail_ReadSeqSoundInfo(soundId, &seqInfo)) {
        return nullptr;
    }

    SoundArchive::BankInfo bankInfo;
    if (!arc.detail_ReadBankInfo(seqInfo.bankId, &bankInfo)) {
        return nullptr;
    }

    const void* bankData = mSoundArchivePlayer.detail_GetFileAddress(bankInfo.fileId);

    if (bankData == nullptr) {
        return nullptr;
    }

    detail::Bank bank(bankData);

    const void* waveData = mSoundArchivePlayer.detail_GetFileWaveDataAddress(bankInfo.fileId);
    if (waveData == nullptr) {
        return nullptr;
    }

    bank.SetWaveDataAddress(waveData);
    return bank.NoteOn(noteOnInfo);
}

bool SoundArchivePlayer::WsdCallback::GetWaveSoundData(detail::WaveSoundInfo* waveSoundInfo,
                                                       detail::WaveSoundNoteInfo* noteInfo, detail::WaveData* waveData,
                                                       const void* pWsdData, int index, int noteIndex,
                                                       u32 userData) const {
    u32 fileId = userData;

    if (!mSoundArchivePlayer.IsAvailable()) {
        return false;
    }

    mSoundArchivePlayer.GetSoundArchive();

    const void* waveAddr = mSoundArchivePlayer.detail_GetFileWaveDataAddress(fileId);
    if (waveAddr == nullptr) {
        return false;
    }

    detail::WsdFileReader reader(pWsdData);

    if (!reader.ReadWaveSoundInfo(waveSoundInfo, index)) {
        return false;
    }

    if (!reader.ReadWaveSoundNoteInfo(noteInfo, index, noteIndex)) {
        return false;
    }

    if (!reader.ReadWaveParam(noteInfo->waveIndex, waveData, waveAddr)) {
        return false;
    }

    return true;
}

SoundArchivePlayer::StrmCallback::StrmCallback(const SoundArchivePlayer& player) : mSoundArchivePlayer(player) {
    OSInitMutex(&mMutex);
}

detail::StrmPlayer::StrmCallback::Result
SoundArchivePlayer::StrmCallback::LoadHeader(detail::StrmPlayer::NotifyLoadHeaderAsyncEndCallback callback,
                                             void* callbackData, u32 userId, u32 userData) const {
    if (!mSoundArchivePlayer.IsAvailable()) {
        return RESULT_FAILED;
    }

    void* addr = detail::TaskManager::GetInstance().Alloc();
    NW4HBMAssertPointerNonnull_Line(addr, 1612);

    const SoundArchive& sndArc = mSoundArchivePlayer.GetSoundArchive();

    NW4HBMAssert_Line(detail::TaskManager::GetInstance().GetTaskBufferSize() >= sizeof(StrmHeaderLoadTask), 1616);
    StrmHeaderLoadTask* task = new (addr) StrmHeaderLoadTask(callback, callbackData, sndArc, userData, userId, mMutex);

    detail::TaskManager::GetInstance().AppendTask(task, detail::TaskManager::PRIORITY_MIDDLE);
    detail::TaskThread::GetInstance().SendWakeupMessage();

    return RESULT_ASYNC;
}

detail::StrmPlayer::StrmCallback::Result SoundArchivePlayer::StrmCallback::LoadStream(
    void* mramAddr, u32 size, s32 offset, int numChannels, u32 blockSize, s32 blockHeaderOffset,
    bool needUpdateAdpcmLoop, detail::StrmPlayer::LoadCommand& callback, u32 userId, u32 userData) const {
    if (!mSoundArchivePlayer.IsAvailable()) {
        return RESULT_FAILED;
    }

    const SoundArchive& sndArc = mSoundArchivePlayer.GetSoundArchive();

    void* addr = detail::TaskManager::GetInstance().Alloc();
    NW4HBMAssertPointerNonnull_Line(addr, 1659);

    NW4HBMAssert_Line(detail::TaskManager::GetInstance().GetTaskBufferSize() >= sizeof(StrmDataLoadTask), 1661);
    StrmDataLoadTask* task =
        new (addr) StrmDataLoadTask(mramAddr, size, offset, numChannels, blockSize, blockHeaderOffset,
                                    needUpdateAdpcmLoop, callback, sndArc, userData, userId, mMutex);

    detail::TaskManager::GetInstance().AppendTask(task, detail::TaskManager::PRIORITY_HIGH);
    detail::TaskThread::GetInstance().SendWakeupMessage();

    return RESULT_ASYNC;
}

void SoundArchivePlayer::StrmCallback::CancelLoading(u32 userId, u32 userData) const {
    detail::TaskManager::GetInstance().CancelByTaskId(userId);
}

SoundArchivePlayer::SeqLoadTask::SeqLoadTask(detail::SeqSound::NotifyAsyncEndCallback callback, void* callbackArg,
                                             const SoundArchive& arc, u32 fileId, u32 dataOffset, SoundHeap& heap,
                                             u32 taskId, OSMutex& mutex)
    : mCallback(callback), mCallbackData(callbackArg), mSoundArchive(arc), mFileId(fileId), mDataOffset(dataOffset),
      mHeap(heap), Task(taskId), mMutex(mutex) {}

void SoundArchivePlayer::SeqLoadTask::Execute() {
    detail::SoundArchiveLoader loader(mSoundArchive);
    {
        ut::AutoMutexLock lock(mMutex);
        mLoader = &loader;
    }

    void* seqData = loader.LoadFile(mFileId, &mHeap);
    {
        ut::AutoMutexLock lock(mMutex);
        mLoader = nullptr;
    }

    if (seqData == nullptr) {
        if (mCallback != nullptr) {
            mCallback(false, nullptr, 0, mCallbackData);
        }
        return;
    }

    detail::SeqFileReader reader(seqData);

    const void* baseAddr = reader.GetBaseAddress();
    s32 offset = static_cast<s32>(mDataOffset);

    if (mCallback != nullptr) {
        mCallback(true, baseAddr, offset, mCallbackData);
    }
}

void SoundArchivePlayer::SeqLoadTask::Cancel() {
    ut::AutoMutexLock lock(mMutex);
    if (mLoader) {
        mLoader->Cancel();
    }
}

SoundArchivePlayer::StrmHeaderLoadTask::StrmHeaderLoadTask(
    detail::StrmPlayer::NotifyLoadHeaderAsyncEndCallback callback, void* callbackData, const SoundArchive& arc,
    u32 fileId, u32 taskId, OSMutex& mutex)
    : Task(taskId), mSoundArchive(arc), mFileId(fileId), mCallback(callback), mCallbackData(callbackData),
      mStream(nullptr), mMutex(mutex) {}

void SoundArchivePlayer::StrmHeaderLoadTask::Execute() {
    static u8 buffer[STREAM_BUFFER_SIZE] alignas(32);
    static OSMutex mutex;

    static bool initFlag = false;

    u8 streamBuffer[STREAM_BUFFER_SIZE];

#undef NULL
#define NULL nullptr
    NW4HBMAssert_Line(mStream == NULL, 1789);

    {
        ut::AutoMutexLock lock(mMutex);
        mStream = mSoundArchive.detail_OpenFileStream(mFileId, streamBuffer, sizeof(streamBuffer));
    }

    if (mStream == nullptr) {
        if (mCallback != nullptr) {
            mCallback(false, nullptr, mCallbackData);
        }
        return;
    }

    if (!mStream->CanSeek() || !mStream->CanRead()) {
        mStream->Close();
        mStream = nullptr;

        if (mCallback != nullptr) {
            mCallback(false, nullptr, mCallbackData);
        }
        return;
    }

    if (!initFlag) {
        OSInitMutex(&mutex);
        initFlag = true;
    }

    ut::AutoMutexLock lock(mutex);

    detail::StrmFileLoader loader(*mStream);
    if (!loader.LoadFileHeader(buffer, sizeof(buffer))) {
        mStream->Close();
        mStream = nullptr;

        if (mCallback != nullptr) {
            mCallback(false, nullptr, mCallbackData);
        }
        return;
    }

    {
        ut::AutoMutexLock lock(mMutex);

        mStream->Close();
        mStream = nullptr;
    }

    detail::StrmPlayer::StrmHeader header;
    loader.ReadStrmInfo(&header.strmInfo);

    for (int i = 0; i < header.strmInfo.numChannels; i++) {
        loader.ReadAdpcmInfo(&header.adpcmInfo[i], i);
    }

    if (mCallback != nullptr) {
        mCallback(true, &header, mCallbackData);
    }
}

void SoundArchivePlayer::StrmHeaderLoadTask::Cancel() {
    ut::AutoMutexLock lock(mMutex);

    mCallback = nullptr;

    if (mStream != nullptr && mStream->CanCancel()) {
        if (mStream->CanAsync()) {
            mStream->CancelAsync(nullptr, nullptr);
        } else {
            mStream->Cancel();
        }
    }
}

SoundArchivePlayer::StrmDataLoadTask::StrmDataLoadTask(void* addr, u32 size, s32 offset, int numChannels, u32 blockSize,
                                                       s32 blockHeaderOffset, bool needUpdateAdpcmLoop,
                                                       detail::StrmPlayer::LoadCommand& callback,
                                                       const SoundArchive& arc, u32 fileId, u32 taskId, OSMutex& mutex)
    : Task(taskId), mAddr(addr), mSize(size), mOffset(offset), mSoundArchive(arc), mFileId(fileId), mStream(nullptr),
      mNumChannels(numChannels), mCallback(&callback), mBlockSize(blockSize), mBlockHeaderOffset(blockHeaderOffset),
      mNeedUpdateAdpcmLoop(needUpdateAdpcmLoop), mMutex(mutex) {
    NW4HBMAssertAligned_Line(1890, addr, 32);
    NW4HBMAssertAligned_Line(1891, size, 32);
    NW4HBMAssertAligned_Line(1892, offset, 4);
}

void SoundArchivePlayer::StrmDataLoadTask::Execute() {
    DCInvalidateRange(mAddr, mSize);

    u8 streamBuffer[STREAM_BUFFER_SIZE];
    {
        ut::AutoMutexLock lock(mMutex);
        mStream = mSoundArchive.detail_OpenFileStream(mFileId, streamBuffer, sizeof(streamBuffer));
    }

    if (mStream == nullptr) {
        if (mCallback != nullptr) {
            mCallback->NotifyAsyncEnd(false);
        }
        return;
    }

    if (!mStream->CanSeek() || !mStream->CanRead()) {
        mStream->Close();
        mStream = nullptr;

        if (mCallback != nullptr) {
            mCallback->NotifyAsyncEnd(false);
        }
        return;
    }

    ut::DvdFileStream* dvdStream = ut::DynamicCast<ut::DvdFileStream*>(mStream);
    if (dvdStream != nullptr) {
        dvdStream->SetPriority(DVD_PRIO_HIGH);
    }

    mStream->Seek(mOffset, ut::FileStream::SEEK_BEG);
    s32 bytesRead = mStream->Read(mAddr, mSize);

    {
        ut::AutoMutexLock lock(mMutex);
        mStream->Close();
        mStream = nullptr;
    }

    if (bytesRead == DVD_RESULT_CANCELED) {
        return;
    }

    if (bytesRead != mSize) {
        NW4HBMAssertWarningMessage_Line(bytesRead != 0, 1948, "failed to load stream\n");
        detail::TaskManager::GetInstance().CancelByTaskId(GetTaskId());

        if (mCallback != nullptr) {
            mCallback->NotifyAsyncEnd(false);
        }

        return;
    }

    u16 adpcmPredScale[CHANNEL_MAX];
    u8* pAddr = static_cast<u8*>(mAddr);
    for (int i = 0; i < mNumChannels; i++) {
        if (mNeedUpdateAdpcmLoop) {
            adpcmPredScale[i] = pAddr[i * ut::RoundUp(mBlockSize, 32) + mBlockHeaderOffset];
        }

        if (mCallback != nullptr) {
            const void* pSrc = i * ut::RoundUp(mBlockSize, 32) + mBlockHeaderOffset + static_cast<u8*>(mAddr);

            u32 len = static_cast<u32>(ut::RoundUp(mBlockSize, 32));

            void* pDst = mCallback->GetBuffer(i);

            memcpy(pDst, pSrc, len);
            DCFlushRange(pDst, len);
        }
    }

    if (mNeedUpdateAdpcmLoop) {
        if (mCallback != nullptr) {
            mCallback->SetAdpcmLoopContext(mNumChannels, adpcmPredScale);
        }
    }

    if (mCallback != nullptr) {
        mCallback->NotifyAsyncEnd(true);
    }
}

void SoundArchivePlayer::StrmDataLoadTask::Cancel() {
    ut::AutoMutexLock lock(mMutex);

    mCallback = nullptr;

    if (mStream && mStream->CanCancel()) {
        if (mStream->CanAsync()) {
            mStream->CancelAsync(nullptr, nullptr);
        } else {
            mStream->Cancel();
        }
    }
}

} // namespace snd
} // namespace nw4hbm
