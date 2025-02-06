/*
  ==============================================================================

    LFO.h
    Created: 23 Oct 2024 2:32:13pm
    Author:  laura

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class LFOVisualizer  : public juce::Component , public juce::Timer, public juce::Slider::Listener
{
public:
    LFOVisualizer(GlitchPluginAudioProcessor&, LFO&);
    ~LFOVisualizer() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void enableLFO();
    void toggleSync();

    float currentLFOValue{ 0.0f };

    void sliderValueChanged(juce::Slider* slider) override;

private:

    void timerCallback() override;
    float visualizationStartX;
    float visualizationStartY;
    float visualizationWidth;
    float visualizationHeight;
    juce::ToggleButton lfoToggle;
    bool lfoEnabled{ false };
    juce::ToggleButton syncToggle;
    bool syncEnabled{ false };

    juce::Slider modDepthSlider;
    juce::Label modDepthLabel;
    juce::Slider frequencySlider;
    juce::Label frequencyLabel;
    juce::Slider syncSlider;
    GlitchPluginAudioProcessor& audioProcessor;
    LFO& lfo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LFOVisualizer)

};
