/*
  ==============================================================================

    LFO.cpp
    Created: 24 Oct 2024 11:53:46am
    Author:  laura

  ==============================================================================
*/

#include "LFO.h"

LFO::LFO() : freq(1.0), phase(0.0), sampleRate(44100.0), waveType(Sine), syncEnabled(false), bpm(120.0)
{
}

LFO::~LFO()
{
}

void LFO::updateLFOState(int bufferSize)
{ 

    if (syncEnabled) {
        freq = syncedFreq();
    }

    float lfoPhaseIncrement = float(juce::MathConstants<float>::twoPi * freq) / (float(this->sampleRate / bufferSize));
     
    switch (waveType) {
        case Sine:     currentValue = (std::sin(phase) / 2.f + 0.5f); break;
        case Square:   currentValue = phase < juce::MathConstants<float>::pi ? -1.0f : 1.0f; break;
    }

    phase += lfoPhaseIncrement;

    if (phase >= 2 * juce::MathConstants<float>::pi)
        phase -= 2 * juce::MathConstants<float>::pi;    

}

void LFO::setPhase(double phase)
{
    this->phase = phase;
}

void LFO::setFreq(double freq)
{
    this->freq = freq;
}

void LFO::setSampleRate(double sr)
{
    this->sampleRate = sr;
}

void LFO::setSyncFactor(float factor)
{
    this->syncFactor = factor;
}

void LFO::setBpm(double bpm)
{
    this->bpm = bpm;
}

void LFO::toggleSync()
{
    syncEnabled = !syncEnabled;
}

void LFO::enableLFO(bool shouldBeEnabled)
{
    this->isEnabled = shouldBeEnabled;
}

float LFO::getCurrentValue()
{
    return currentValue;
}

double LFO::syncedFreq()
{
    return bpm / 60.0 * syncFactor; // bei bpm 120 & syncFaktor 0.5 = 1
}
