#include "BaseApplication.h"


using namespace Game;

bool BaseApplication::initialize() {
	return true;
}

void BaseApplication::finalize() {

}

bool BaseApplication::isQuit() {
	return quit;
}

void BaseApplication::tick() {

}