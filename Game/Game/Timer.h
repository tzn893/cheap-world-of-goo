#pragma once
#include "IRuntimeModule.hpp"
#include <stdint.h>

namespace Game {
	class Timer : public IRuntimeModule{
	public:
		virtual bool initialize() override;
		virtual void tick() override;
		virtual void finalize() override;

		float TotalTime();
		float DeltaTime();

		void Reset();
		void Pause();
		void Start();

	private:

		uint64_t prevClockTick;
		uint64_t currClockTick;
		uint64_t pauseClockTick;
		uint64_t startClockTick;

		uint64_t clockFrequency;

		bool paused;
	};
}