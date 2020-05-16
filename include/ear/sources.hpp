#ifndef EAR_SOURCES_HPP
#define EAR_SOURCES_HPP

#include "juce_audio_formats/juce_audio_formats.h"

namespace ear {

class AudioSource : public juce::AudioSource {
public:
	virtual ~AudioSource() = default;

	virtual void prepareToPlay(int, double) override {}

	virtual void releaseResources() override {}
};

class WhiteNoiseSource : public AudioSource {
    juce::Random random;

public:
    virtual ~WhiteNoiseSource() = default;

    int getChannelCount() const {
        return 1;
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo &info) override {
        DBG("WhiteNoiseSource audioBlock range=" + juce::String(0) + " numChannels=" + juce::String(info.buffer->getNumChannels()));

        if (info.buffer->getNumChannels() > 0) {
            auto* p = info.buffer->getWritePointer(0, info.startSample);

            for (auto sample = 0; sample < info.numSamples; ++sample) {
                p[sample] = random.nextFloat() * 2.0 - 1.0;
            }
        }
    }
};

class FileAudioSource : public juce::AudioSource {
    juce::AudioFormatManager _manager;
    std::unique_ptr<juce::AudioFormatReaderSource> _source{nullptr};

public:
    FileAudioSource(const juce::File& file) {
        _manager.registerBasicFormats();
        _source = std::make_unique<juce::AudioFormatReaderSource>(_manager.createReaderFor(file), true);
    }

    ~FileAudioSource() = default;

    int getChannelCount() const {
        return _source->getAudioFormatReader()->numChannels;
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {
        _source->prepareToPlay(samplesPerBlockExpected, sampleRate);
    }

    void releaseResources() override {
        _source->releaseResources();
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo &info) override {
        _source->getNextAudioBlock(info);
    }
};


} // namespace ear

#endif
