#include "juce_audio_processors/juce_audio_processors.h"

namespace ear {

class GraphDeviceSink : public GraphProcessor, public juce::AudioIODeviceCallback {
public:
	std::shared_ptr<AudioIoDevice> _device;

	juce::CriticalSection _mutex;
	juce::AudioSampleBuffer _cache;

	GraphDeviceSink(const std::shared_ptr<AudioIoDevice> &device)
		: GraphProcessor()
		, _device(device)
	{
		_device->addCallback(this);
	}

	virtual ~GraphDeviceSink() {
		_device->removeCallback(this);
	}

	void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock) override {
		//setPlayConfigDetails(1, 2, sampleRate, estimatedSamplesPerBlock);
		//_source->prepareToPlay(sampleRate, estimatedSamplesPerBlock);
	}

	void releaseResources() override {
		//_source->releaseResources();
	}

	void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) override {
		DBG("GraphDeviceSink#processBlock [0][0]=" + juce::String(buffer.getSample(0, 0)));

		juce::ScopedLock lock(_mutex);
		_cache.makeCopyOf(buffer);
	}

private:
	void audioDeviceIOCallback(const float**, int, float** outputs, int outputChannels, int samples) override {
		juce::AudioSampleBuffer buffer(outputs, outputChannels, samples);

		DBG("GraphDeviceSink#audioDeviceIOCallback [0][0]=" + juce::String(buffer.getSample(0, 0)));

		juce::ScopedLock lock(_mutex);
		buffer.makeCopyOf(_cache);
	}

	void audioDeviceAboutToStart(juce::AudioIODevice *device) override {}

	void audioDeviceStopped() override {}
};

class AudioGraph : public juce::AudioProcessorGraph {
public:
	AudioGraph() {
		setPlayConfigDetails(0, 0, 44100, 512);
	}

	virtual ~AudioGraph() = default;


};

} // namespace ear
