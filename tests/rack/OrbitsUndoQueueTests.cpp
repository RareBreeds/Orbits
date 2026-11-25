#include "doctest.h"

#include "OrbitsUndoQueue.hpp"

#include <atomic>
#include <thread>

struct TestRecord
{
        uint32_t sequence;
        float payload[8];
};

static TestRecord makeRecord(uint32_t sequence)
{
        TestRecord record;
        record.sequence = sequence;
        for(float &value : record.payload)
        {
                value = static_cast<float>(sequence);
        }
        return record;
}

static bool isIntact(const TestRecord &record)
{
        for(float value : record.payload)
        {
                if(value != static_cast<float>(record.sequence))
                {
                        return false;
                }
        }
        return true;
}

TEST_CASE("The undo queue returns records in the order they were posted")
{
        OrbitsUndoQueue<TestRecord> queue;
        for(uint32_t i = 0; i < 5; ++i)
        {
                CHECK(queue.post(makeRecord(i)));
        }

        TestRecord record;
        for(uint32_t i = 0; i < 5; ++i)
        {
                REQUIRE(queue.pop(record));
                CHECK(record.sequence == i);
                CHECK(isIntact(record));
        }
        CHECK_FALSE(queue.pop(record));
}

TEST_CASE("A full undo queue drops new records and keeps the ones it has")
{
        OrbitsUndoQueue<TestRecord, 32> queue;
        for(uint32_t i = 0; i < 32; ++i)
        {
                CHECK(queue.post(makeRecord(i)));
        }
        CHECK_FALSE(queue.post(makeRecord(32)));

        TestRecord record;
        for(uint32_t i = 0; i < 32; ++i)
        {
                REQUIRE(queue.pop(record));
                CHECK(record.sequence == i);
        }
        CHECK_FALSE(queue.pop(record));

        // Once there's space again records are accepted.
        CHECK(queue.post(makeRecord(33)));
}

TEST_CASE("Records cross from one thread to another intact and in order")
{
        // As in the plugin: the engine thread posts, the UI thread pops. Records may be dropped
        // when the queue is full, but the ones that arrive must be complete and in order.
        static OrbitsUndoQueue<TestRecord> queue;
        const uint32_t count = 200000;
        std::atomic<bool> done{false};

        std::thread producer(
                [&]()
                {
                        for(uint32_t i = 1; i <= count; ++i)
                        {
                                queue.post(makeRecord(i));
                        }
                        done = true;
                });

        uint32_t last = 0;
        uint32_t received = 0;
        uint32_t problems = 0;
        TestRecord record;
        for(;;)
        {
                const bool finished = done;
                while(queue.pop(record))
                {
                        if(record.sequence <= last || !isIntact(record))
                        {
                                ++problems;
                        }
                        last = record.sequence;
                        ++received;
                }
                if(finished)
                {
                        break;
                }
        }
        producer.join();

        CHECK(received > 0);
        CHECK(problems == 0);
}
