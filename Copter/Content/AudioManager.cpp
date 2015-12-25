#include "pch.h"
#include "..\Common\DirectXHelper.h"
#include "AudioManager.h"

// 4 Character Tags in the RIFF File of Interest (read backwards)
const uint32 FOURCC_RIFF_TAG = 'FFIR';
const uint32 FOURCC_FORMAT_TAG = ' tmf';
const uint32 FOURCC_DATA_TAG = 'atad';
const uint32 FOURCC_WAVE_FILE_TAG = 'EVAW';

// The header of every 'chunk' of data in the RIFF file
struct ChunkHeader
{
    uint32 tag;
    uint32 size;
};

// Helper function to find a riff-chunk with-in the sent bounds.
// It is assumed the bounds begin at the start of a chunk
uint64 FindChunk(RandomAccessReader^ file, uint32 tag, uint64 startLoc, uint64 endLoc)
{
    file->SeekAbsolute(startLoc);
    uint64 newLoc = startLoc;
    while (endLoc > (newLoc + sizeof(ChunkHeader)))
    {
        Platform::Array<byte>^ headerBytes = file->Read(sizeof(ChunkHeader));
        ChunkHeader* header = reinterpret_cast<ChunkHeader*>(headerBytes->Data);
        if (header->tag == tag)
            return newLoc;
        file->SeekRelative(static_cast<int64>(header->size));
        newLoc += header->size + sizeof(*header);
    }
    throw ref new Platform::FailureException();
}

// Read the riff chunk header at the send location
void ReadHeader(RandomAccessReader^ file, uint64 loc, ChunkHeader& header)
{
    file->SeekAbsolute(loc);
    Platform::Array<byte>^ headerBytes = file->Read(sizeof(ChunkHeader));
    header = *reinterpret_cast<ChunkHeader*>(headerBytes->Data);
}

Audio::Audio(Platform::String^ fileName)
{
    RandomAccessReader^ riffFile = ref new RandomAccessReader(fileName);
    uint64 fileSize = riffFile->GetFileSize();
    uint64 riffLoc = FindChunk(riffFile, FOURCC_RIFF_TAG, 0, fileSize);
    ChunkHeader chunkHeader;
    ReadHeader(riffFile, riffLoc, chunkHeader);

    uint32 tag = 0;
    Platform::Array<byte>^ riffData = riffFile->Read(sizeof(tag));
    tag = *reinterpret_cast<uint32*>(riffData->Data);
    if (tag != FOURCC_WAVE_FILE_TAG)
        throw ref new Platform::FailureException(); // Only support .wav files

    uint64 riffChildrenStart = riffLoc + sizeof(chunkHeader)+sizeof(tag);
    uint64 riffChildrenEnd = riffLoc + sizeof(chunkHeader)+chunkHeader.size;

    uint64 formatLoc = FindChunk(riffFile, FOURCC_FORMAT_TAG, riffChildrenStart, riffChildrenEnd);
    ReadHeader(riffFile, formatLoc, chunkHeader);
    if (chunkHeader.size < sizeof(WAVEFORMATEX))
        throw ref new Platform::FailureException();

    m_soundFormat = riffFile->Read(chunkHeader.size);
    WAVEFORMATEX format = *reinterpret_cast<WAVEFORMATEX*>(m_soundFormat->Data);
    if (format.wFormatTag != WAVE_FORMAT_PCM && format.wFormatTag != WAVE_FORMAT_ADPCM)
        throw ref new Platform::FailureException();
  
    uint64 dataChunkStart = FindChunk(riffFile, FOURCC_DATA_TAG, riffChildrenStart, riffChildrenEnd);
    ReadHeader(riffFile, dataChunkStart, chunkHeader);

    m_soundData = riffFile->Read(chunkHeader.size);
}

AudioManager::AudioManager() : m_masterVoice(nullptr)
{
    DX::ThrowIfFailed(XAudio2Create(&m_xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR));
    DX::ThrowIfFailed(m_xaudio2->CreateMasteringVoice(&m_masterVoice));
}

unsigned int AudioManager::AddAudio(Audio^ audio)
{
    Sound sound;
    sound.audio = audio;
    DX::ThrowIfFailed(m_xaudio2->CreateSourceVoice(&sound.sourceVoice, audio->GetSoundFormat()));
    m_sounds.push_back(sound);
    return m_sounds.size() - 1;
}

void AudioManager::Play(unsigned int index, bool loop)
{
    DX::ThrowIfFailed(m_sounds[index].sourceVoice->Stop());
    DX::ThrowIfFailed(m_sounds[index].sourceVoice->FlushSourceBuffers());

    Audio ^ audio = m_sounds[index].audio;
    XAUDIO2_BUFFER buffer = { 0 };
    buffer.AudioBytes = audio->GetSoundData()->Length;
    buffer.pAudioData = audio->GetSoundData()->Data;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    if (loop) buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

    DX::ThrowIfFailed(m_sounds[index].sourceVoice->SubmitSourceBuffer(&buffer));
    DX::ThrowIfFailed(m_sounds[index].sourceVoice->Start());
}

void AudioManager::Stop(unsigned int index)
{
    DX::ThrowIfFailed(m_sounds[index].sourceVoice->Stop());
    DX::ThrowIfFailed(m_sounds[index].sourceVoice->FlushSourceBuffers());
}