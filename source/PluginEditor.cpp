#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p) : juce::AudioProcessorEditor (&p)
{
    addAndMakeVisible (vblankGraph);
    addAndMakeVisible (paintGraph);
    addAndMakeVisible (timerGraph);
    addAndMakeVisible (asyncGraph);
    addAndMakeVisible (hiResTimerGraph);

    setSize (1000, 650);
    setResizable (true, true);

    Timer::startTimer (1);
    HighResolutionTimer::startTimer (1);
}

PluginEditor::~PluginEditor()
{
    Timer::stopTimer();
    HighResolutionTimer::stopTimer();
}

//==============================================================================
void PluginEditor::paint (juce::Graphics& g)
{
    paintEvent.begin = juce::Time::getHighResolutionTicks();
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    const auto bounds = getLocalBounds().withHeight (vblankGraph.getY()).reduced (5);

    std::vector<juce::String> info;

    using System = juce::SystemStats;
    info.push_back (System::getJUCEVersion());
   #if JUCE_DEBUG
    info.push_back ("Config: Debug");
   #else
    info.push_back ("Config: Release");
   #endif

  #if JUCE_ARM
   #if JUCE_32BIT
    info.push_back ("Architecture: ARM (32-bit)");
   #elif JUCE_64BIT
    info.push_back ("Architecture: ARM (64-bit)");
   #else
    info.push_back ("Architecture: ARM (Unknown)");
   #endif
  #elif JUCE_INTEL
   #if JUCE_32BIT
    info.push_back ("Architecture: x86 (32-bit)");
   #elif JUCE_64BIT
    info.push_back ("Architecture: x86 (64-bit)");
   #else
    info.push_back ("Architecture: x86 (Unknown)");
   #endif
  #else
    info.push_back ("Architecture: Unknown");
  #endif

    info.push_back ("OS: " + System::getOperatingSystemName() + (System::isOperatingSystem64Bit() ? " (64-bit)" : " (32-bit)"));
    info.push_back ("CPU Vendor: " + System::getCpuVendor());
    info.push_back ("CPU Model: " + System::getCpuModel());
    info.push_back ("CPU speed: " + juce::String ((float) System::getCpuSpeedInMegahertz() / 1000.0f, 2) + "GHz");
    info.push_back ("CPU Logical cores: " + juce::String (System::getNumCpus()));
    info.push_back ("CPU Physical cores: " + juce::String (System::getNumPhysicalCpus()));
    info.push_back ("RAM: " + juce::String ((float) System::getMemorySizeInMegabytes() / 1000.0f, 2) + "GB");

    const auto& p = *getAudioProcessor();
    info.push_back ("Host: " + juce::String (juce::PluginHostType{}.getHostDescription()));
    info.push_back ("Wrapper type: " + juce::String (p.getWrapperTypeDescription (p.wrapperType)));
    info.push_back ("Sample-rate: " + juce::String (p.getSampleRate(), 0) + "Hz");
    info.push_back ("Buffer size: " + juce::String (p.getBlockSize()) + " samples");
    info.push_back ("Channels: In: " + juce::String (p.getTotalNumInputChannels())
                         + ", Out: " + juce::String (p.getTotalNumInputChannels()));

    const auto* display = juce::Desktop::getInstance().getDisplays().getDisplayForRect (getScreenBounds());
    info.push_back ("Display is primary: " + juce::String (display != nullptr && display->isMain ? "YES" : "NO"));
    info.push_back ("Display resolution: " + juce::String (display != nullptr ? juce::String (display->totalArea.getWidth())
                                                                      + " x " + juce::String (display->totalArea.getHeight()) : "Unknown"));
    info.push_back ("Display scale: " + (display != nullptr ? juce::String (display->scale * 100.0, 0) + "%" : "Unknown"));
    info.push_back ("Display DPI: " + (display != nullptr ? juce::String (display->dpi, 0) : "Unknown"));
    info.push_back ("Display refresh rate: " + (display != nullptr && display->verticalFrequencyHz.has_value() ? juce::String (*display->verticalFrequencyHz, 0) + "Hz" : "Unknown"));

    auto* peer = getPeer();
    info.push_back ("Window is focused: " + juce::String (peer != nullptr && peer->isFocused() ? "YES" : "NO"));
    info.push_back ("Renderer: " + juce::String (peer != nullptr ? peer->getAvailableRenderingEngines()[peer->getCurrentRenderingEngine()] : "Unknown"));
    info.push_back ("Platform scale: " + juce::String (peer != nullptr ? juce::String (peer->getPlatformScaleFactor() * 100.0, 0) + "%" : "Unknown"));
    info.push_back ("Graphics scale: " + juce::String (g.getInternalContext().getPhysicalPixelScaleFactor() * 100.0, 0) + "%");

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions { juce::Font::getDefaultMonospacedFontName(), 12.0f, 0 }
                   .withFallbackEnabled (false));

    const auto lineHeight = g.getCurrentFont().getHeight() + 2.0f;
    const auto numLinesPerColumn = (int) ((float) bounds.getHeight() / lineHeight);
    const auto characterWidth = juce::GlyphArrangement::getStringWidthInt (g.getCurrentFont(), "_");

    if (numLinesPerColumn <= 0)
        return;

    int indent = 0;
    int longestLengthInColumn = 0;

    for (size_t i = 0; i < info.size(); ++i)
    {
        const int line = ((int) i) % numLinesPerColumn;

        if (line == 0)
        {
            indent += longestLengthInColumn;
            longestLengthInColumn = 0;

            if (indent >= bounds.getWidth())
                return;
        }

        longestLengthInColumn = juce::jmax (longestLengthInColumn, info[i].length() * characterWidth);

        auto b = bounds;
        b.removeFromLeft (indent);
        b.removeFromTop ((int) ((float) line * lineHeight));
        g.drawFittedText (info[i], b, juce::Justification::topLeft, 1);
    }
}

void PluginEditor::resized()
{
    auto bounds = getLocalBounds();
    const auto h = bounds.getHeight() / 6;
    bounds.removeFromTop (h);
    vblankGraph.setBounds (bounds.removeFromTop (h));
    paintGraph.setBounds (bounds.removeFromTop (h));
    timerGraph.setBounds (bounds.removeFromTop (h));
    asyncGraph.setBounds (bounds.removeFromTop (h));
    hiResTimerGraph.setBounds (bounds.removeFromTop (h));
}
