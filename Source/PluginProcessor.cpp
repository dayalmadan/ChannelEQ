/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ChannelEQAudioProcessor::ChannelEQAudioProcessor()
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

ChannelEQAudioProcessor::~ChannelEQAudioProcessor()
{
}

//==============================================================================
const juce::String ChannelEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ChannelEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ChannelEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ChannelEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ChannelEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ChannelEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ChannelEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ChannelEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ChannelEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void ChannelEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ChannelEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::dsp::ProcessSpec Spec;
    
    Spec.maximumBlockSize = samplesPerBlock;
    Spec.numChannels = 1; // for mono
    Spec.sampleRate = sampleRate;
    
    LeftChain.prepare(Spec);
    RightChain.prepare(Spec);
    
    UpdateFilters();
    
}

void ChannelEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ChannelEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ChannelEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    UpdateFilters();

    juce::dsp::AudioBlock<float> Block(buffer);
    auto LeftBlock = Block.getSingleChannelBlock(0);
    auto RightBlock = Block.getSingleChannelBlock(1);
    
    juce::dsp::ProcessContextReplacing<float> LeftContext(LeftBlock);
    juce::dsp::ProcessContextReplacing<float> RightContext(RightBlock);
    
    LeftChain.process(LeftContext);
    RightChain.process(RightContext);
}

//==============================================================================
bool ChannelEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ChannelEQAudioProcessor::createEditor()
{
    return new ChannelEQAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void ChannelEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void ChannelEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState(tree);
        UpdateFilters();
    }
}

ChainSettings GetChainSettings(juce::AudioProcessorValueTreeState &apvts)
{
    ChainSettings Settings;
    
    Settings.LowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
    Settings.LowCutSlope = static_cast<Slope> (apvts.getRawParameterValue("LowCut Slope")->load());
    Settings.HighCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
    Settings.HighCutSlope = static_cast<Slope> (apvts.getRawParameterValue("HighCut Slope")->load());
    Settings.BellFreq = apvts.getRawParameterValue("Bell Freq")->load();
    Settings.BellGainInDB = apvts.getRawParameterValue("Bell Gain")->load();
    Settings.BellQ = apvts.getRawParameterValue("Bell Width (Q)")->load();
    
    return Settings;
}

void ChannelEQAudioProcessor::UpdateFilters()
{
    auto chainSettings = GetChainSettings(apvts);
    UpdateLowCutFilter(chainSettings);
    UpdateBellFilter(chainSettings);
    UpdateHighCutFilter(chainSettings);
    return;
}

void ChannelEQAudioProcessor::UpdateBellFilter(const ChainSettings &chainSettings)
{
    auto BellFilterCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.BellFreq, chainSettings.BellQ, juce::Decibels::decibelsToGain(chainSettings.BellGainInDB));
    
    UpdateCoefficients(LeftChain.get<ChainPositions::Bell>().coefficients, BellFilterCoefficients);
    UpdateCoefficients(RightChain.get<ChainPositions::Bell>().coefficients, BellFilterCoefficients);
    return;
}

void ChannelEQAudioProcessor::UpdateLowCutFilter(const ChainSettings &chainSettings)
{
    auto LowCutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.LowCutFreq, getSampleRate(), 2 * (chainSettings.LowCutSlope + 1));
    
    auto &LeftLowCut = LeftChain.get<ChainPositions::LowCut>();
    UpdateCutFilter(LeftLowCut, LowCutCoefficients, chainSettings.LowCutSlope);
    
    auto &RightLowCut = RightChain.get<ChainPositions::LowCut>();
    UpdateCutFilter(RightLowCut, LowCutCoefficients, chainSettings.LowCutSlope);
    return;
}

void ChannelEQAudioProcessor::UpdateHighCutFilter(const ChainSettings &chainSettings)
{
    auto HighCutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.HighCutFreq, getSampleRate(), 2 * (chainSettings.HighCutSlope + 1));
    
    auto &LeftHighCut = LeftChain.get<ChainPositions::HighCut>();
    UpdateCutFilter(LeftHighCut, HighCutCoefficients, chainSettings.HighCutSlope);
    
    auto &RightHighCut = RightChain.get<ChainPositions::HighCut>();
    UpdateCutFilter(RightHighCut, HighCutCoefficients, chainSettings.HighCutSlope);
    return;
}

void ChannelEQAudioProcessor::UpdateCoefficients(Coefficients &old, const Coefficients &replacements)
{
    *old = *replacements;
    return;
}

juce::AudioProcessorValueTreeState::ParameterLayout ChannelEQAudioProcessor::CreateParameterLayout() // for layout of our parameters
{
    juce::AudioProcessorValueTreeState::ParameterLayout Layout;
    Layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq",
                                                           "LowCut Freq",
                                                           juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.5f),
                                                           20.0f));
    Layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq",
                                                           "HighCut Freq",
                                                           juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.5f),
                                                           20000.0f));
    Layout.add(std::make_unique<juce::AudioParameterFloat>("Bell Freq",
                                                           "Bell Freq",
                                                           juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.5f),
                                                           750.0f));
    Layout.add(std::make_unique<juce::AudioParameterFloat>("Bell Gain",
                                                           "Bell Gain",
                                                           juce::NormalisableRange<float>(-24.0f, 24.0f, 0.5f, 1.0f),
                                                           0.0f));
    Layout.add(std::make_unique<juce::AudioParameterFloat>("Bell Width (Q)",
                                                           "Bell Width (Q)",
                                                           juce::NormalisableRange<float>(0.1f, 20.0f, 0.05f, 1.0f),
                                                           0.7f));
    juce::StringArray SlopeArray;
    for (int i = 0; i < 5; ++i)
    {
        juce::String slope;
        slope << (i + 1) * 12;
        slope << " dB/oct";
        SlopeArray.add(slope);
    }
    
    Layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", SlopeArray, 0));
    Layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope", SlopeArray, 0));
    
    return Layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChannelEQAudioProcessor();
}
