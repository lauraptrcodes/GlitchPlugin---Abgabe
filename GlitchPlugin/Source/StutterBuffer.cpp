/*
  ==============================================================================

	StutterBuffer.cpp
	Created: 5 Feb 2025 10:20:25am
	Author:  laura

  ==============================================================================
*/

#include "StutterBuffer.h"

StutterBuffer::StutterBuffer()
{
	targetDuration = maxStutterIndex;
	targetRatio = ratio;
	targetRepeats = stutterRepeats;
}

void StutterBuffer::prepareToPlay(int channels, int maximumStutterSize, int maxIndex)
{
	int ringBufferSize = maximumStutterSize * 4;
	maxStutterIndex = maxIndex;

	ringBuffer.setSize(channels, ringBufferSize);
	stutterBuffer.setSize(channels, maximumStutterSize);

	ringBuffer.clear();
	stutterBuffer.clear();
}

void StutterBuffer::process(juce::AudioBuffer<float>& buffer)
{
	pushBuffer(buffer);

	if (stutterState.get())
	{
		if (copyStutterToggle.get() || stutterBuffer.hasBeenCleared()) 
		{
			copyStutter();
		}
		else 
		{
			int numSamples = buffer.getNumSamples();
			int maxSample = juce::jmin(numSamples, int(maxStutterIndex - stutterReadIndex));
			bool paramsUpdated = anyParameterUpdated(numSamples);

			for (int chan = buffer.getNumChannels(); --chan >= 0;)
			{
				int readIndex = stutterReadIndex;
				int repeats = currentRepeat;

				for (int samp = 0; samp < maxSample; samp++)
				{
					if (paramsUpdated) rampParameters();
					float s = getInterpolatedSample(chan, readIndex);
					if (readIndex >= maxStutterIndex - numFadeSamples) {
						s += getCrossfadeSample(chan, readIndex, s);
					}
					buffer.setSample(chan, samp, s);
					readIndex = readIndex + 1;
				}
			}

			if (maxSample < numSamples) 
			{
				if (currentRepeat >= int(stutterRepeats - 1)) 
				{
					copyStutter();
					currentRepeat = 0;
				}
				for (int chan = buffer.getNumChannels(); --chan >= 0;) 
				{
					for (int samp = 0; samp < numSamples - maxSample; samp++)
					{
						if (paramsUpdated) rampParameters();
						buffer.setSample(chan, samp, getInterpolatedSample(chan, samp));
					}
				}
			}
			stutterReadIndex += numSamples;
			if (stutterReadIndex >= maxStutterIndex)
			{
				currentRepeat = (currentRepeat + 1) % int(stutterRepeats);
				stutterReadIndex = numFadeSamples;
			}
			stutterReadIndex %= int(maxStutterIndex);
		}
	}
}

void StutterBuffer::pushBuffer(juce::AudioBuffer<float>& buf)
{
	{
		for (int chan = ringBuffer.getNumChannels(); --chan >= 0;)
		{
			int wIndex = ringWriteIndex;
			for (int i = 0; i < buf.getNumSamples(); i++)
			{
				ringBuffer.setSample(chan, wIndex, buf.getSample(chan, i));
				wIndex = (wIndex + 1) % ringBuffer.getNumSamples();
			}
		}

		ringWriteIndex += buf.getNumSamples();
		ringWriteIndex %= ringBuffer.getNumSamples();
	}
}

void StutterBuffer::copyStutter()
{
	for (int chan = ringBuffer.getNumChannels(); --chan >= 0;)
	{
		int readSampleIndex = ringWriteIndex; 
		for (int i = stutterBuffer.getNumSamples(); --i >= 0;)
		{
			int destSampleIndex = i;
			readSampleIndex = (readSampleIndex - 1 + ringBuffer.getNumSamples()) % ringBuffer.getNumSamples();
			stutterBuffer.setSample(chan, destSampleIndex, ringBuffer.getSample(chan, readSampleIndex));
		}
	}

	applyFade();
	stutterReadIndex = 0;
	copyStutterToggle.set(false);
}

float StutterBuffer::getInterpolatedSample(int channel, int currentIndex)
{
	double absRatio = std::abs(ratio);
	float inputIdx = currentIndex / absRatio;

	int x1 = static_cast<int>(std::floor(inputIdx));
	int x2 = x1 + 1;
	if (x1 >= maxStutterIndex - 1) x1 = maxStutterIndex - 1;
	if (x2 >= maxStutterIndex) x2 = maxStutterIndex - 1;
	float fraction = inputIdx - x1;

	if (ratio < 0.0)
	{
		x1 = maxStutterIndex - 1 - x1;
		x2 = maxStutterIndex - 1 - x2;
		if (x1 <= 0) x1 = 0;
		if (x2 <= 0) x2 = 0;
	}
	float y1 = stutterBuffer.getSample(channel, x1);
	float y2 = stutterBuffer.getSample(channel, x2);

	return y1 + fraction * (y2 - y1);
}

bool StutterBuffer::anyParameterUpdated(int numSamples)
{
	bool updated = false;

	if (!juce::approximatelyEqual(int(targetDuration), int(maxStutterIndex)))
	{
		durRampIncrement = (targetDuration - maxStutterIndex) / numSamples;
		updated = true;
	}
	if (!juce::approximatelyEqual(int(targetRepeats), int(stutterRepeats))) {
		repeatsRampIncrement = (targetRepeats - stutterRepeats) / numSamples;
		updated = true;
	}
	if (!juce::approximatelyEqual(targetRatio, ratio)) {
		ratioRampIncrement = (targetRatio - ratio) / numSamples;
		updated = true;
	}
	return updated;
}

void StutterBuffer::rampParameters()
{
	if (!juce::approximatelyEqual(int(targetDuration), int(maxStutterIndex))) {
		maxStutterIndex += durRampIncrement;
	}
	if (!juce::approximatelyEqual(int(targetRepeats), int(stutterRepeats))) {
		stutterRepeats += repeatsRampIncrement;
	}
	if (!juce::approximatelyEqual(std::round(targetRatio * 10) / 10, std::round(ratio * 10) / 10)) {
		ratio += ratioRampIncrement;
	}
}

void StutterBuffer::applyFade()
{
	for (int chan = stutterBuffer.getNumChannels(); --chan >= 0;) {

		auto* data = stutterBuffer.getWritePointer(chan);
		//fade in
		for (int i = 0; i < numFadeSamples; ++i) {
			data[i] *= static_cast<float>(i) / numFadeSamples;
		}

		//fade out
		for (int i = 0; i < numFadeSamples; ++i)
		{
			const int idx = maxStutterIndex - numFadeSamples + i;
			data[idx] *= 1.0f - static_cast<float>(i) / numFadeSamples;
		}
	}
}

float StutterBuffer::getCrossfadeSample(int chan, int index, float sample)
{
	int crossFadeIdx = sample - (maxStutterIndex - numFadeSamples);
	return getInterpolatedSample(chan, crossFadeIdx);
}

void StutterBuffer::setStutterDurationInSamples(int numSamples)
{
	if (!juce::approximatelyEqual(int(maxStutterIndex), numSamples)) {
		targetDuration = numSamples;
		//fade out
		/*
		for (int chan = stutterBuffer.getNumChannels(); --chan >= 0;) {
			auto* data = stutterBuffer.getWritePointer(chan);
			for (int i = 0; i < numFadeSamples; ++i)
			{
				const int idx = targetDuration - numFadeSamples + i;
				data[idx] *= 1.0f - static_cast<float>(i) / numFadeSamples;
			}
		}*/
		
		return;
	}
	targetDuration = numSamples;
	maxStutterIndex = numSamples;
}

void StutterBuffer::setStutterRepeats(float repeats)
{
	if (!juce::approximatelyEqual(stutterRepeats, repeats)) {
		targetRepeats = repeats;
		return;
	}
	targetRepeats = repeats;
	stutterRepeats = repeats;
}

void StutterBuffer::setRatio(float playbackRatio)
{
	if (!juce::approximatelyEqual(ratio, playbackRatio)) {
		targetRatio = playbackRatio;
		return;
	}
	targetRatio = playbackRatio;
	ratio = playbackRatio;
}