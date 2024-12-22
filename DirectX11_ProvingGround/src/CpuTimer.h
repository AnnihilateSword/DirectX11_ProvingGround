#ifndef CPU_TIMER_H
#define CPU_TIMER_H

class CpuTimer
{
public:
	CpuTimer();
	~CpuTimer() = default;

public:
	void Tick();

	void Reset();
	void Start();
	void Stop();

public:
	float TotalTime() const;
	float DeltaTime() const { return static_cast<float>(m_DeltaTime); }
	bool IsStopped() const { return m_bIsStopped; }

private:
	double m_SecondsPerCount;
	double m_DeltaTime;

	__int64 m_BaseTime;
	__int64 m_PausedTime;
	__int64 m_StopTime;
	__int64 m_PrevTime;
	__int64 m_CurrTime;

	bool m_bIsStopped;
};

#endif  // CPU_TIMER_H