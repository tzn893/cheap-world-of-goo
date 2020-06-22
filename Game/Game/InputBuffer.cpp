#include "InputBuffer.h"

enum KeyState {
	NONE = 0, DOWN = 1, HOLD = 2, UP = 3
};

void Game::InputBuffer::BufferWriteKeyDown(Game::InputBuffer::KeyCode code) {
	buffer[code] = DOWN;
}

void Game::InputBuffer::BufferWriteKeyUp(Game::InputBuffer::KeyCode code) {
	buffer[code] = UP;
}

void Game::InputBuffer::BufferWriteMousePosition(uint32_t x,uint32_t y) {
	mousePosx = x, mousePosy = y;
}

void Game::InputBuffer::tick() {
	for (int i = 0; i != KEY_CODE_SIZE;i++) {
		switch (buffer[i]) {
		case DOWN :
			buffer[i] = HOLD;
			break;
		case UP:
			buffer[i] = NONE;
			break;
		}
	}
}

bool Game::InputBuffer::KeyDown(Game::InputBuffer::KeyCode code) { return buffer[code] == DOWN; }
bool Game::InputBuffer::KeyUp(Game::InputBuffer::KeyCode code) { return buffer[code] == UP; }
bool Game::InputBuffer::KeyHold(Game::InputBuffer::KeyCode code) {
	return buffer[code] == HOLD; 
}

Game::Vector2 Game::InputBuffer::MousePosition() {
	return Vector2(mousePosx,mousePosy);
}