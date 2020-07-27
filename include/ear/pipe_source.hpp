#ifndef EAR_PIPE_SOURCE_HPP
#define EAR_PIPE_SOURCE_HPP

#include "buffer.hpp"
#include "sources.hpp"
#include "processor.hpp"

#include <boost/asio.hpp>
//#include <boost/process.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/endian/buffers.hpp>

#include <functional>

namespace ear {

class PipeSource : public AudioSource {
	std::function<void (const boost::system::error_code, std::size_t)> _bind;

	juce::CriticalSection _mutex;

	std::string _file;
	std::thread _thread;
	//boost::process::async_pipe _pipe;
	//boost::asio::posix::stream_descriptor _source;
	static constexpr unsigned BUFFER_SIZE = 4096;
	uint8_t _buffer[BUFFER_SIZE];
	boost::asio::streambuf _dynamic;

	std::atomic<bool> _reading{false};

	void read_callback(const boost::system::error_code&, std::size_t);
/*
	void _doRead() {
		DBG("reading");
		boost::asio::mutable_buffer buffer(_buffer, BUFFER_SIZE);
		boost::asio::async_read(_source, buffer, boost::asio::transfer_at_least(1), _bind);
	}
*/
	void _run() {
		bool reading = _reading;
		//DBG("run -- " + juce::String(reading?"reading":"not reading"));

		//boost::asio::streambuf dynamic;

		uint8_t buffer[4096];

		while (_reading) {
			try {
				juce::File file(_file);
				juce::FileInputStream stream(file);
				if (stream.openedOk()) {
					while (_reading) {
						int size = stream.read(buffer, 4096);
						if (size > 0) {
							//DBG(juce::String("read ") << size << " bytes");
							processRead(buffer, size);
						} else {
							std::this_thread::sleep_for(std::chrono::milliseconds(1));
						}
					}
				} else {
					DBG("Stream not opened; sleeping for 5 seconds before trying again");
					std::this_thread::sleep_for(std::chrono::seconds(5));
				}
			} catch (const std::runtime_error&) {
				DBG("runtime_error, sleeping for 5 seconds before reopneing file");
				std::this_thread::sleep_for(std::chrono::seconds(5));
			}
		}

		//DBG("exiting _run");
	}

protected:
	virtual void processRead(const uint8_t*, std::size_t) = 0;

public:
	PipeSource(const std::string& file)
		: _bind{std::bind(&PipeSource::read_callback, this, std::placeholders::_1, std::placeholders::_2)}
		, _file(file)
		//, _source(context)
		//, _pipe(context, file)
	{
/*		int read_fd = open(file.c_str(), O_RDWR);
		if (read_fd == -1) {
			//boost::process::detail::throw_last_error();
			throw std::runtime_error("Could not open file");
		}

		_source.assign(read_fd);
*/
	}

	void start_reading() {
		//DBG("start_reading");
		juce::ScopedLock lock(_mutex);
		bool wasReading = _reading.exchange(true);

		if (!wasReading) {
			//DBG("starting to read");
			//boost::asio::mutable_buffer buffer(_buffer, BUFFER_SIZE);
			//boost::asio::async_read_some(_pipe, buffer, boost::asio::transfer_at_least(1), _bind);

			//_doRead();

			_thread = std::thread(&PipeSource::_run, this);
			/*] () {
				DBG("thread starting");
				_run();
				DBG("thread done");
			});*/
		}
	}

	void stop_reading() {
		DBG("stop_reading");
		juce::ScopedLock lock(_mutex);
		bool wasReading = _reading.exchange(false);

		if (wasReading) {
			//_source.cancel();

			DBG("thread join");
			_thread.join();
			DBG("thread joined");
		}
	}
};

void PipeSource::read_callback(const boost::system::error_code &ec, std::size_t size) {
	if (ec) {
		DBG(juce::String("boost error code") << ec.message());
	} else {
		if (size > 0) {
			DBG(juce::String("!!!!!!! read ") << size << " bytes");

			processRead(_buffer, size);
		}
	}

	if (_reading) {
		//boost::asio::mutable_buffer buffer(_buffer, BUFFER_SIZE);
		//boost::asio::async_read_some(_pipe, buffer, boost::asio::transfer_at_least(1), _bind);
		//_doRead();
	}
}

class AirplaySource : public PipeSource {
private:
	//Buffer _buffer;
	//static constexpr unsigned FRAMES_TO_BUFFER = 10;
	//static constexpr unsigned BUFFER_SIZE = 512 * 2 * FRAMES_TO_BUFFER;
	unsigned _framesToBuffer;
	unsigned _bufferSize;
	//float _buffer[BUFFER_SIZE];
	std::unique_ptr<float[]> _buffer{nullptr};

	int _writeIndex = 0;
	int _readIndex = 0;
	int buffering{0};// = //FRAMES_TO_BUFFER / 2;
	juce::CriticalSection _mutex;
	uint64_t writeCount{0}, readCount{0}, writeFrames{0};

	void processRead(const uint8_t* buffer, std::size_t size) override {
		assert(size % 4 == 0);

		static constexpr float SCALE = 1.0f / 0x7fff;

		const int SAMPLES = size / 2;
		uint16_t* buf = (uint16_t*)buffer;

		//int startIndex = _writeIndex;

		//std::cout << "buf ";
		juce::ScopedLock lock(_mutex);
		{
			for (int i = 0; i < SAMPLES; ++i) {
				uint16_t value = boost::endian::little_to_native(buf[i]);
				float sample = SCALE * (short)value;
				_buffer[_writeIndex++] = sample;
				writeCount++;

				if (_writeIndex >= _bufferSize) {
					_writeIndex = 0;
				}
			}
			//std::cout << std::endl;

			DBG(juce::String() << "input  writeIndex=" << _writeIndex << ", readIndex=" << _readIndex << ", writeCount=" << writeCount << ", readCount=" << readCount << ", readFrameCount=" << writeFrames << ", buffering=" << buffering);
		}
	}

public:
	AirplaySource(const std::string& file, unsigned framesToBuffer)
		: PipeSource(file)
		, _framesToBuffer(framesToBuffer)
		, _bufferSize(512 * 2 * framesToBuffer)
	{
		_buffer = std::make_unique<float[]>(_bufferSize);

		DBG(juce::String("Created AirplaySource from file=") + juce::String(file) + juce::String(" with framesToBuffer=") + juce::String(framesToBuffer));
	}

	void prepareToPlay(int samples, double rate) override {
		PipeSource::prepareToPlay(samples, rate);

		//DBG(juce::String("prepareToPlay") << " channels=" << channels << " samples=" << samples);

		start_reading();
	}

	void releaseResources() override {
		stop_reading();
	}

	void getNextAudioBlock (const juce::AudioSourceChannelInfo& buffer) override {
		/*DBG(juce::String("AirplaySource#getNextAudioBlock write=") << _writeIndex
			<< ", read=" << _readIndex
			<< ", numChannels=" << buffer.buffer->getNumChannels()
			<< ", startSample=" << buffer.startSample
			<< ", numSamples=" << buffer.numSamples
		);*/

		buffer.buffer->clear(0, buffer.startSample, buffer.numSamples);
		buffer.buffer->clear(1, buffer.startSample, buffer.numSamples);

		int startIndex = _readIndex;

		juce::ScopedLock lock(_mutex);

		++writeFrames;

		if (buffering > 0) {
			if (_readIndex == _writeIndex) {
				// do nothing
			} else {
				--buffering;
			}
		} else {
			for (int sample = buffer.startSample; sample < buffer.startSample + buffer.numSamples; ++sample) {
				for (int channel = 0; channel < 2; ++channel) {
					if (_readIndex == _writeIndex) {
						// caught up to writes, so start buffering again
						buffering = _framesToBuffer / 2;
					} else {
						buffer.buffer->setSample(channel, sample, _buffer[_readIndex++]);
						if (_readIndex >= _bufferSize) {
							_readIndex = 0;
						}

						++readCount;
					}
				}
			}

			DBG(juce::String() << "output writeIndex=" << _writeIndex << ", readIndex=" << _readIndex << ", writeCount=" << writeCount << ", readCount=" << readCount << ", readFrameCount=" << writeFrames << ", buffering=" << buffering);
		}
	}
};

class AirplayProcessor : public GraphSource {
private:
	AirplaySource _source;

public:
	AirplayProcessor(const std::string& file, unsigned bufferSize)
		: GraphSource(&_source, juce::AudioProcessor::BusesProperties().withOutput("main", juce::AudioChannelSet::stereo()))
		, _source(file, bufferSize)
	{}
};

} // ear namespace

#endif
