#include "IApplication.h"
#include "Common.h"

namespace Game {
	extern IApplication* app;
}

int main() {
	if (!Game::app->initialize()) {
		Log("fail to initialize application\n");
		return -1;
	}

	Log("initialize application successfully\n");
	while (!Game::app->isQuit()) {
		Game::app->tick();
	}

	Log("application quit\n");

	Game::app->finalize();
}