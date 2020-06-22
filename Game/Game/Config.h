#pragma once

namespace Game {
	struct Config {
		int width, height;
		Config(float width, float height) :
			width(width), height(height) {}
	};
}