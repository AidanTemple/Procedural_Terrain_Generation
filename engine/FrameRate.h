#ifndef FRAMERATE_H
#define FRAMERATE_H

#pragma comment(lib, "winmm.lib")

#include <windows.h>
#include <mmsystem.h>

class FrameRate
{
public:
	FrameRate();
	FrameRate(const FrameRate&);
	~FrameRate();

	void Initialize();
	void Frame();
	int GetFps();

private:
	int m_fps, m_count;
	unsigned long m_startTime;
};

#endif