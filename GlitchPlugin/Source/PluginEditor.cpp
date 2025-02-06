/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GlitchPluginAudioProcessorEditor::GlitchPluginAudioProcessorEditor (GlitchPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), lfo(p, p.getLFO())
{
    lookAndFeel.setColour(juce::ResizableWindow::backgroundColourId, juce::Colours::black);
    lookAndFeel.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    setLookAndFeel(&lookAndFeel);


    toggleStutterButton.setToggleState(false, false);
    toggleStutterButton.onClick = [&]()
        {
            toggleStutterButton.setToggleState(!toggleStutterButton.getToggleState(), false);
            toggleStutterButton.setButtonText(toggleStutterButton.getToggleState() ? "Stutter Off" : "Stutter On");
            audioProcessor.setStutterState(toggleStutterButton.getToggleState());
        };
    addAndMakeVisible(toggleStutterButton);

    createStutterParameterSlider(stutterDurationSlider, 20.f, 1000.f, 500, 0.0, " ms", 1, stutterDurationLabel, "Size");
    createStutterParameterSlider(repeatSlider, 1, 10, 3, 0.0, "", 0, repeatLabel, "Repeats");
    createStutterParameterSlider(ratioSlider, -2.0, 2.0, 1.0, 0.1, "", 1, ratioLabel, "Ratio");

    createModSlider(stutterDurationModSlider);
    createModSlider(repeatModSlider);
    createModSlider(ratioModSlider);

    addAndMakeVisible(lfo);
    setSize (600, 400);
}

GlitchPluginAudioProcessorEditor::~GlitchPluginAudioProcessorEditor()
{
    repeatSlider.removeListener(this);
    stutterDurationSlider.removeListener(this);
    ratioSlider.removeListener(this);
    setLookAndFeel(nullptr);
}

//==============================================================================
void GlitchPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
}

void GlitchPluginAudioProcessorEditor::resized()
{
    int effectAreaHeight = 200;
    int lfoAreaHeight = 200;
    toggleStutterButton.setBounds(50, effectAreaHeight * 0.25, 80, 50);
    
    float durationSliderX = (getWidth() / 4 * 2) - 120;
    stutterDurationSlider.setBounds(durationSliderX , effectAreaHeight * 0.2, 80, 80);
    stutterDurationModSlider.setBounds(durationSliderX + 16, effectAreaHeight * 0.8, 45, 45);

    float repeatSliderX = (getWidth() / 4 * 3) - 120;
    repeatSlider.setBounds(repeatSliderX, effectAreaHeight * 0.2, 80, 80);
    repeatModSlider.setBounds(repeatSliderX + 16, effectAreaHeight * 0.8, 45, 45);

    float ratioSliderX = (getWidth() / 4 * 4) - 120;
    ratioSlider.setBounds(ratioSliderX, effectAreaHeight * 0.2, 80, 80);
    ratioModSlider.setBounds(ratioSliderX + 16, effectAreaHeight * 0.8, 45, 45);


    lfo.setBounds(0, effectAreaHeight + 10, getWidth(), lfoAreaHeight - 10);
}

void GlitchPluginAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &stutterDurationSlider) {
        audioProcessor.setStutterDuration(stutterDurationSlider.getValue());
    }
    else if (slider == &repeatSlider) {
        audioProcessor.setStutterRepeats(int(repeatSlider.getValue()));
    }
    else if (slider == &ratioSlider) {
        auto value = ratioSlider.getValue();

        if (value != 0.0) {
            auto ratio = 1 / value;
            audioProcessor.setPlaybackRatio(ratio);
        }
    }
    else if (slider == &stutterDurationModSlider) {
        audioProcessor.durationModDepth = stutterDurationModSlider.getValue();
    }
    else if (slider == &repeatModSlider) {
        audioProcessor.repeatModDepth = repeatModSlider.getValue();
    }
    else if (slider == &ratioModSlider) {
        audioProcessor.ratioModDepth = ratioModSlider.getValue();
    }
}

void GlitchPluginAudioProcessorEditor::createModSlider(juce::Slider& slider)
{
    slider.setRange(0, 1);
    slider.setValue(0.0);
    slider.setNumDecimalPlacesToDisplay(1);
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 60, 20);
    slider.addListener(this);
    addAndMakeVisible(slider);
}

void GlitchPluginAudioProcessorEditor::createStutterParameterSlider(juce::Slider& slider, float min, float max, float defaultVal, double interval, juce::String suffix, int decimals, juce::Label& label, juce::String labelText)
{
    slider.setRange(min, max, interval);
    slider.setValue(defaultVal);
    if(suffix.isNotEmpty()) slider.setTextValueSuffix(suffix);
    slider.setNumDecimalPlacesToDisplay(decimals);
    slider.addListener(this);
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(slider);

    label.setText(labelText, juce::dontSendNotification);
    label.attachToComponent(&slider, false);
    addAndMakeVisible(label);
}
