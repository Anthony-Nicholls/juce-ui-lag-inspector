#include "Graph.h"

void Graph::registerEvent (GraphEvent event)
{
    if (fifo1.getFreeSpace() == 0)
        return;

    threadId = juce::Thread::getCurrentThreadId();
    const auto scope = fifo1.write (1);
    eventsBuffer[(size_t) scope.startIndex1] = event;
}

void Graph::setSyncTime (juce::int64 newSyncTime)
{
    syncTime = newSyncTime;
}

void Graph::paint (juce::Graphics& g)
{
    if (auto n = fifo1.getNumReady(); n > 0)
    {
        const auto scope1 = fifo1.read (n);

        jassert (fifo2.getFreeSpace() >= scope1.blockSize1 + scope1.blockSize2);

        if (scope1.blockSize1 > 0)
        {
            const auto scope2 = fifo2.write (scope1.blockSize1);

            if (scope2.blockSize1 > 0)
                std::copy (eventsBuffer.begin() + scope1.startIndex1,
                           eventsBuffer.begin() + scope1.startIndex1 + scope2.blockSize1,
                           events.begin() + scope2.startIndex1);

            if (scope2.blockSize2 > 0)
                std::copy (eventsBuffer.begin() + scope1.startIndex1 + scope2.blockSize1,
                           eventsBuffer.begin() + scope1.startIndex1 + scope2.blockSize1 + scope2.blockSize2,
                           events.begin() + scope2.startIndex2);
        }

        if (scope1.blockSize2 > 0)
        {
            const auto scope2 = fifo2.write (scope1.blockSize2);

            if (scope2.blockSize1 > 0)
                std::copy (eventsBuffer.begin() + scope1.startIndex2,
                           eventsBuffer.begin() + scope1.startIndex2 + scope2.blockSize1,
                           events.begin() + scope2.startIndex1);

            if (scope2.blockSize2 > 0)
                std::copy (eventsBuffer.begin() + scope1.startIndex2 + scope2.blockSize1,
                           eventsBuffer.begin() + scope1.startIndex2 + scope2.blockSize1 + scope2.blockSize2,
                           events.begin() + scope2.startIndex2);
        }
    }

    g.setColour (juce::Colours::black);
    g.fillRect (getLocalBounds());

    const auto h = (float) getHeight();
    const auto w = (float) getWidth();

    for (auto i = 0; i < 100; ++i)
    {
        g.setColour (juce::Colours::white.withAlpha (i % 10 == 0 ? 0.3f : 0.1f));
        g.drawVerticalLine ((int) (w * (float) i / 100.f), 0.0f, h);
    }

    g.setColour (juce::Colours::yellow);

    int startIndex1, blockSize1, startIndex2, blockSize2;
    fifo2.prepareToRead (fifo2.getNumReady(), startIndex1, blockSize1, startIndex2, blockSize2);

    int numRead{};
    const GraphEvent* previousEvent{};
    juce::StatisticsAccumulator<double> msIntervalStats;
    int coalesceCount{};

    const auto processEvent = [&](const auto& event)
    {
        jassert (&event != previousEvent);

        if (event.begin > syncTime)
            return;

        juce::ScopeGuard previousEventSetter { [&] { previousEvent = &event; } };

        const auto elapsedTicksEnd = syncTime - event.end;

        if (elapsedTicksEnd > analysisWindowTicks)
        {
            ++numRead;
            return;
        }

        if (previousEvent != nullptr)
        {
            const auto intervalTicks = event.begin - previousEvent->begin;
            const auto intervalSeconds = juce::Time::highResolutionTicksToSeconds (intervalTicks);
            msIntervalStats.addValue (intervalSeconds * 1000.0);
        }

        auto elapsedTicksStart = syncTime - event.begin;

        if (elapsedTicksStart < visibleWindowTicks || elapsedTicksEnd < visibleWindowTicks)
        {
            if (previousEvent != nullptr)
            {
                const auto diff = event.begin - previousEvent->begin;

                if (diff < coalesceThresholdTicks)
                {
                    elapsedTicksStart = syncTime - previousEvent->begin;
                    ++coalesceCount;
                }
                else
                {
                    coalesceCount = 1;
                }
            }
            else
            {
                coalesceCount = 1;
            }

            const auto eventStartPos = 1.0f - ((float) elapsedTicksStart / (float) visibleWindowTicks);
            const auto eventEndPos = 1.0f - ((float) elapsedTicksEnd / (float) visibleWindowTicks);

            const auto eventStartX = juce::roundToInt (eventStartPos * w);
            const auto eventWidth = juce::jmax (1, juce::roundToInt (eventEndPos * w) - eventStartX);

            const auto lineHeight = (int) (h * (float) coalesceCount / 4.0f);
            g.fillRect (eventStartX, (int) h - lineHeight, eventWidth, lineHeight);
        }
    };

    for (auto i = 0; i < blockSize1; ++i)
        processEvent (events[(size_t) (startIndex1 + i)]);

    for (auto i = 0; i < blockSize2; ++i)
        processEvent (events[(size_t) (startIndex2 + i)]);

    if (numRead > 0)
        fifo2.finishedRead (numRead);

    const auto gradientBox = getLocalBounds().removeFromLeft (200).toFloat();

    g.setGradientFill (juce::ColourGradient (juce::Colours::black, gradientBox.getTopLeft(),
                                             juce::Colours::transparentBlack, gradientBox.getTopRight(),
                                             false));

    g.fillRect (gradientBox);

    g.setColour (juce::Colours::white);
    g.drawRect (getLocalBounds());

    g.setFont (juce::FontOptions { juce::Font::getDefaultMonospacedFontName(), 12.0f, 0 }
               .withFallbackEnabled (false));

    auto textBounds = getLocalBounds().reduced (5).toFloat();
    const auto lineHeight = g.getCurrentFont().getHeight() + 2.0f;

    // By breaking up lines into parts we can significantly speed up drawing.
    // This is because the parts of the text that don't change will be cached.
    const auto drawLine = [&](juce::StringArray parts)
    {
        const auto lineBounds = textBounds.removeFromTop (lineHeight);
        juce::String indent;

        for (const auto& part : parts)
        {
            g.drawText (indent + part, lineBounds, juce::Justification::topLeft);
            indent += juce::String::repeatedString (" ", part.length());
        }
    };

    drawLine ({ getName(), " (0x", juce::String::toHexString ((uint64_t) threadId.load()), ")" });
    drawLine ({ "Freq: ", juce::String (msIntervalStats.getCount() / 2), "Hz" });
    drawLine ({ " Min: ", juce::String (msIntervalStats.getMinValue(), 1), "ms" });
    drawLine ({ " Max: ", juce::String (msIntervalStats.getMaxValue(), 1), "ms" });
    drawLine ({ " Avg: ", juce::String (msIntervalStats.getAverage(), 1), "ms",
                " (SD ", juce::String (msIntervalStats.getStandardDeviation(), 3), ")" });
}
