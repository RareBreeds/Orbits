#pragma once

#include "plugin.hpp"

// Lets a module add undo history entries for changes it makes on the engine thread.
template <typename Record, size_t Size = 32>
struct OrbitsUndoQueue
{
        // Engine thread only. Never blocks or allocates.
        bool post(const Record &record)
        {
                if(m_records.full())
                {
                        return false;
                }

                m_records.push(record);
                return true;
        }

        // UI thread only. Returns false when there are no records left.
        bool pop(Record &record)
        {
                if(m_records.empty())
                {
                        return false;
                }

                record = m_records.shift();
                return true;
        }

private:
        dsp::RingBuffer<Record, Size> m_records;
};

// Implemented by modules that post to an OrbitsUndoQueue.
struct OrbitsUndoSource
{
        // Called by OrbitsWidget::step() on the UI thread, once per frame.
        // Pops every pending record and passes an action for it to orbitsPushUndoAction().
        virtual void flushUndoHistory() = 0;
        virtual ~OrbitsUndoSource()
        {
        }
};

// UI thread only. Adds the action to Rack's undo history.
inline void orbitsPushUndoAction(history::Action *action)
{
        if(APP->history)
        {
                APP->history->push(action);
        }
        else
        {
                delete action;
        }
}
