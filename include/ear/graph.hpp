namespace ear {

class WhiteNoiseSource : public juce::AudioSource {
    juce::Random random;

public:
    virtual ~WhiteNoiseSource() = default;

    int getChannelCount() const {
        return 1;
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) {}

    void releaseResources() {}

    void getNextAudioBlock(const juce::AudioSourceChannelInfo &info) {
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

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
        _source->prepareToPlay(samplesPerBlockExpected, sampleRate);
    }

    void releaseResources() {
        _source->releaseResources();
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo &info) {
        _source->getNextAudioBlock(info);
    }
};

class GraphProcessor : public juce::AudioProcessor {
public:
	GraphProcessor() = default;

    //==============================================================================
    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    void processBlock (juce::AudioSampleBuffer&, juce::MidiBuffer&) override {}

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override          { return nullptr; }
    bool hasEditor() const override                        { return false; }

    //==============================================================================
    const juce::String getName() const override                  { return {}; }
    bool acceptsMidi() const override                      { return false; }
    bool producesMidi() const override                     { return false; }
    double getTailLengthSeconds() const override           { return 0; }

    //==============================================================================
    int getNumPrograms() override                          { return 0; }
    int getCurrentProgram() override                       { return 0; }
    void setCurrentProgram (int) override                  {}
    const juce::String getProgramName (int) override             { return {}; }
    void changeProgramName (int, const juce::String&) override   {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock&) override       {}
    void setStateInformation (const void*, int) override   {}

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphProcessor)
};

class GraphSource : public GraphProcessor {
public:
	std::shared_ptr<juce::AudioSource> _source;

	GraphSource(const std::shared_ptr<juce::AudioSource> &source)
		: GraphProcessor()
		, _source(source)
	{}

	void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock) override {
		//setPlayConfigDetails(0, 1, sampleRate, estimatedSamplesPerBlock);
		_source->prepareToPlay(sampleRate, estimatedSamplesPerBlock);
	}

	void releaseResources() override {
		_source->releaseResources();
	}

	void processBlock(juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) override {
		DBG("GraphSource#processBlock");
		float input = buffer.getSample(0, 0);
		juce::AudioSourceChannelInfo info(buffer);
		_source->getNextAudioBlock(info);

		DBG("~GraphSource#processBlock input=" + juce::String(input) + " output=" + juce::String(buffer.getSample(0, 0)));
	}
};

class GraphDeviceSink : public GraphProcessor {
public:
	std::shared_ptr<AudioIoDevice> _sink;

	GraphDeviceSink(const std::shared_ptr<AudioIoDevice> &device)
		: GraphProcessor()
		, _sink(device)
	{}

	void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock) override {
		//setPlayConfigDetails(1, 2, sampleRate, estimatedSamplesPerBlock);
		//_source->prepareToPlay(sampleRate, estimatedSamplesPerBlock);
	}

	void releaseResources() override {
		//_source->releaseResources();
	}

	void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override {
		DBG("GraphDeviceSink#processBlock [0][0]=" + juce::String(buffer.getSample(0, 0)));
		_sink->outputBuffer(buffer);
	}
};

/*
class GraphDeviceSink : public juce::AudioProcessorGraph::AudioGraphIOProcessor {
public:
	std::shared_ptr<AudioIoDevice> _sink;

	GraphDeviceSink(const std::shared_ptr<AudioIoDevice> &device)
		: juce::AudioProcessorGraph::AudioGraphIOProcessor(juce::AudioProcessorGraph::AudioGraphIOProcessor::IODeviceType::audioOutputNode)
		, _sink(device)
	{}

	void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock) override {
		//_source->prepareToPlay(sampleRate, estimatedSamplesPerBlock);
	}

	void releaseResources() override {
		//_source->releaseResources();
	}

	void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override {
		DBG("GraphDeviceSink#processBlock [0][0]=" + juce::String(buffer.getSample(0, 0)));
		_sink->outputBuffer(buffer);
	}
};
*/

} // namespace ear
