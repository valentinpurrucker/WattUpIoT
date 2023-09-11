#include "Scheduler.h"

// PUBLIC:

Scheduler::Scheduler() {}

void Scheduler::schedule(int16_t id, TimerCallback cb, u_long time, bool isTimestamp, bool retain)
{
    for (int i = 0; i < SCHEDULED_TIMER_NUMBER; i++)
    {
        if (!mScheduledTasks[i].mIsUsed)
        {
            D_Printf("Inserting new task at %d with id: %d\n", i, id);
            mScheduledTasks[i] = Task(id, cb, time, isTimestamp, retain);
            return;
        }
    }
}

bool Scheduler::cancel(int16_t id)
{
    for (int i = 0; i < SCHEDULED_TIMER_NUMBER; i++)
    {
        if (mScheduledTasks[i].mIsUsed && mScheduledTasks[i].mId == id)
        {
            mScheduledTasks[i].mIsUsed = false;
            return true;
        }
    }
    return false;
}

void Scheduler::loop()
{
    runRealtimeSchedule();

    u_long m = (long)millis();

    long diff;

    if (m < mPreviousTime)
    {
        diff = MAX_MILLIS - mPreviousTime + m;
    }
    else
    {
        diff = m - mPreviousTime;
    }

    if (diff < 1000)
    {
        return;
    }

    mDiff = diff;

    mPreviousTime = m;

    mCurrentTimestamp += mDiff / 1000;

    D_Println(mCurrentTimestamp);

    checkTimers();

    yield();
}

void Scheduler::scheduleRealtime(int16_t id, RealtimeScheduleCallback cb)
{
    for (int i = 0; i < SCHEDULED_REALTIME_NUMBER; i++)
    {
        if (!mRealtimeScheduledTasks[i].mIsUsed)
        {
            mRealtimeScheduledTasks[i] = Task(id, cb, 0, false, false);
            return;
        }
    }
}

// PRIVATE:
void Scheduler::checkTimers()
{
    for (int i = 0; i < SCHEDULED_TIMER_NUMBER; i++)
    {
        if (mScheduledTasks[i].mIsUsed)
        {

            mScheduledTasks[i].mTimeout -= (long)mScheduledTasks[i].mIsTimestamp ? 0l : mDiff;
            // D_Printf("Task: id:%ld, time:%ld, timeout:%ld\n", mScheduledTasks[i].mId, mScheduledTasks[i].mTime, mScheduledTasks[i].mTimeout);

            long timeout = (long)mScheduledTasks[i].mIsTimestamp ? mCurrentTimestamp : 0l;

            if (mScheduledTasks[i].mTimeout <= timeout)
            {
                mScheduledTasks[i].mCallback();
                if (mScheduledTasks[i].mIsRetained)
                {
                    mScheduledTasks[i].mTimeout = (long)mScheduledTasks[i].mTime;
                }
                else
                {
                    mScheduledTasks[i].mIsUsed = false;
                }
            }
        }
    }
}

void Scheduler::runRealtimeSchedule()
{
    for (int i = 0; i < SCHEDULED_REALTIME_NUMBER; i++)
    {
        if (mRealtimeScheduledTasks[i].mIsUsed)
        {
            mRealtimeScheduledTasks[i].mCallback();
        }
    }
}
