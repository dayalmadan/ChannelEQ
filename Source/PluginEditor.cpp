/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ChannelEQAudioProcessorEditor::ChannelEQAudioProcessorEditor (ChannelEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
BellFreqKnobAttachment(audioProcessor.apvts, "Bell Freq", BellFreqKnob),
BellGainKnobAttachment(audioProcessor.apvts, "Bell Gain", BellGainKnob),
BellQKnobAttachment(audioProcessor.apvts, "Bell Width (Q)", BellQKnob),
LowCutFreqKnobAttachment(audioProcessor.apvts, "LowCut Freq", LowCutFreqKnob),
HighCutFreqKnobAttachment(audioProcessor.apvts, "HighCut Freq", HighCutFreqKnob),
LowCutSlopeKnobAttachment(audioProcessor.apvts, "LowCut Slope", LowCutSlopeKnob),
HighCutSlopeKnobAttachment(audioProcessor.apvts, "HighCut Slope", HighCutSlopeKnob)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    for (auto *component: GetComponents())
    {
        addAndMakeVisible(component);
    }
    
    setSize (600, 400);
}

ChannelEQAudioProcessorEditor::~ChannelEQAudioProcessorEditor()
{
}

//==============================================================================
void ChannelEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void ChannelEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);
    
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);
    
    LowCutFreqKnob.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
    LowCutSlopeKnob.setBounds(lowCutArea);
    
    HighCutFreqKnob.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));
    HighCutSlopeKnob.setBounds(highCutArea);
    
    BellFreqKnob.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
    BellGainKnob.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    BellQKnob.setBounds(bounds);
}

std::vector<juce::Component*> ChannelEQAudioProcessorEditor::GetComponents()
{
    return
    {
        &BellFreqKnob,
        &BellGainKnob,
        &BellQKnob,
        &LowCutFreqKnob,
        &HighCutFreqKnob,
        &LowCutSlopeKnob,
        &HighCutSlopeKnob
    };
}
