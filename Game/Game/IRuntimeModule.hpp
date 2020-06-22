#pragma once

namespace Game {
	class IRuntimeModule {
	public:
		virtual bool initialize() = 0;
		virtual void tick() = 0 ;
		virtual void finalize() = 0;

		virtual ~IRuntimeModule() {}
	};
}