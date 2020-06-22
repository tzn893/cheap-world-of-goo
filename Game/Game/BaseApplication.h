#pragma once
#include "IApplication.h"

namespace Game {
	class BaseApplication : public IApplication {
	public:
		virtual bool initialize() override;
		virtual void tick() override;
		virtual void finalize() override;

		virtual bool isQuit() override;
		void Quit() { quit = true; }
	protected:
		bool quit = false;
	};
}