/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LFOVisualizer.h"

//==============================================================================
/**
*/ 
class GlitchPluginAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Slider::Listener
{
public:
    GlitchPluginAudioProcessorEditor (GlitchPluginAudioProcessor&);
    ~GlitchPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void sliderValueChanged(juce::Slider* slider) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    juce::TextButton toggleStutterButton{ "Stutter On" };
    juce::Slider stutterDurationSlider;
    juce::Slider repeatSlider;
    juce::Slider ratioSlider;

    juce::Slider stutterDurationModSlider;
    juce::Slider repeatModSlider;
    juce::Slider ratioModSlider;


    juce::Label stutterDurationLabel;
    juce::Label repeatLabel;
    juce::Label ratioLabel;

    juce::LookAndFeel_V4 lookAndFeel;

    LFOVisualizer lfo;
    GlitchPluginAudioProcessor& audioProcessor;

    void createModSlider(juce::Slider& slider);
    void createStutterParameterSlider(juce::Slider& slider, float min, float max, float defaultVal, double interval, juce::String suffix, int decimals, juce::Label& label, juce::String labelText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlitchPluginAudioProcessorEditor)
};
