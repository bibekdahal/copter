#pragma once

ref class Audio sealed
{
private: 
    Platform::Array<byte>^    m_soundData;
    Platform::Array<byte>^    m_soundFormat;
internal:
    Audio(Platform::String^ fileName);
    Platform::Array<byte>^ GetSoundData() const { return m_soundData; }
    WAVEFORMATEX* GetSoundFormat() const { return reinterpret_cast<WAVEFORMATEX*>(m_soundFormat->Data); }
};

ref class AudioManager sealed
{
private:
    Microsoft::WRL::ComPtr<IXAudio2> m_xaudio2;
    IXAudio2MasteringVoice* m_masterVoice;

    struct Sound
    {
        IXAudio2SourceVoice* sourceVoice;
        Audio^ audio;
    };
    std::vector<Sound> m_sounds;

public:
    AudioManager();
    virtual ~AudioManager()
    {
        if (m_masterVoice != nullptr)
        {
            m_masterVoice->DestroyVoice();
            m_masterVoice = nullptr;
        }
        for (unsigned int i = 0; i < m_sounds.size(); ++i)
            m_sounds[i].sourceVoice->DestroyVoice();
        m_sounds.clear();
    }
    void Initialize();
    unsigned int AddAudio(Audio^ audio);
    void Play(unsigned int index, bool loop = false);
    void Stop(unsigned int index);
};

