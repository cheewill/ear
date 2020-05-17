#include "ear/graph.hpp"
#include "ear/sources.hpp"
#include "ear/pipe_source.hpp"

#include <boost/program_options.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>

#include <thread>

int run(const boost::program_options::variables_map& vm) {
	std::cout << "Server run " << std::endl;

	auto devices = ear::AudioIoDevice::getAllDevices();
	std::shared_ptr<ear::AudioIoDevice> device(nullptr);

	if (vm.count("device")) {
		std::string string = vm["device"].as<std::string>();

		try {
			boost::uuids::string_generator generator;
			boost::uuids::uuid uuid = generator(string);

			device = ear::AudioIoDevice::getDevice(uuid);
		} catch (const std::runtime_error&) {}

		if (device == nullptr) {
			int index = atoi(string.c_str());
			if (string == std::to_string(index) && index < devices.size()) {
				device = devices[index];
			}
		}
	}

	if (device == nullptr && devices.size() > 0) {
		device = devices[0];
	}

	int i = 0;
	for (auto& d : devices) {
		std::cout << ((d == device) ? "*" : " ")
			<< std::right << std::setw(3) << (i++) << ") "
			<< std::left << std::setw(20) << d->getName()
			<< d->getUuid()
			<< " (" << d->getTypeName() << ") " << std::endl;
	}

	if (device == nullptr) {
		std::cerr << "No device" << std::endl;
		return 1;
	}

	ear::AudioGraphStandalone graph;
	auto outputNode = graph.setDevice(device);

	std::cout << "Registered output device with graph" << std::endl;
/*
	auto whiteNoiseNode1 = graph.addNode(std::make_unique<ear::WhiteNoiseProcessor>());
	auto whiteNoiseNode2 = graph.addNode(std::make_unique<ear::WhiteNoiseProcessor>());

	assert(graph.addConnection({{whiteNoiseNode1, 0}, {outputNode, 0}}));
	assert(graph.addConnection({{whiteNoiseNode1, 0}, {outputNode, 1}}));
	assert(graph.addConnection({{whiteNoiseNode2, 0}, {outputNode, 1}}));

	std::cout << "Connected white noise to output" << std::endl;
*/

	boost::asio::io_context io_context;
	auto airplayProcessor = std::make_unique<ear::AirplayProcessor>("/tmp/airplay", io_context);
	//airplayProcessor->start_reading();
	auto airplayNode = graph.addNode(std::move(airplayProcessor));

	assert(graph.addConnection({{airplayNode, 0}, {outputNode, 0}}));
	assert(graph.addConnection({{airplayNode, 1}, {outputNode, 1}}));

	std::thread t([&io_context] {

		io_context.run();
	});

	bool isOpen = device->open();
	if (!isOpen) {
		std::cerr << "Failed to open device:" << device->getName() << std::endl;
		return 1;
	}

	device->start();

	while (true) {}

	device->close();

	return 0;
}
