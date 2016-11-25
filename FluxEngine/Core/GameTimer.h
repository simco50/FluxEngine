#pragma once
class GameTimer
{
public:
	GameTimer();
	~GameTimer();

	static float GameTime();
	static float DeltaTime();

	static void Reset();
	static void Start();
	static void Stop();
	static void Tick();
	static vector<float> GetFrameTimes() { return m_FrameTimes; }

private:
	static double m_SecondsPerCount;
	static double m_DeltaTime;

	static __int64 m_BaseTime;
	static __int64 m_PausedTime;
	static __int64 m_StopTime;
	static __int64 m_PrevTime;
	static __int64 m_CurrTime;

	static bool m_IsStopped;

	static vector<float> m_FrameTimes;
};

