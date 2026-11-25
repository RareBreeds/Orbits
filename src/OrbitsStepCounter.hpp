#pragma once

// Tracks which step of a rhythm plays on each clock.
//
// Going forwards a clock plays the current step, then moves on to the next. Going backwards it
// moves back a step first, then plays it. So from the start of a rhythm, forwards plays step 0
// first and backwards plays the last step first. The length can change between clocks; a
// position beyond the new length starts again from step 0.
struct OrbitsStepCounter
{
        struct Step
        {
                // The step to play.
                unsigned int index;
                // Whether it's the first or last step of the cycle in the direction of play.
                bool is_first;
                bool is_last;
        };

        // Called on each clock with the rhythm's current length, which must be at least 1.
        Step advance(unsigned int length, bool reverse)
        {
                if(m_position >= length)
                {
                        m_position = 0;
                }

                if(reverse)
                {
                        m_position = m_position == 0 ? length - 1 : m_position - 1;
                }

                Step step;
                step.index = m_position;
                step.is_first = m_position == (reverse ? length - 1 : 0);
                step.is_last = m_position == (reverse ? 0 : length - 1);

                if(!reverse)
                {
                        m_position = m_position == length - 1 ? 0 : m_position + 1;
                }

                return step;
        }

        // The current position for the display, which shows a position beyond the length as 0.
        unsigned int position(unsigned int length) const
        {
                return m_position >= length ? 0 : m_position;
        }

        // Back to the start of the rhythm, e.g. on sync.
        void reset()
        {
                m_position = 0;
        }

      private:
        unsigned int m_position = 0;
};
