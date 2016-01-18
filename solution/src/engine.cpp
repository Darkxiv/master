#include "engine.h"
#include "settings.h"
#include "material.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

static Engine *engine;

Engine::Engine(): useMotionBlur(false), drawLightSources(false),
	curFrame(0), collisionCoord(9.0), framesPerFrame(3),
	ball(glm::vec3(0.0, 1.0, 0.0), SPHERE_SHAPE, SPHERE_SHAPE), 
	lightSphere(glm::vec3(0.0), LIGHT_SPHERE_SHAPE, LIGHT_SPHERE_SHAPE)
{
	engine = this;

	if(gss.initGraphicsSubsystem())
		return;

	printf("Loading meshes...\n");
	ball.load();
	lightSphere.load();
	plane.load();
	cube.load();

	ballMat.specularColor = glm::vec4(0.8, 0.8, 0.8, 1.0);
	ballMat.specularShininess = 0.07f;
	ballMat.reflectivity = 0.3f;

	clothMat.specularShininess = 0.0f;
	clothMat.reflectivity = 0.0f;

	woodMat.specularColor = glm::vec4(0.7, 0.7, 0.7, 1.0);
	woodMat.specularShininess = 0.15f;
	woodMat.reflectivity = 0.0f;

	glutDisplayFunc(Engine::drawCallMediator);
	glutKeyboardFunc(Engine::keyboardCallMediator);
	glutKeyboardUpFunc(Engine::keyboardUpCallMediator);
	glutMouseFunc(Engine::mouseCallMediator);
	glutMotionFunc(Engine::mouseMotionCallMediator);
	glutReshapeFunc(Engine::reshapeCallMediator);
	glutTimerFunc(TIMER_SPEED, Engine::timerMediator, 0);
	glutMainLoop();
}

void Engine::workCycle()
{
	/*=============================================
	  keys processing -> ball's movement -> drawing
	===============================================*/
	
	float div = useMotionBlur ? 1.0f / framesPerFrame : 1.0f;
	for (std::set<Key>::iterator key = pressedKey.begin(); key != pressedKey.end(); key++)
	{
		glm::vec3 camv = gss.getViewVector();
		switch (*key)
		{
		case Engine::KEY_UP:
			ball.changeVelocity(camv, div);
			break;
		case Engine::KEY_DOWN:
			ball.changeVelocity(-camv, div);
			break;
		case Engine::KEY_LEFT:
			ball.changeVelocity(glm::vec3(camv.z, 0.0f, -camv.x), div);
			break;
		case Engine::KEY_RIGHT:
			ball.changeVelocity(glm::vec3(-camv.z, 0.0f, camv.x), div);
			break;
		default:
			break;
		}
	}
	ball.move(div);
	glm::vec3 bwp = ball.getWorldPos();
	glm::vec3 bv = ball.getVelocity();

	if (abs(bwp.x) > collisionCoord)
		ball.setVelocity(glm::vec3(-bv.x, 0.0, bv.z));
	if (abs(bwp.z) > 9.0)
		ball.setVelocity(glm::vec3(bv.x, 0.0, -bv.z));
	bwp = glm::clamp(bwp, -collisionCoord, collisionCoord);
	
	gss.setCamTarget(bwp);
	ball.setWorldPos(bwp);

	glutPostRedisplay();
}

void Engine::drawHandler()
{
	gss.shadowMapPass(static_cast<Mesh*>(&ball), lss);
	gss.clearBuffers();
	gss.setCam();
	gss.bindLighting(lss);

	gss.bindMaterial(ballMat);
	gss.drawBall(ball);
	
	gss.bindMaterial(clothMat);
	setPlane(0);
	gss.drawPlane(plane, gss.getClothTexture());

	gss.bindMaterial(woodMat);
	for (int i = 1; i < 5; i++)
	{
		setPlane(i);
		gss.drawPlane(plane, gss.getWoodTexture());
	}

	if (drawLightSources)
		gss.drawLight(static_cast<Mesh*>(&lightSphere), lss);

	gss.drawSkybox(cube);

	if (useMotionBlur)
	{
		gss.accumFrame(curFrame, framesPerFrame);
		curFrame++;
		if (curFrame >= framesPerFrame)
		{
			curFrame = 0;
			gss.returnFrame();
			gss.swapBuffers();
		}
	}
	else
		gss.swapBuffers();
}

void Engine::keyPressHandler(unsigned char key, int x, int y, bool pressed)
{
	Key input;
	switch (key)
	{
	case 'W':
	case 'w': input = Engine::KEY_UP; break;
	case 'S':
	case 's': input = Engine::KEY_DOWN;	break;
	case 'A':
	case 'a': input = Engine::KEY_LEFT; break;
	case 'D':
	case 'd': input = Engine::KEY_RIGHT; break;
	case 'q':
	case 'Q':
	case 27: exit(0); break;
	}

	if (pressed)
	{
		pressedKey.insert(input);
		switch (key)
		{
		case 'Z':
		case 'z': ballMat.specularShininess -= 0.03f; break;
		case 'X':
		case 'x': ballMat.specularShininess += 0.03f; break;
		case 'C':
		case 'c': ballMat.reflectivity -= 0.03f; break;
		case 'V':
		case 'v': ballMat.reflectivity += 0.03f; break;
		case 'B':
		case 'b': 
			useMotionBlur = !useMotionBlur; curFrame = 0; 
			printf("Motion blur %s\n", useMotionBlur ? "ON" : "OFF");
			break;
		case 'L':
		case 'l': 
			drawLightSources = !drawLightSources; 
			printf("%s\n", drawLightSources ? "Light sources are shown" : "Light sources are hidden");
			break;
		}
		ballMat.specularShininess = glm::clamp(ballMat.specularShininess, 0.0f, 0.3f);
		ballMat.reflectivity = glm::clamp(ballMat.reflectivity, 0.0f, 1.0f);
	}
	else
	{
		std::set<Key>::iterator it;
		if ((it = pressedKey.find(input)) != pressedKey.end())
			pressedKey.erase(it);
	}
}

void Engine::mouseHandler(int button, int state, int x, int y)
{
	if ((button >= 3 && button <= 8) || (button == 35 || button == 36)) // lack of defines; for the mouse wheel
		gss.rotateCam(glm::vec3(0.0, 0.0, button % 2 == 1 ? -0.5 : 0.5));
	
	switch (state)
	{
	case GLUT_DOWN:
		if (button == GLUT_LEFT_BUTTON)
		{
			pressedKey.insert(KEY_MOUSE);
			mouseCoord = glm::vec2(x, y);
		}
		break;
	case GLUT_UP:
		if (button == GLUT_LEFT_BUTTON) 
		{
			std::set<Key>::iterator it;
			if ((it = pressedKey.find(KEY_MOUSE)) != pressedKey.end())
				pressedKey.erase(it);
		}
		break;
	case MOTION_CALL:
		if (pressedKey.find(KEY_MOUSE) != pressedKey.end())
		{
			glm::vec2 newMouseCoord(x, y);
			glm::vec3 diff = glm::vec3(newMouseCoord - mouseCoord, 0.0);
			gss.rotateCam(diff * 0.2f);
			mouseCoord = newMouseCoord;
		}
		break;
	default:
		break;
	}
}

void Engine::reshapeHandler(int w, int h)
{
	gss.reshape(w, h);
}


void Engine::setPlane(int index)
{
	const glm::vec3 planePos[] = {
		glm::vec3(0.0, 0.0, 0.0),
		glm::vec3(10.0, 0.5, 0.0),
		glm::vec3(-10.0, 0.5, 0.0),
		glm::vec3(0.0, 0.5, -10.0),
		glm::vec3(0.0, 0.5, 10.0),
	};
	const glm::vec3 planeRot[] = {
		glm::vec3(0.0, 0.0, 0.0),
		glm::vec3(0.0, 0.0, M_PI / 2.0),
		glm::vec3(0.0, 0.0, -M_PI / 2.0),
		glm::vec3(M_PI / 2.0, 0.0, 0.0),
		glm::vec3(-M_PI / 2.0, 0.0, 0.0),
	};
	const glm::vec3 planeScale[] = {
		glm::vec3(10.0, 1.0, 10.0),
		glm::vec3(0.5, 1.0, 10.0),
		glm::vec3(0.5, 1.0, 10.0),
		glm::vec3(10.0, 1.0, 0.5),
		glm::vec3(10.0, 1.0, 0.5),
	};
	const glm::vec2 planeTexScale[] = {
		glm::vec2(10.0, 10.0),
		glm::vec2(0.5, 10.0),
		glm::vec2(0.5, 10.0),
		glm::vec2(10.0, 0.5),
		glm::vec2(10.0, 0.5),
	};

	plane.setWorldPos(planePos[index]);
	plane.setRotate(planeRot[index]);
	plane.setScale(planeScale[index]);
	plane.setTextureScale(planeTexScale[index]);
}


/*=================================
		   Call Handlers
===================================*/

void Engine::timerMediator(int value)
{
	engine->workCycle();
	glutTimerFunc(TIMER_SPEED, Engine::timerMediator, 0);
}

void Engine::drawCallMediator()
{
	engine->drawHandler();
}

void Engine::keyboardCallMediator(unsigned char key, int x, int y)
{
	engine->keyPressHandler(key, x, y, true);
}

void Engine::keyboardUpCallMediator(unsigned char key, int x, int y)
{
	engine->keyPressHandler(key, x, y, false);
}

void Engine::mouseCallMediator(int button, int state, int x, int y)
{
	engine->mouseHandler(button, state, x, y);
}

void Engine::mouseMotionCallMediator(int x, int y)
{
	engine->mouseHandler(0, MOTION_CALL, x, y);
}

void Engine::reshapeCallMediator(int w, int h)
{
	engine->reshapeHandler(w, h);
} 