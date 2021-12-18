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

using Filter = juce::dsp::IIR::Filter<float>;

using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter, Filter>;

using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

enum ChainPositions
{
    LowCut, Bell, HighCut
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
    MonoChain LeftChain, RightChain;
    
    using Coefficients = Filter::CoefficientsPtr;
    
    static void UpdateCoefficients(Coefficients &old, const Coefficients &replacements);
    
    template <int Index, typename ChainType, typename CoefficientType>
    void update(ChainType &chain, CoefficientType &newCoefficients)
    {
        UpdateCoefficients(chain.template get<Index>().coefficients, newCoefficients[Index]);
        chain.template setBypassed <Index>(false);
    }
    
    template <typename ChainType, typename CoefficientType>
    void UpdateCutFilter(ChainType &Cut, CoefficientType &CutCoefficients, const Slope &CutSlope)
    {
        Cut.template setBypassed <0>(true);
        Cut.template setBypassed <1>(true);
        Cut.template setBypassed <2>(true);
        Cut.template setBypassed <3>(true);
        Cut.template setBypassed <4>(true);
        
        switch (CutSlope)
        {
            case Slope_60:
            {
                update<4>(Cut, CutCoefficients);
            }
            case Slope_48:
            {
                update<3>(Cut, CutCoefficients);
            }
            case Slope_36:
            {
                update<2>(Cut, CutCoefficients);
            }
            case Slope_24:
            {
                update<1>(Cut, CutCoefficients);
            }
            case Slope_12:
            {
                update<0>(Cut, CutCoefficients);
            }
        }
        return;
    }
    
    void UpdateLowCutFilter(const ChainSettings &chainSettings);
    void UpdateHighCutFilter(const ChainSettings &chainSettings);
    void UpdateBellFilter(const ChainSettings &chainSettings);
    
    void UpdateFilters();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelEQAudioProcessor)
};
