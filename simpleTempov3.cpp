#include <Bela.h>
#include <libraries/AudioFile/AudioFile.h>
#include <vector>
#include <cmath>

// Dataset: pedestrian density 0-10
std::vector<int> pedestrianDensity = {1, 3, 6, 2, 8, 4, 9, 5};
unsigned int currentIndex = 0;

// Drum buffers (multi-channel)
std::vector<std::vector<float>> kick, snare, hihat;
int kickPos=-1, snarePos=-1, hihatPos=-1;

// Beat timing
int samplesUntilNextBeat = 0;

// Current beat volume
float currentVolume = 1.0f;

// Soft clip for punch
float softClip(float x) { return x / (1.0f + 0.5f*fabsf(x)); }

bool setup(BelaContext *context, void *userData)
{
    kick = AudioFileUtilities::load("kick.wav");
    snare = AudioFileUtilities::load("snare.wav");
    hihat = AudioFileUtilities::load("hihat.wav");

    if(kick.empty() || kick[0].empty() || snare.empty() || snare[0].empty() || hihat.empty() || hihat[0].empty()) {
        rt_printf("Error loading drum samples\n");
        return false;
    }
    return true;
}

void render(BelaContext *context, void *userData)
{
    for(unsigned int n=0; n<context->audioFrames; n++) {

        if(samplesUntilNextBeat <= 0) {

            int density = pedestrianDensity[currentIndex];

            // ---------- VOLUME MAPPING ----------
            currentVolume = powf((density+1)/11.0f, 3.0f); // exponential
            if(density >= 7) currentVolume *= 2.0f;       // accent

            // ---------- TRIGGER DRUMS ----------
            // Kick on strong beats
            kickPos = 0;

            // Snare on medium density
            if(density >= 4) snarePos = 0;

            // Hi-hat always
            hihatPos = 0;

            // ---------- TEMPO ----------
            float beatsPerSecond = 1.0f + density * 0.9f;
            float secondsPerBeat = 1.0f / beatsPerSecond;
            samplesUntilNextBeat = (int)(secondsPerBeat * context->audioSampleRate);

            currentIndex = (currentIndex + 1) % pedestrianDensity.size();
        }

        float out = 0.0f;

        // Kick
        if(kickPos >=0 && kickPos < (int)kick[0].size()) {
            out += kick[0][kickPos]*currentVolume;
            kickPos++;
        } else kickPos=-1;

        // Snare
        if(snarePos >=0 && snarePos < (int)snare[0].size()) {
            out += snare[0][snarePos]*currentVolume;
            snarePos++;
        } else snarePos=-1;

        // Hi-hat
        if(hihatPos >=0 && hihatPos < (int)hihat[0].size()) {
            out += hihat[0][hihatPos]*currentVolume*0.5f; // slightly quieter
            hihatPos++;
        } else hihatPos=-1;

        samplesUntilNextBeat--;

        // Soft clip for punch
        out = softClip(out);

        // Output to all channels
        for(unsigned int ch=0; ch<context->audioOutChannels; ch++)
            audioWrite(context, n, ch, out);
    }
}

void cleanup(BelaContext *context, void *userData) {}
