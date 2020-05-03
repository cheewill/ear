#include <chrono>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <algorithm>


#include "juce_core/juce_core.h"
#include "juce_audio_devices/juce_audio_devices.h"
#include "juce_audio_basics/juce_audio_basics.h"

namespace ear {



class AudioSource : public juce::AudioSource {
    juce::Uuid _uuid;

public:
    virtual ~AudioSource() = default;

    juce::Uuid getUuid() const { return _uuid; }

    virtual int getChannelCount() const = 0;

    virtual void audioDeviceAboutToStart(juce::AudioIODevice* device) {}

    virtual void audioDeviceStopped() {}

    virtual void releaseResources() {}
};
/*
class RawAudioSource : public AudioSource {
	juce::AudioSource* _source{nullptr};

public:
	RawAudioSource(juce::AudioSource* source)
	: _source(source)
	{}

	int getChannelCount() const {
		return 0;
	}

	void releaseResources() {
		_source->releaseResources();
	}

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
		_source->prepareToPlay(samplesPerBlockExpected, sampleRate);
	}

	void getNextAudioBlock(const AudioSourceChannelInfo &bufferToFill) {
		_source->getNextAudioBlock(bufferToFill);
	}
};
*/
class AudioDeviceSink {
public:
    juce::CriticalSection _mutex;
    std::unique_ptr<AudioSource> _source{nullptr};
    juce::AudioSourcePlayer _player;


    void setSource(std::unique_ptr<AudioSource>&& source) {
        juce::ScopedLock lock(_mutex);
        _source = std::move(source);
        _player.setSource(_source.get());
    }
    void update(float** data, int numChannels, int numSamples) {
        //DBG("AudioDeviceSink::update");
        _player.audioDeviceIOCallback(NULL, 0, data, numChannels, numSamples);
    }

    void audioDeviceAboutToStart(juce::AudioIODevice* device) {
        _player.audioDeviceAboutToStart(device);
    }

    void audioDeviceStopped() {
        _player.audioDeviceStopped();
    }
};

class AudioDeviceSource : public AudioSource {
    juce::CriticalSection _mutex;
    juce::AudioBuffer<float> buffer;

public:
    AudioDeviceSource() = default;
    ~AudioDeviceSource() = default;

    void update(const float** data, int numChannels, int numSamples) {

    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) {
        // TODO
    }
};

class AudioIoDevice : juce::AudioIODeviceCallback {
    std::shared_ptr<juce::AudioIODevice> _device{nullptr};
    std::shared_ptr<juce::AudioIODeviceType> _type{nullptr};

    juce::CriticalSection _mutex;
	std::vector<juce::AudioIODeviceCallback> _callbacks;
    //std::shared_ptr<AudioDeviceSource> _input;
    std::shared_ptr<AudioDeviceSink> _output;
	juce::AudioBuffer<float> _outputCache;

	juce::AudioSampleBuffer _buffer;
	juce::AudioProcessorGraph* _graph{nullptr};

public:
    static juce::Array<std::shared_ptr<juce::AudioIODeviceType>> getAllDeviceTypes() {
        juce::AudioDeviceManager manager;

        juce::OwnedArray<juce::AudioIODeviceType> types;
        manager.createAudioDeviceTypes(types);

        juce::Array<std::shared_ptr<juce::AudioIODeviceType>> typeArray;

        while (!types.isEmpty()) {
            std::shared_ptr<juce::AudioIODeviceType> type(types.removeAndReturn(0));
            typeArray.add(std::move(type));
        }

        return typeArray;
    }

    static juce::Array<std::shared_ptr<AudioIoDevice>> getAllDevices() {
        juce::Array<std::shared_ptr<AudioIoDevice>> devices;

        for (auto& type: getAllDeviceTypes()) {
            type->scanForDevices();

            juce::StringArray deviceNames(type->getDeviceNames());

            for (juce::String& name : deviceNames) {
                auto d = std::make_shared<AudioIoDevice>(type, name);
                devices.add(d);
            }
        }

        return devices;
    }

    AudioIoDevice(std::shared_ptr<juce::AudioIODeviceType>& type, juce::String name)
    : _device(type->createDevice(name, name))
    , _type(type)
    {}

    ~AudioIoDevice() {
        close();
    }

	std::shared_ptr<juce::AudioIODevice> getDevice() const { return _device; }

	std::shared_ptr<juce::AudioIODeviceType> getDeviceType() const { return _type; }

    juce::String getName() const { return _device->getName(); }

    juce::String getTypeName() const { return _device->getTypeName(); }

    juce::StringArray getOutputChannelNames() const { return _device->getOutputChannelNames(); }

    unsigned getOutputChannelCount() const { return _device->getOutputChannelNames().size(); }

    unsigned getInputChannelCount() const { return _device->getInputChannelNames().size(); }

	void setGraph(juce::AudioProcessorGraph* graph) {
		juce::ScopedLock lock(_mutex);

		_graph = graph;
	}

    bool open() {
        juce::BigInteger inputs;
        inputs.setRange(0, getInputChannelCount(), true);

        juce::BigInteger outputs;
        outputs.setRange(0, getOutputChannelCount(), true);

        juce::String err = _device->open(inputs, outputs, 44100, 512);

        if (err.isEmpty()) {
            juce::Logger::writeToLog("Device open error=" + err);
        }

        //_input = std::make_shared<AudioDeviceSource>();
        _output = std::make_shared<AudioDeviceSink>();

        return err == "";
    }

    void close() {
        _device->close();

        //_input = nullptr;
        _output = nullptr;
    }

    void start(juce::AudioIODeviceCallback* callback=nullptr) {
        DBG("starting device");
		if (callback == nullptr) {
	        _device->start(this);
		} else {
			_device->start(callback);
		}
    }

    void stop() {
        _device->stop();
    }

    //std::shared_ptr<AudioDeviceSource> getInputDevice() const { return _input; }

    std::shared_ptr<AudioDeviceSink> getOutputDevice() const { return _output; }

	void outputBuffer(juce::AudioBuffer<float>& buffer) {
		DBG("outputBuffer");
		juce::ScopedLock lock(_mutex);
		_outputCache.makeCopyOf(buffer);
	}

private:
    void audioDeviceIOCallback (const float **inputChannelData, int numInputChannels, float **outputChannelData, int numOutputChannels, int requestedSamples) {
        DBG("audioDeviceIOCallback=");

        const juce::ScopedLock lock(_mutex);

		if (_graph) {
			_buffer.setSize(numOutputChannels, requestedSamples);

//			juce::AudioBuffer<float> buffer(outputChannelData, numOutputChannels, requestedSamples);
			juce::MidiBuffer midi;

			//buffer.setSample(0, 0, 0.01);
			DBG("before processBlock sample[0][0]=" + juce::String(_buffer.getSample(0, 0)));
			_graph->processBlock(_buffer, midi);

			DBG("audioDeviceIOCallback graph#processBlock numOutputChannels=" + juce::String(numOutputChannels) + " numSamples=" + juce::String(requestedSamples) + " [0][0]=" + juce::String(_buffer.getSample(0, 0)));
		}

		int numChannels = std::min(_outputCache.getNumChannels(), numOutputChannels);
		int numSamples = std::min(_outputCache.getNumSamples(), requestedSamples);
		for (int i = 0; i < numChannels; ++i) {
			memcpy(outputChannelData[i], _outputCache.getReadPointer(i), numSamples);
		}

        //_input->update(inputChannelData, numInputChannels, numSamples);
        //_output->update(outputChannelData, numOutputChannels, numSamples);

		DBG("~audioDeviceIOCallback [0][0]=" + juce::String(_outputCache.getSample(0, 0)));
    }


    void audioDeviceAboutToStart(juce::AudioIODevice *device) {
        DBG("audioDeviceAboutToStart");

		auto newSampleRate = device->getCurrentSampleRate();
		auto newBlockSize  = device->getCurrentBufferSizeSamples();
		auto numChansIn    = device->getActiveInputChannels().countNumberOfSetBits();
		auto numChansOut   = device->getActiveOutputChannels().countNumberOfSetBits();

		juce::ScopedLock lock(_mutex);

		_graph->prepareToPlay(newSampleRate, newBlockSize);
        //_input->audioDeviceAboutToStart(device);
        //_output->audioDeviceAboutToStart(device);
    }

    void audioDeviceStopped () {
        DBG("audioDeviceStopped");

        //_input->audioDeviceStopped();
        //_output->audioDeviceStopped();
    }

    void audioDeviceError (const juce::String &errorMessage) {
        juce::Logger::writeToLog("audioDeviceError=" + errorMessage);
    }
};
/*
class OutputProcessor : public AudioGraphIOProcessor {
	OutputProcessor(std::shared_ptr<AudioIoDevice>& device)
		: AudioGraphIOProcessor(AudioGraphIOProcessor::IODeviceType::audioOutputNode)
		, _device(device)
	{}

	const juce::String getName() const override { return "OutputProcessor"; }

	void fillInPluginDescription(PluginDescription &) const override {}

	void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
		juce::Logger::writeToLog("OutputProcessor::prepareToPlay()");
	}
};
*/
} // namespace ear
