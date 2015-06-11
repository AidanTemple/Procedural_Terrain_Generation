#include "FrameRate.h"

FrameRate::FrameRate()
{

}

FrameRate::FrameRate(const FrameRate& other)
{

}

FrameRate::~FrameRate()
{

}

void FrameRate::Initialize()
{
	// Initialize the counters and the start time.
	m_fps = 0;
	m_count = 0;
	m_startTime = timeGetTime();
	
	return;
}

void FrameRate::Frame()
{
	m_count++;

	// If one second has passed then update the frame per second speed.
	if(timeGetTime() >= (m_startTime + 1000))
	{
		m_fps = m_count;
		m_count = 0;
		
		m_startTime = timeGetTime();
	}
}

int FrameRate::GetFps()
{
	return m_fps;
}