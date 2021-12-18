/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/

struct CustomKnob: juce::Slider
{
    CustomKnob(): juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                               juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
        
    }
};

class ChannelEQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ChannelEQAudioProcessorEditor (ChannelEQAudioProcessor&);
    ~ChannelEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ChannelEQAudioProcessor& audioProcessor;
    
    CustomKnob BellFreqKnob, BellGainKnob, BellQKnob, LowCutFreqKnob, HighCutFreqKnob;
    
    CustomKnob LowCutSlopeKnob, HighCutSlopeKnob;
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachments = APVTS::SliderAttachment;
    
    Attachments BellFreqKnobAttachment, BellGainKnobAttachment, BellQKnobAttachment, LowCutFreqKnobAttachment, HighCutFreqKnobAttachment;
    Attachments LowCutSlopeKnobAttachment, HighCutSlopeKnobAttachment;
    
    MonoChain monoChain;
    
    std::vector<juce::Component*> GetComponents();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelEQAudioProcessorEditor)
};
