
#include "GameTimer.h"

//GameTimer constructor
GameTimer::GameTimer()
: mSecondsPerCount(0.0), mBaseTime(0),
mPausedTime(0), mPrevTime(0), mCurrTime(0)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	mSecondsPerCount = 1.0 / (double)countsPerSec;
}


void GameTimer::Tick()
{
	
	// Get the time this frame.
	//WindowsFunction
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	mCurrTime = currTime;
	// Time difference between this frame and the previous.

	// Prepare for next frame.
	mPrevTime = mCurrTime;
}

void GameTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	mBaseTime = currTime;
	mPrevTime = currTime;
	mStopTime = 0;
	
}


float GameTimer::TotalTime()const
{	
		return (float)(((mCurrTime - mPausedTime) -
			mBaseTime)*mSecondsPerCount);
}