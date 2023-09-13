#pragma once

#include <Arduino.h>

const unsigned long MAX_MILLIS = std::numeric_limits<unsigned long>().max();

using TimerCallback = std::function<void()>;
using RealtimeScheduleCallback = std::function<bool()>;

template <typename T>
struct Task
{
    int16_t mId;

    u_long mTime;
    long mTimeout;

    T mCallback;

    bool mIsTimestamp;
    bool mIsRetained;
    bool mIsUsed = false;
    Task(int16_t id, T cb, u_long time, bool isTimestamp, bool retained) : mId(id), mCallback(cb), mTime(time), mTimeout((long)time), mIsTimestamp(isTimestamp), mIsRetained(retained), mIsUsed(true)
    {
        mIsUsed = true;
    }

    Task()
    {
        mIsUsed = false;
    }
};

class Scheduler
{

public:
    static const int8_t SCHEDULED_TIMER_NUMBER = 8;
    static const int8_t SCHEDULED_REALTIME_NUMBER = 2;

    Scheduler();

    void schedule(int16_t id, TimerCallback cb, u_long time, bool isTimestamp, bool retain);

    bool cancel(int16_t id);

    void scheduleAt(int16_t id, TimerCallback cb, u_long time, bool retain);

    void scheduleEvery(int16_t id, TimerCallback cb, u_long time);

    void scheduleInOnce(int16_t id, TimerCallback cb, u_long time);

    void scheduleRealtime(int16_t id, RealtimeScheduleCallback cb);

    void setTimestamp(u_long timestamp);

    int getTimestamp();

    void loop();

private:
    void checkTimers();

    void runRealtimeSchedule();

    Task<TimerCallback> mScheduledTasks[SCHEDULED_TIMER_NUMBER];
    Task<RealtimeScheduleCallback> mRealtimeScheduledTasks[SCHEDULED_REALTIME_NUMBER];

    u_long mCurrentTimestamp = 0;
    u_long mPreviousTime = 0;
    u_long mDiff = 0;
};