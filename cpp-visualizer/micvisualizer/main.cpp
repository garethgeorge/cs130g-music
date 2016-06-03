#include <iostream>
#include <chrono>
#include <thread>
#include <climits>
#include <math.h>
#include <stdint.h>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "fft.h"

namespace po = boost::program_options;
using namespace std;

struct RecorderVisualizer : sf::SoundRecorder {
    const static int bufferSize = 2048;
    int bufferUsed = 0;
    int16_t* buffer = nullptr;
    sf::RenderWindow& renderWindow;

    RecorderVisualizer(sf::RenderWindow& renderWindow) : sf::SoundRecorder(), renderWindow(renderWindow) {
        buffer = new int16_t[bufferSize];
    }

    ~RecorderVisualizer() {
        delete[] buffer;
    }


    virtual bool onStart() override {
        bufferUsed = 0;
        return true;
    }

    virtual bool onProcessSamples(const int16_t* samples, std::size_t sampleCount) override {
        const int16_t* end = samples + sampleCount;
        do {
            for (int i = 0; bufferUsed != bufferSize && samples != end; ++i) {
                buffer[bufferUsed++] = *samples++;
            }
            if (bufferUsed == bufferSize) {
                this->updateGraphics();
                /*std::this_thread::sleep_for(std::chrono::milliseconds((long) ( 1000 * (
                                (float) bufferSize / (float) this->getSampleRate()
                            ))));
                */
                bufferUsed = 0;
            }
        } while (samples != end);
        return true;
    }
    
    virtual void onStop() override {
        bufferUsed = 0;
    }

    virtual void updateGraphics() {
        renderWindow.clear(sf::Color::Black);
        float w = this->renderWindow.getSize().x;
        float h = this->renderWindow.getSize().y;
        
        Complex input[bufferSize];
        for (int i = 0; i < bufferSize; ++i) {
            input[i] = Complex(buffer[i] / ((float) INT16_MAX), 0);
        }
        
        Complex output[bufferSize];
        FFT<bufferSize>::run(input, output);
        
        float barW = w / ((float) bufferSize);
        for (int i = 0; i < bufferSize; ++i) {
            float energy = std::abs(output[i]);
            sf::RectangleShape rect;
            rect.setSize(sf::Vector2f(barW, energy * h));
            rect.setPosition(barW * i, h - energy * h);
            rect.setFillColor(sf::Color::White);
            renderWindow.draw(rect);
        }

        renderWindow.display();
    }
};

int main(int argc, const char** argv) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,H", "produce help message")
        ("device,D", po::value<string>(), "set the device to record from.")
        ("list,L", "list available devices");
    
    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
    } catch (std::exception& e) {
        std::cerr << "failed to parse arguments type --help for usage instructions." << std::endl;
    }

    if (vm.count("help") || argc == 1) {
        std::cout << desc << std::endl;
        return 1;
    }

    if (vm.count("list")) {
        sf::SoundBufferRecorder recorder;
        vector<string> devices = recorder.getAvailableDevices();
        for (auto d : devices)
            std::cout << d << std::endl;
        return 1;
    }

    if (vm.count("device")) {
        sf::RenderWindow window(sf::VideoMode(1200, 800), "Mic-Visualizer");
        
        string device = vm["device"].as<string>();
        const int SAMPLE_COUNT = 2048;
        RecorderVisualizer recorder(window);
        recorder.setDevice(device);
        if (!recorder.isAvailable()) {
            std::cout << "Device \'" << device << "\' is not available." << std::endl;
        }
        std::cout << "Recording on device: " << device << std::endl;
        recorder.start(44100); 
        while (window.isOpen() && recorder.isAvailable()) {
            sf::Event event;

            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    recorder.stop();
                    window.close();
                } else if (event.type == sf::Event::Resized) {
                    window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1000/60));
        }

        return 1;
    }
}
