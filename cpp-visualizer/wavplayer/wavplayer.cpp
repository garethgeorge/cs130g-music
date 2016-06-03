// stl Libraries
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <stdint.h>
#include <climits>
#include <math.h>

// sfml Libraries
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

// boost Libraries
#include <boost/program_options.hpp>

// My Libraries
#include "fft.h"

namespace po = boost::program_options;
using namespace std;

int main(int argc, const char** argv) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,H", "produce help message")
        ("file,F", po::value< vector<string> >(), "set the file(s) to play.")
    ;

    po::positional_options_description p;
    p.add("file", -1);

    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv).
        options(desc).positional(p).run()
        , vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    if (vm.count("file")) {

        vector<string> files = vm["file"].as< vector<string> >();
        std::cout << "play list: " << std::endl;
        for (int i = 0; i < files.size(); ++i) {
            std::cout << "\t" << i + 1 << ") " << files[i] << std::endl;
        }

        std::cout << "playing... " << std::endl;
        for (auto file : files) {
            std::cout << "\t play file: " << file << std::endl;
            sf::SoundBuffer buffer;
            if (!buffer.loadFromFile(file.c_str()))
                return -1;

            const int16_t* samples = buffer.getSamples();
            int sampleCount = buffer.getSampleCount();
            int channelCount = buffer.getChannelCount();
            int sampleRate = buffer.getSampleRate();

            sf::Sound sound;
            sound.setBuffer(buffer);
            sound.play();

            sf::RenderWindow window(sf::VideoMode(1200, 1000), "Visualizer");

            while (window.isOpen()) {
                if (sound.getStatus() != sf::Music::Playing) {
                    window.close();
                    break ;
                }
                
                sf::Event event;
                while (window.pollEvent(event)) {
                    if (event.type == sf::Event::Closed) {
                        window.close();
                        sound.stop();
                    } else if (event.type == sf::Event::Resized) {
                        window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
                    }
                }
                
                // index of the current sample
                int sampleIndex = sound.getPlayingOffset().asSeconds() * sampleRate * channelCount;
                
                const int BUCKETS = 1024;
                if (sampleIndex + channelCount * BUCKETS > sampleCount) continue ;
                
                Complex samplesIn[BUCKETS];
                Complex fftOut[BUCKETS];
                for (int i = 0; i < BUCKETS; ++i) {
                    samplesIn[i] = Complex(samples[sampleIndex + i * channelCount] / ((float) UINT16_MAX), 0);
                }

                FFT<BUCKETS>::run(samplesIn, fftOut);

                window.clear(sf::Color::Black);
                float w = window.getSize().x;
                float h = window.getSize().y;
                float barWidth = w / ((float) BUCKETS);

                for (int i = 0; i < BUCKETS; ++i) {
                    float energy = std::abs(fftOut[i]) / 10.0f;
                    sf::RectangleShape rect;
                    rect.setSize(sf::Vector2f(barWidth, h * energy));
                    rect.setPosition(barWidth * i, h - h * energy);
                    rect.setFillColor(sf::Color::White);
                    window.draw(rect);
                }
                
                window.display();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000/60));
            }
        }

        /*
        sf::SoundBuffer buffer;
        if (!buffer.loadFromFile("sound.wav")) {
            std::cout << "failed to load sound.wav" << std::endl;
            return -1;
        }
        */
    } else {
        std::cout << "No files provided. Flag --help for help information\n";
        return 0;
    }
}
