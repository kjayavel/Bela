#include <Bela.h>
#include <libraries/AudioFile/AudioFile.h>
#include <cmath>
#include <vector>

// Example dataset: pedestrian counts per location (0–10 scale)
std::vector<int> pedestrianDensity = {1, 3, 6, 2, 8, 4, 9, 5};
unsigned int currentIndex = 0;

std::vector<std::vector<float>> drum;
int drumPosition = -1;
int samplesUntilNextBeat = 0;

float currentVolume = 1.0f;

float softClip(float x)
{
    return x / (1.0f + 0.5f * fabsf(x));
}

bool setup(BelaContext *context, void *userData)
{
    drum = AudioFileUtilities::load("drum.wav");

    if(drum.size() == 0 || drum[0].size() == 0) {
        rt_printf("Error loading drum.wav\n");
        return false;
    }

    return true;
}

void render(BelaContext *context, void *userData)
{
    for(unsigned int n = 0; n < context->audioFrames; n++) {

        if(samplesUntilNextBeat <= 0) {
            
            drumPosition = 0;
            int density = pedestrianDensity[currentIndex];

            // ---------- NEW: Very dramatic volume curve ----------
            // Exponential mapping for vivid loudness differences
            currentVolume = powf((density + 1) / 11.0f, 3.0f);

            // Extra accent for very high density
            if(density >= 7)
                currentVolume *= 2.0f;

            // Tempo mapping stays the same
            float beatsPerSecond = 1.0 + density * 0.9f;
            float secondsPerBeat = 1.0f / beatsPerSecond;
            samplesUntilNextBeat = (int)(secondsPerBeat * context->audioSampleRate);

            currentIndex = (currentIndex + 1) % pedestrianDensity.size();
        }

        float out = 0.0f;

        if(drumPosition >= 0 && drumPosition < (int)drum[0].size()) {
            out = drum[0][drumPosition] * currentVolume;
            drumPosition++;
        } else {
            drumPosition = -1;
        }

        samplesUntilNextBeat--;

        // Soft clipping makes loud hits more “punchy”
        out = softClip(out);

        for(unsigned int ch = 0; ch < context->audioOutChannels; ch++) {
            audioWrite(context, n, ch, out);
        }
    }
}

void cleanup(BelaContext *context, void *userData) {}
