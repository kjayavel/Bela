#include <Bela.h>
#include <libraries/AudioFile/AudioFile.h>
#include <cmath>
#include <vector>

// Example dataset: pedestrian counts per location (0–10 scale)
std::vector<int> pedestrianDensity = {1, 3, 6, 2, 8, 4, 9, 5};

unsigned int currentIndex = 0;

// Drum sample buffer (multi-channel)
std::vector<std::vector<float>> drum;
int drumPosition = -1; // -1 means not currently playing


// Beat timing
int samplesUntilNextBeat = 0;

bool setup(BelaContext *context, void *userData)
{
   drum = AudioFileUtilities::load("drum.wav");

	if(drum.size() == 0 || drum[0].size() == 0) {
    	rt_printf("Error: couldn't load drum sample\n");
    	return false;
}
    return true;
}

void render(BelaContext *context, void *userData)
{
    for(unsigned int n = 0; n < context->audioFrames; n++) {

        if(samplesUntilNextBeat <= 0) {
            // Trigger drum
            drumPosition = 0;

            // Get current density value
            int density = pedestrianDensity[currentIndex];

            // Map density to tempo:
            // d=0 → ~1 beat/s, d=10 → ~10 beat/s
            float beatsPerSecond = 1.0 + density * 0.9f;
            float secondsPerBeat = 1.0f / beatsPerSecond;
            samplesUntilNextBeat = (int)(secondsPerBeat * context->audioSampleRate);

            // Step dataset index
            currentIndex = (currentIndex + 1) % pedestrianDensity.size();
        }

        float out = 0.0f;

        // If drum is playing, fetch sample
        if(drumPosition >= 0 && drumPosition < (int)drum[0].size()) {
    		out = drum[0][drumPosition];
            drumPosition++;
        } else {
            drumPosition = -1; // finished
        }

        samplesUntilNextBeat--;

        // Write to all channels
        for(unsigned int ch = 0; ch < context->audioOutChannels; ch++) {
            audioWrite(context, n, ch, out * 0.8f);
        }
    }
}

void cleanup(BelaContext *context, void *userData) {}
