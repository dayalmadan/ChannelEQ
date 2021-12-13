/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum Slope
{
    Slope_12, Slope_24, Slope_36, Slope_48, Slope_60
};

struct ChainSettings
{
    float BellFreq {0}, BellGainInDB {0}, BellQ {0};
    float LowCutFreq {0}, HighCutFreq {0};
    Slope LowCutSlope {Slope::Slope_12}, HighCutSlope {Slope::Slope_12};
};

ChainSettings GetChainSettings(juce::AudioProcessorValueTreeState &apvts);

//==============================================================================
/**
*/
class ChannelEQAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    ChannelEQAudioProcessor();
    ~ChannelEQAudioProcessor() override;

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
    
    static juce::AudioProcessorValueTreeState::ParameterLayout CreateParameterLayout(); // for layout of our parameters
    juce::AudioProcessorValueTreeState apvts{*this, nullptr, "Parameters", CreateParameterLayout()};

private:
    
    using Filter = juce::dsp::IIR::Filter<float>;
    
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter, Filter>;
    
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;
    
    MonoChain LeftChain, RightChain;
    
    enum ChainPositions
    {
        LowCut, Bell, HighCut
    };
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelEQAudioProcessor)
};
