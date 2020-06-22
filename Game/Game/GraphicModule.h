#pragma once
#include "IRuntimeModule.hpp"
#include "Vector.h"


namespace Game {

	class GraphicModule : public IRuntimeModule {
	public:
		virtual bool initialize() = 0;
		virtual void tick() = 0;
		virtual void finalize() = 0;

		//The 2D rendering part

		//Draw a point at position pos on the screen.The depth determines the covering order on the screen
		//Smaller the depth value the geometry will be more likely to cover other geometries
		//For exmaple a point which has depth value 0.5 will cover the point which has depth value 0.7
		//at the same place
		virtual void point2D(Vector2 pos,float size,Vector4 Color,float depth = .5f) = 0;
		//Draw a line from position start to position end.The depth determines the covering order on the screen
		//Smaller the depth value the geometry will be more likely to cover other geometries
		//For exmaple a point which has depth value 0.5 will cover the point which has depth value 0.7
		//at the same place
		virtual void line2D(Vector2 start,Vector2 end,float width,Vector4 Color,float depth = .7f) = 0;

		//Set the view port of the 2D space.The center of the screen will be center value in world space
		//,the width of the view port rectangle will be caculated by the height and the screen's resolution
		virtual void set2DViewPort(Vector2 center,float height) = 0;
	};
}