#include "CpuTimer.h"
#include "WinMin.h"

CpuTimer::CpuTimer()
	: m_SecondsPerCount(0.0),
	m_DeltaTime(-1.0),
	m_BaseTime(0),
	m_PausedTime(0),
	m_StopTime(0),
	m_PrevTime(0),
	m_CurrTime(0),
	m_bIsStopped(false)
{
	__int64 m_CountsPerSecond = {};
	QueryPerformanceFrequency( reinterpret_cast<LARGE_INTEGER*>(&m_CountsPerSecond) );
	m_SecondsPerCount = 1.0 / static_cast<double>(m_CountsPerSecond);
}

void CpuTimer::Tick()
{
	if (m_bIsStopped)
	{
		m_DeltaTime = 0.0;
		return;
	}

	__int64 currTime = {};
	QueryPerformanceCounter( reinterpret_cast<LARGE_INTEGER*>(&currTime) );
	m_CurrTime = currTime;

	m_DeltaTime = (m_CurrTime - m_PrevTime) * m_SecondsPerCount;
	m_PrevTime = m_CurrTime;

	// 这里 m_DeltaTime 初始值为 -1.0
	if (m_DeltaTime < 0.0)
	{
		m_DeltaTime = 0.0;
	}
}

void CpuTimer::Reset()
{
	__int64 currTime = {};
	QueryPerformanceCounter( reinterpret_cast<LARGE_INTEGER*>(&currTime) );

	m_BaseTime = currTime;
	m_PrevTime = currTime;
	m_StopTime = 0;
	m_PausedTime = 0;
	m_bIsStopped = false;
}

void CpuTimer::Start()
{
	if (m_bIsStopped)
	{
		__int64 startTime = {};
		QueryPerformanceCounter( reinterpret_cast<LARGE_INTEGER*>(&startTime) );

		m_PausedTime += (startTime - m_StopTime);
		m_PrevTime = startTime;
		m_StopTime = 0;
		m_bIsStopped = false;
	}
}

void CpuTimer::Stop()
{
	if (!m_bIsStopped)
	{
		__int64 currTime = {};
		QueryPerformanceCounter( reinterpret_cast<LARGE_INTEGER*>(&currTime) );

		m_StopTime = currTime;
		m_bIsStopped = true;
	}
}

float CpuTimer::TotalTime() const
{
	if (m_bIsStopped)
	{
		return static_cast<float>( ((m_StopTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount );
	}
	else
	{
		return static_cast<float>( ((m_CurrTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount );
	}
}
