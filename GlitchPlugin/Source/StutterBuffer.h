/*
  ==============================================================================

    StutterBuffer.h
    Created: 29 Nov 2024 11:57:00am
    Author:  laura

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


class StutterBuffer
{
public:
    StutterBuffer();

    void prepareToPlay(int channels, int maximumStutterSize, int maxIndex);
    void process(juce::AudioBuffer<float>& buffer);

    void setStutterDurationInSamples(int numSamples);
    int getStutterDuration() { return maxStutterIndex; }

    void setStutterRepeats(float repeats);
    int getStutterRepeats() { return stutterRepeats; }
    void setStutterState(bool state) { stutterState.set(state); }

    void setRatio(float playbackRatio);
    double getRatio() { return ratio; }

    //getter and setter of original values of parameters that have to be set when changing the parameters via lfo
    int getOrigDuration() { return origDuration; }
    void setOrigDuration(int duration) { origDuration = duration; }
    int getOrigRepeats() { return origRepeats; }
    void setOrigRepeats(int repeats) { origRepeats = repeats; }
    double getOrigRatio() { return origRatio; }
    void setOrigRatio(double ratio) { origRatio = ratio; }

private:
    juce::AudioBuffer<float> ringBuffer;
    juce::AudioBuffer<float> stutterBuffer;

    juce::Atomic<bool> copyStutterToggle{ false };
    juce::Atomic<bool> stutterState{ false }; 

    //TODO: set with apvts parameters
    int ringWriteIndex{ 0 };
    int stutterReadIndex{ 0 }; 
    float maxStutterIndex{ 0 };
    float stutterRepeats{ 5 };
    int currentRepeat{ 0 };
    float ratio{ 1.0f };

    //used to store target values when ramping parameters
    float targetDuration;
    float targetRepeats;
    float targetRatio;

    //used to store an increment to use when ramping the parameters
    float durRampIncrement;
    float repeatsRampIncrement;
    float ratioRampIncrement;

    //used to modify parameters with an LFO object
    int origDuration;
    int origRepeats;
    double origRatio;

    const int numFadeSamples{ 500 };

    void pushBuffer(juce::AudioBuffer<float>& buf);
    void copyStutter();
    float getInterpolatedSample(int channel, int currentIndex);
    bool anyParameterUpdated(int numSamples);
    void rampParameters();
    void applyFade();
    float getCrossfadeSample(int chan, int index, float sample);
};