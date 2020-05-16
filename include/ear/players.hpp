#ifndef EAR_PLAYERS_HPP
#define EAR_PLAYERS_HPP

namespace ear {

class AudioSourcePlayer : public juce::AudioSourcePlayer {
private:
	std::shared_ptr<AudioSource> _source{nullptr};

public:
	AudioSourcePlayer(const std::shared_ptr<AudioSource>& source)
		: _source(source)
	{
		setSource(_source.get());
	}

	virtual ~AudioSourcePlayer() = default;
};

} // namespace ear

#endif
