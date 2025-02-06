/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "LFO.h"
#include "StutterBuffer.h"

//==============================================================================
/**
*/
class GlitchPluginAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    GlitchPluginAudioProcessor();
    ~GlitchPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    void setStutterState(bool state);
    void setStutterDuration(float durationInMs);
    void setStutterRepeats(int repeats);
    void setPlaybackRatio(double ratio);

    LFO& getLFO() { return lfo; }
    void enableLFO(bool shouldBeEnabled);

    double durationModDepth = 0.0;
    double repeatModDepth = 0.0;
    double ratioModDepth = 0.0;
private:
    //==============================================================================
    StutterBuffer stutterBuffer;
    LFO lfo;

    std::atomic<float> stutterDuration{ 500.f };
    bool shouldRecordStutter = false;
    bool stutterDurationUpdated = false;

    int sr;
    int convertMsToSamples(float ms);

    juce::Optional<juce::AudioPlayHead::PositionInfo> playheadInfo;
    bool isPlaying = false;

    void modulateStutterParameters();
    void updatePositionInfoForLFO(juce::AudioPlayHead* playhead);


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlitchPluginAudioProcessor)
};
