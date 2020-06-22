#include "lesson1_spring_sys.h"
#include "Timer.h"
#include "GraphicModule.h"
#include "InputBuffer.h"
#include "IApplication.h"
#include <string>

namespace Game {
	extern Timer gTimer;
	
	SpringSys springSys;
	IRuntimeModule* clientModule = &springSys;

	extern InputBuffer gInput;

	extern GraphicModule* gGraphic;

	extern IApplication* app;
}

using namespace Game;

bool SpringSys::initialize() {
	viewCenter = Vector2(0.,0.);
	viewHeight = 4.;

	Game::gGraphic->set2DViewPort(viewCenter,viewHeight);

	return true;
}


void SpringSys::initScene() {

	maxHeight = 0., recordLine = 0.;

	massPoints.push_back(MassPoint(Vector2(0, -.9), 1.));
	massPoints.push_back(MassPoint(Vector2(0.5, -1.7), 1.));
	massPoints.push_back(MassPoint(Vector2(-0.5, -1.7), 1.));

	float dis = length(massPoints[0].position - massPoints[1].position);
	springs.push_back(Spring(0, 1, maxSpringLength, minSpringLength, dis));
	dis = length(massPoints[0].position - massPoints[2].position);
	springs.push_back(Spring(0, 2, maxSpringLength, minSpringLength, dis));
	dis = length(massPoints[1].position - massPoints[2].position);
	springs.push_back(Spring(1, 2, maxSpringLength, minSpringLength, dis));
}

const char* welcomeWords = "Welcome to the cheap world of goo!\n"
						   "W,S,A,D will move camera around the scene,while Q,E will zoom the camera\n"
						   "There are two modes,challege and infinity.In challege mode you only have 50\n"
						   "goos while in infinity mode there are on limitations.Try to build higher towers\n"
						   ",the height will be recorded by the blue line in the middle.Watch out for adding\n"
						   "too much force on one spring,it will turn red and beark down while taking too \n"
						   "much weight!Press R will reset the every thing(Which means that every thing you\n"
						   "are working on will be gone and you will return to this message box).\n\n"
						   "Press YES to Challege mode,Press No to infinity mode\n";


void SpringSys::tick() {

	static bool welcome = false;
	if (!welcome) {
		welcome = true;
		if (Game::app->YesNoBox("welcome", welcomeWords)) {
			limitation = 50;
		}
		else {
			limitation = -1;
		}
		massPoints.clear();
		springs.clear();
		initScene();

		viewCenter = Vector2(0., 0.);
		viewHeight = 4.;

		Game::gGraphic->set2DViewPort(viewCenter, viewHeight);
	}

	std::string title = "cheap world of goo current height " + std::to_string(recordLine);
	if (limitation >= 0) {
		title += " chanllege mode " + std::to_string(limitation) + " goos left ";
	}
	else {
		title += " infinity mode ";
	}
	app->setTitle(title.c_str());

	float deltaTime = gTimer.DeltaTime();
	const float dt = 4e-3;
	//iterate the system
	
	while (dt < deltaTime) {
		iter(dt);
		deltaTime -= dt;
	}
	iter(deltaTime);
	
	drawPoints();

	if (limitation == 0) {
		std::string endWord = std::string("Well done! you have reached at height ") + std::to_string(maxHeight);
		Game::app->messageBox("END", endWord.c_str());
		welcome = false;
	}

	Vector2 cursorPos = cursorPosition();
	if (gInput.KeyHold(InputBuffer::KeyCode::MOUSE_LEFT)) {
		gGraphic->point2D(cursorPos, pointSize , Game::Vector4(0, 0, 0, 1));

		for (const auto& p : massPoints) {
			float dis = length(p.position - cursorPos);
			if (dis < maxSpringLength) {
				gGraphic->line2D(cursorPos,p.position,lineWidth,Vector4(0,1,0,1));
			}
		}
	}
	if (gInput.KeyUp(InputBuffer::KeyCode::MOUSE_LEFT)) {
		
		massPoints.push_back(MassPoint(cursorPos,1.));
		int newPoint = massPoints.size() - 1;
		bool flag = true;
		//the vector container will extend itself and move the data to a bigger place 
		//when we push back something and all the pointers to the elements in it will be invaild
		for (int i = 0; i != massPoints.size() - 1;i++) {
			float dis = length(massPoints[i].position - cursorPos);
			if (dis < maxSpringLength) {
				flag = false;
				springs.push_back(Spring(i,newPoint,maxSpringLength,minSpringLength,dis));
			}
		}
		if (flag) {
			massPoints.pop_back();
		}else {
			limitation--;
		}
	}
	
	//moving camera around
	if (gInput.KeyHold(InputBuffer::KeyCode::W)) {
		viewCenter.y += 1e-2;
		gGraphic->set2DViewPort(viewCenter, viewHeight);
	}
	else if (gInput.KeyHold(InputBuffer::KeyCode::S)) {
		viewCenter.y -= 1e-2;
		viewCenter.y = viewCenter.y > 0 ? viewCenter.y : 0;
		gGraphic->set2DViewPort(viewCenter, viewHeight);
	}
	else if (gInput.KeyHold(InputBuffer::KeyCode::A)) {
		viewCenter.x -= 1e-2;
		gGraphic->set2DViewPort(viewCenter, viewHeight);
	}
	else if (gInput.KeyHold(InputBuffer::KeyCode::D)) {
		viewCenter.x += 1e-2;
		gGraphic->set2DViewPort(viewCenter, viewHeight);
	}
	//zoom the camera
	if (gInput.KeyHold(InputBuffer::KeyCode::E)) {
		viewHeight += 1e-2;
		viewHeight = viewHeight < 16. ? viewHeight : 16.;
		gGraphic->set2DViewPort(viewCenter, viewHeight);
	}
	else if (gInput.KeyHold(InputBuffer::KeyCode::Q)) {
		viewHeight -= 1e-2;
		viewHeight = viewHeight > 4. ? viewHeight : 4.;
		gGraphic->set2DViewPort(viewCenter,viewHeight);
	}


	if (gInput.KeyDown(InputBuffer::KeyCode::R)) {
		massPoints.clear();
		springs.clear();

		welcome = false;
	}


	//draw the record line to specify the heighest place the player have reached
	if (recordLine < maxHeight) {
		recordLine += deltaTime * 4.;
	}

	Config con = app->getSysConfig();
	float viewWidth = viewHeight * ((float)con.width / (float)con.height);
	Vector2 rStart = Vector2(viewCenter.x - viewWidth / 2.,recordLine)
		, rEnd = Vector2(viewCenter.x + viewWidth / 2., recordLine);
	gGraphic->line2D(rStart,rEnd,lineWidth * 0.8,Vector4(0,1,1,1),0.9);

	//draw the ground
	rStart.y = ground - 0.3;
	rEnd.y = ground - 0.3;
	gGraphic->line2D(rStart,rEnd,0.6,Vector4(0.2,0.3,0.5,1.));

}

void SpringSys::finalize() {
	massPoints.clear();
	springs.clear();
}

void SpringSys::iter(float dt) {
	//Don't integrate accelerate by time!
	for (auto& point : massPoints) {
		point.accel = Vector2();
	}

	auto iter = springs.begin();

	while(iter != springs.end()) {
		auto& spring = *iter;
		
		MassPoint& p1 = massPoints[spring.p0index], & p2 = massPoints[spring.p1index];

		float dis = length(p1.position - p2.position);
		Vector2 dir = normalize(p1.position - p2.position);

		Vector2 Force = dir * (spring.originLength - dis) *  stillness * damp;
		spring.currentForce = length(Force);
		if (spring.currentForce  < spring.maxForce) {
			p1.accel = p1.accel + Force / p1.mass;
			p2.accel = p2.accel - Force / p2.mass;
		}
		else {
			iter = springs.erase(iter);
			if (iter == springs.end()) {
				break;
			}
		}
		iter++;
	}

	for (auto& point : massPoints) {

		point.accel = point.accel + Vector2(0,-gravity);
		point.velocity = point.velocity + point.accel * dt;
		point.position = point.position + point.velocity * dt;

		point.accel = Vector2();
		//collision with the ground
		if (point.position.y < ground) {
			point.velocity.y = 0;
			point.position.y = ground;
		}

		//wether the point is higher than the record?
		maxHeight = point.position.y > maxHeight ? point.position.y : maxHeight;
	}
}

void SpringSys::drawPoints() {
	for (auto point : massPoints) {
		gGraphic->point2D(point.position,pointSize,Vector4(0,0,0,1));
	}
	for (auto spring : springs) {
		Vector2 start = massPoints[spring.p0index].position, end = massPoints[spring.p1index].position;

		Vector4 color = lerp(Vector4(1.,1.,1.,1.),Vector4(1.,0.,0.,1.),spring.currentForce / spring.maxForce);
		gGraphic->line2D(start,end,lineWidth,color);
	}
}

Vector2 SpringSys::cursorPosition() {
	Config con = app->getSysConfig();
	float height = con.height, width = con.width;

	Vector2 cursorPos;

	cursorPos.y = -(gInput.MousePosition().y / (height - 40.) * 2. - 1. ) *
		(viewHeight / 2.);
	cursorPos.x = (gInput.MousePosition().x / (width - 10.) * 2. - 1.) * 
		(width / height) * (viewHeight / 2.);

	return cursorPos + viewCenter;
}