/*
  ==============================================================================

    This file contains the basic framework cod
    2e for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GlitchPluginAudioProcessor::GlitchPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

GlitchPluginAudioProcessor::~GlitchPluginAudioProcessor()
{
}

//==============================================================================
const juce::String GlitchPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GlitchPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GlitchPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GlitchPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GlitchPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GlitchPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GlitchPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GlitchPluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String GlitchPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void GlitchPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void GlitchPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    stutterBuffer.prepareToPlay(getTotalNumInputChannels(), sr, static_cast<int>((stutterDuration.load() / 1000.f) * sr));
    lfo.setSampleRate(sampleRate);
}

void GlitchPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    //stutterBuffer.clear();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GlitchPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void GlitchPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    lfo.updateLFOState(buffer.getNumSamples());

    juce::AudioPlayHead* playhead = getPlayHead();
    if (playhead) {
        isPlaying = playhead->getPosition()->getIsPlaying();
    }

    if (isPlaying) {
        if (lfo.isEnabled) {
            updatePositionInfoForLFO(playhead);
            lfo.updateLFOState(buffer.getNumSamples());
            modulateStutterParameters();
        }
        stutterBuffer.process(buffer);
    }
}

//==============================================================================
bool GlitchPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GlitchPluginAudioProcessor::createEditor()
{
    return new GlitchPluginAudioProcessorEditor (*this);
}

//==============================================================================
void GlitchPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void GlitchPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void GlitchPluginAudioProcessor::setStutterState(bool state)
{
    stutterBuffer.setStutterState(state);
}

void GlitchPluginAudioProcessor::setStutterDuration(float durationInMs)
{
    if (lfo.isEnabled) stutterBuffer.setOrigDuration(convertMsToSamples(durationInMs));

    int numSamples = convertMsToSamples(durationInMs);
    stutterBuffer.setStutterDurationInSamples(numSamples);
}

void GlitchPluginAudioProcessor::setStutterRepeats(int repeats)
{  
    if (lfo.isEnabled) stutterBuffer.setOrigRepeats(repeats);
    stutterBuffer.setStutterRepeats(repeats);
}

void GlitchPluginAudioProcessor::setPlaybackRatio(double ratio)
{
    if (lfo.isEnabled) stutterBuffer.setOrigRatio(ratio);
    stutterBuffer.setRatio(ratio);
}

void GlitchPluginAudioProcessor::enableLFO(bool shouldBeEnabled)
{
    stutterBuffer.setOrigRepeats(stutterBuffer.getStutterRepeats());
    stutterBuffer.setOrigDuration(stutterBuffer.getStutterDuration());
    stutterBuffer.setOrigRatio(stutterBuffer.getRatio());
    lfo.enableLFO(shouldBeEnabled);
}

int GlitchPluginAudioProcessor::convertMsToSamples(float ms)
{
    return (ms / 1000.f) * sr;
}

void GlitchPluginAudioProcessor::modulateStutterParameters()
{
    if (durationModDepth > 0.f) 
    {
        float minValue = stutterBuffer.getOrigDuration() * (1 - durationModDepth);
        int modulatedDurationInSamples = std::max(float(lfo.getCurrentValue() * durationModDepth * stutterBuffer.getOrigDuration() + minValue), float(convertMsToSamples(10.f)));
        stutterBuffer.setStutterDurationInSamples(modulatedDurationInSamples);
    }
    if (repeatModDepth > 0.f) {
        float minValue = stutterBuffer.getOrigRepeats() * (1 - repeatModDepth);
        stutterBuffer.setStutterRepeats(std::max(float(lfo.getCurrentValue() * repeatModDepth * stutterBuffer.getOrigRepeats()), 1.f));
    }
    if (ratioModDepth > 0.f) {
        float minValue = -2.f;
        float lfoValue = (lfo.getCurrentValue() * 2 - 1) * ratioModDepth;
        stutterBuffer.setRatio(std::max(float(stutterBuffer.getOrigRatio()) + lfoValue, minValue));
    }
}

void GlitchPluginAudioProcessor::updatePositionInfoForLFO(juce::AudioPlayHead* playhead)
{
    if (playhead) {
        playheadInfo = playhead->getPosition();
        if (playheadInfo.hasValue()) {
            double bpm = playhead->getPosition()->getBpm().orFallback(0.0);
            lfo.setBpm(bpm);
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GlitchPluginAudioProcessor();
}
