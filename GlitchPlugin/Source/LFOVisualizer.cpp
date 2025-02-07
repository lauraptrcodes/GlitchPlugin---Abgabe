/*
  ==============================================================================

    LFO.cpp
    Created: 23 Oct 2024 2:32:13pm
    Author:  laura

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LFOVisualizer.h"

//==============================================================================
LFOVisualizer::LFOVisualizer(GlitchPluginAudioProcessor& p, LFO& l) : audioProcessor(p), lfo(l)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

    lfoToggle.setButtonText("LFO On/Off");
    lfoToggle.setToggleState(false, false);
    lfoToggle.onClick = [&]()
    {
        enableLFO(); 
    };
    addAndMakeVisible(lfoToggle);

    syncToggle.setButtonText("Sync to beat");
    syncToggle.setToggleState(false, false);
    syncToggle.onClick = [&]()
    {
        toggleSync();
    };
    addAndMakeVisible(syncToggle);


    syncSlider.setRange(-2, 4);
    syncSlider.setNumDecimalPlacesToDisplay(0);
    syncSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    syncSlider.setTextBoxIsEditable(false);
    syncSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 20);
    syncSlider.textFromValueFunction = [](double value) 
    {
            double valueInBeats = pow(2, round(value));

            if (value >= 2) {
                return juce::String(valueInBeats / 4) + " bars";
            }
            return "1 / " + juce::String(4 / valueInBeats);
    };
    syncSlider.onValueChange = [&]() 
    {
            float factor = 1 / (pow(2, round(syncSlider.getValue())));
            audioProcessor.getLFO().setSyncFactor(factor);
    };
    syncSlider.setValue(1);
    addAndMakeVisible(syncSlider);
    
    frequencySlider.setRange(0.1, 1.0);
    frequencySlider.setTextValueSuffix("Hz");
    frequencySlider.setValue(0.5);
    frequencySlider.setNumDecimalPlacesToDisplay(1);
    frequencySlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    frequencySlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 20);
    frequencySlider.addListener(this);
    addAndMakeVisible(frequencySlider);

    frequencyLabel.setText("Freq", juce::dontSendNotification);
    frequencyLabel.attachToComponent(&frequencySlider, true);
    addAndMakeVisible(frequencyLabel);

    currentLFOValue = audioProcessor.getLFO().getCurrentValue();
}

LFOVisualizer::~LFOVisualizer()
{
    modDepthSlider.removeListener(this);
}

void LFOVisualizer::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    juce::Rectangle<float> lfoVisualization(visualizationStartX, visualizationStartY, visualizationWidth, visualizationHeight);
    g.setColour(juce::Colours::darkgrey);
    g.fillRoundedRectangle(lfoVisualization, 8.f);

    g.setColour(juce::Colours::white);
    if (currentLFOValue) {
        float y = juce::jmap(currentLFOValue, 0.0f, 1.0f, visualizationStartY, visualizationStartY + visualizationHeight);
        g.fillRect(visualizationStartX, y, visualizationWidth, 4.f);
    }
}

void LFOVisualizer::resized()
{    
    visualizationStartX = getWidth() - 140;
    visualizationStartY = 30.f;
    visualizationWidth = 20.f;
    visualizationHeight = getHeight() - 40;

    float lfoX = getWidth() / 4;
    lfoToggle.setBounds(lfoX, 20, 100, 50);
    float syncX = getWidth() / 4 * 2;
    syncToggle.setBounds(syncX, 20, 120, 50);
    float sliderY = 80.f;
    frequencySlider.setBounds(lfoX, sliderY, 80, 70);
    syncSlider.setBounds(syncX, sliderY, 80, 70);
}

void LFOVisualizer::enableLFO()
{
    lfoToggle.setToggleState(!lfoToggle.getToggleState(), false);
    lfoEnabled = !lfoEnabled;
    this->audioProcessor.enableLFO(lfoEnabled);

    if (lfoEnabled) {
        startTimerHz(60);
    }
    else {
        stopTimer();
    }

}

void LFOVisualizer::toggleSync()
{
    audioProcessor.getLFO().toggleSync();
}

void LFOVisualizer::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &frequencySlider) {
        lfo.setFreq(slider->getValue());
    }
}

void LFOVisualizer::timerCallback()
{
    currentLFOValue = lfo.getCurrentValue();
    repaint();
}
