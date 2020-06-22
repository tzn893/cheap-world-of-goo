#include "Timer.h"

#ifdef _WIN64
#include <Windows.h>

#define QUERY_CLOCK_TICK(out) QueryPerformanceCounter((LARGE_INTEGER*)&out)
#define QUERY_CLOCK_FREQUENCY(out) QueryPerformanceFrequency((LARGE_INTEGER*)&out)
#endif

bool Game::Timer::initialize() {
	QUERY_CLOCK_FREQUENCY(clockFrequency);
	Reset();

	return true;
}

void Game::Timer::finalize() {}

void Game::Timer::Reset() {
	pauseClockTick = 0;

	paused = false;

	QUERY_CLOCK_TICK(startClockTick);

	currClockTick = prevClockTick = startClockTick;
}

void Game::Timer::Pause() { 
	prevClockTick = currClockTick;

	paused = true; 
}

void Game::Timer::Start() {
	paused = false;
}

void Game::Timer::tick() {
	if (!paused) {
		prevClockTick = currClockTick;

		QUERY_CLOCK_TICK(currClockTick);
	}
	else {
		uint64_t temp;

		QUERY_CLOCK_TICK(temp);

		pauseClockTick += temp - prevClockTick;
		currClockTick = prevClockTick = temp;
	}
}

float Game::Timer::DeltaTime() {
	return (currClockTick - prevClockTick) / static_cast<float>(clockFrequency);
}

float Game::Timer::TotalTime() {
	return (currClockTick - pauseClockTick - startClockTick) / static_cast<float>(clockFrequency);
}