#ifndef EAR_BUFFER_HPP
#define EAR_BUFFER_HPP

namespace ear {
	
class CircularAudioBuffer {
	uint32_t samplesPerBlock{512};
	uint16_t channels;
	uint32_t blocks;

	uint64_t head{0};
	uint64_t tail{0};

	juce::HeapBlock<char> heap;
	juce::dsp::AudioBlock<float> buffer;

public:
	CircularAudioBuffer(uint16_t channels_, uint32_t blocks_)
		: channels(channels_)
		, blocks(blocks_)
		, buffer(heap, channels_, samplesPerBlock * blocks_)
	{}

	juce::dsp::AudioBlock<float> setHead(uint64_t block) {
		head = block + 1;
		if (head >= blocks) {
			tail = head - blocks;
		} else {
			tail = 0;
		}

		return operator[](block);
	}

	uint64_t getBufferCount() {
		return head - tail;
	}

	size_t index(uint64_t block) const {
		if (block >= head || block < tail) {
			throw std::out_of_range("Index is out of range");
		}

		return block % blocks;
	}

	juce::dsp::AudioBlock<float> operator[](uint64_t block) {
		return buffer.getSubBlock(index(block) * samplesPerBlock, samplesPerBlock);
	}
};

} // ear namespace

#endif
