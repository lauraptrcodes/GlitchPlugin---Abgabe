/*
  ==============================================================================

    LFO.h
    Created: 24 Oct 2024 11:53:46am
    Author:  laura

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>
#include <numbers>

class LFO 
{
public:
    LFO();
    ~LFO();

    enum WaveType
    {
        Sine,
        Square,
        Random
    };

    void updateLFOState(int bufferSize);
    void setPhase(double phase);
    void setFreq(double freq);
    void setSampleRate(double sr);
    void setSyncFactor(float factor);
    void setBpm(double bpm);
    void toggleSync();
    bool syncEnabled = false;
    double syncFactor = 0.5;

    void enableLFO(bool shoudBeEnabled);
    float getCurrentValue();
    bool isEnabled = false;
    double freq;
    
private: 
    float currentValue = 0.0f;
    double phase = 0.0;
    double sampleRate;
    double bpm = 0.0;
    WaveType waveType;

    double syncedFreq();
};

#pragma once
