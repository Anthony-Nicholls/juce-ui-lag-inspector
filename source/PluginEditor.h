#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "Graph.h"

//==============================================================================
class PluginEditor  : public juce::AudioProcessorEditor,
                              private juce::Timer,
                              private juce::HighResolutionTimer,
                              private juce::AsyncUpdater
{
public:
    PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override
    {
        paintEvent.end = juce::Time::getHighResolutionTicks();
        paintGraph.registerEvent (paintEvent);
    }

    void resized() override;

private:
    void timerCallback() final { timerGraph.registerEvent(); }
    void hiResTimerCallback() final
    {
        hiResTimerGraph.registerEvent();
        triggerAsyncUpdate();
    }
    
    void handleAsyncUpdate() override
    {
        asyncGraph.registerEvent();
    }

    juce::VBlankAttachment vblank { this, [&]
    {
        const auto now = juce::Time::getHighResolutionTicks();
        vblankGraph.setSyncTime (now);
        timerGraph.setSyncTime (now);
        paintGraph.setSyncTime (now);
        hiResTimerGraph.setSyncTime (now);
        asyncGraph.setSyncTime (now);
        vblankGraph.registerEvent (GraphEvent::createTransient (now));
        repaint();
    } };

    Graph vblankGraph { "VBlank" };
    Graph timerGraph { "Timer" };
    Graph paintGraph { "paint" };
    Graph asyncGraph { "AsyncUpdater" };
    Graph hiResTimerGraph { "HighResolutionTimer" };
    GraphEvent paintEvent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
