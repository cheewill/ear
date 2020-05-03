#include <iostream>
#include <thread>

#include <boost/program_options.hpp>

#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_audio_utils/juce_audio_utils.h"

#include "ear/device.hpp"
#include "ear/graph.hpp"

//#include "juce_core/juce_core.h"



int main(int argc, char** argv) {
	std::cout << "Hello, World 2!" << std::endl;

	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
    	("help,h", "produce help message")
		("list-devices", "List audio devices")
		("quit", "Quit on error")
    	;//("compression", boost::program_options::value<int>(), "set compression level");

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);

	if (vm.count("help")) {
		std::cout << desc << "\n";
		return 0;
	}

	if (vm.count("list-devices")) {
		auto devices = ear::AudioIoDevice::getAllDevices();
		{
		    int i = 0;
		    for (auto& device : devices) {
		        std::cout << juce::String(i++) + ": name=" << device->getName() << ", type=" << device->getTypeName() << ", channels=" << device->getOutputChannelCount() << std::endl;
		    }
		}
		return 0;
	}

	auto devices = ear::AudioIoDevice::getAllDevices();
	auto device = devices[0];
	bool isOpen = device->open();

	auto juceDevice = device->getDevice();

	DBG("open device");
	juce::Logger::writeToLog(std::string("open device status=") + (isOpen?"open":"error"));

	juce::AudioProcessorGraph graph;
	graph.setPlayConfigDetails(2, 2, 44100, 512);

	//device->start();

	DBG("started");

	//auto playerNodeId = graph.addNode(std::move(player));



//	graph.setPlayConfigDetails(1, 2, 44100, 512);


	DBG("connected inputs=" + juce::String(graph.getMainBusNumInputChannels()) + " outputs=" + juce::String(graph.getMainBusNumOutputChannels()));
//return 0;
//	std::unique_ptr<juce::AudioProcessorPlayer> player(new juce::AudioProcessorPlayer);
//	device->start(player.get());
//	player->setProcessor(graphSource.get());

	DBG("add nodes");

	std::unique_ptr<juce::AudioProcessor> graphInput = std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(juce::AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode);
	auto graphInputNode = graph.addNode(std::move(graphInput));
	DBG("added GraphInput: " + juce::String(graphInputNode->nodeID.uid));
/*
	std::unique_ptr<juce::AudioProcessor> graphOutput = std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(juce::AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);
	auto graphOutputNode = graph.addNode(std::move(graphOutput));
	DBG("added GraphOutput: " + juce::String(graphOutputNode->nodeID.uid));
*/
	auto juceSource = std::make_shared<ear::FileAudioSource>(juce::File("/Users/matt/Development/ear/media/01 Misery.m4a"));
	std::unique_ptr<juce::AudioProcessor> graphSource = std::make_unique<ear::GraphSource>(juceSource);
	graphSource->setPlayConfigDetails(0, 1, 44100, 512);

	std::unique_ptr<juce::AudioProcessor> graphSink = std::make_unique<ear::GraphDeviceSink>(device);
	graphSink->setPlayConfigDetails(1, 0, 44100, 512);
	auto sinkNodeId = graph.addNode(std::move(graphSink));
	DBG("added GraphDeviceSink: " + juce::String(sinkNodeId->nodeID.uid));

	auto sourceNodeId = graph.addNode(std::move(graphSource));
	DBG("added WhiteNoiseSource: " + juce::String(sourceNodeId->nodeID.uid));

	juce::AudioProcessorGraph::Connection connection({sourceNodeId->nodeID, 0}, {sinkNodeId->nodeID, 0});
	if (!graph.canConnect(connection)) {
		std::cerr << " cannot connect nodes" << std::endl;
		if (vm.count("quit")) {
			return 1;
		}
		//return 1;
	}
	if (!graph.addConnection(connection)) {
		std::cerr << "did not connect nodes" << std::endl;
		if (vm.count("quit")) {
			return 1;
		}
	}

	device->setGraph(&graph);
	device->start();

	juce::AudioBuffer<float> buffer(2, 1000);
	juce::MidiBuffer incomingMidi;
//	for (int i = 0; i < 100; ++i) {
//		std::this_thread::sleep_for(std::chrono::milliseconds(10));
//		graph.processBlock(buffer, incomingMidi);
//	}
	std::this_thread::sleep_for(std::chrono::seconds(20));

	DBG("done sleeping");
//	player->setProcessor(nullptr);
	device->close();

	juce::Logger::writeToLog("done");
}
