#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

struct GraphEvent
{
    static GraphEvent createTransient (juce::int64 t)
    {
        return GraphEvent { t, t };
    }

    static GraphEvent createTransientNow()
    {
        return createTransient (juce::Time::getHighResolutionTicks());
    }

    inline bool isNull() const
    {
        return begin == 0 && end == 0;
    }

    juce::int64 begin{};
    juce::int64 end{};
};

class Graph : public juce::Component
{
public:
    explicit Graph (const juce::String& name)
        : juce::Component (name)
    {}

    void registerEvent (GraphEvent event = GraphEvent::createTransientNow());
    void setSyncTime (juce::int64 newSyncTime);

    void paint (juce::Graphics& g);

private:
    juce::int64 visibleWindowTicks { juce::Time::getHighResolutionTicksPerSecond() / 10 };
    juce::int64 analysisWindowTicks { juce::Time::getHighResolutionTicksPerSecond() * 2 };
    juce::int64 coalesceThresholdTicks { juce::Time::getHighResolutionTicksPerSecond() / 10'000 };
    juce::int64 syncTime;

    std::array<GraphEvent, 10000> eventsBuffer{};
    juce::AbstractFifo fifo1 { 10000 };

    std::array<GraphEvent, 10000> events{};
    juce::AbstractFifo fifo2 { 10000 };

    juce::VBlankAttachment vblank { this, [&] { repaint(); }};
    std::atomic<juce::Thread::ThreadID> threadId;
};
