#ifndef __ENGINE_H
#define __ENGINE_H

#include "graphicsSubsystem.h"
#include "lightSubsystem.h"
#include "material.h"
#include <glm/glm.hpp>
#include <set>

class Engine
{
public:
	Engine();
	static void timerMediator(int value);
	static void drawCallMediator();
	static void keyboardCallMediator(unsigned char key, int x, int y);
	static void keyboardUpCallMediator(unsigned char key, int x, int y);
	static void mouseCallMediator(int button, int state, int x, int y);
	static void mouseMotionCallMediator(int x, int y);
	static void reshapeCallMediator(int w, int h);

	void workCycle();
	void drawHandler();
	void keyPressHandler(unsigned char key, int x, int y, bool pressed);
	void mouseHandler(int button, int state, int x, int y);
	void reshapeHandler(int w, int h);
private:
	GraphicsSubsystem gss;
	LightSubsystem lss;

	Sphere ball;
	Sphere lightSphere;
	Plane plane;
	Cube cube;
	const float collisionCoord;

	MaterialBlock ballMat;
	MaterialBlock clothMat;
	MaterialBlock woodMat;

	enum Key{ KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_MOUSE };
	std::set<Key> pressedKey;
	glm::vec2 mouseCoord;

	bool useMotionBlur;
	bool drawLightSources;

	// motion blur
	int framesPerFrame;
	int curFrame;

	void setPlane(int index);
};

#endif