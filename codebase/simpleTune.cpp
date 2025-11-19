#include <Bela.h>
#include <cmath>
#include <vector>

// Example dataset: counts of buses per minute at different spots
std::vector<float> data = {2, 5, 7, 3, 9, 4, 6};

// Audio state
float phase = 0;
float frequency = 220.0;
int currentIndex = 0;
int samplesPerEvent;
int counter = 0;

bool setup(BelaContext *context, void *userData)
{
    // Each data point will last half a second
    samplesPerEvent = context->audioSampleRate * 0.5;
    return true;
}

void render(BelaContext *context, void *userData)
{
    for(unsigned int n = 0; n < context->audioFrames; n++) {
        // Every samplesPerEvent, move to next data point
        if(counter++ >= samplesPerEvent) {
            counter = 0;
            currentIndex = (currentIndex + 1) % data.size();
            // Map data (e.g. bus count 0–10) to pitch 200–800 Hz
            float value = data[currentIndex];
            frequency = 200.0 + value * 60.0;
        }

        // Simple sine oscillator
        float out = sinf(phase * 2.0f * M_PI);
        phase += frequency / context->audioSampleRate;
        if(phase >= 1.0)
            phase -= 1.0;

        for(unsigned int ch = 0; ch < context->audioOutChannels; ch++) {
            audioWrite(context, n, ch, out * 0.2);
        }
    }
}

void cleanup(BelaContext *context, void *userData) {}
