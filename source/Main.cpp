#include <cstdlib>
#include <ctime>
#include <Windows.h>
#include <string>
#include "Scene.h"
#include "TraceLib.h"
#include "freeglut\freeglut.h"
#include "Error.h"

DWORD WINAPI imageCalculation(LPVOID) {
	try
	{
		srand(timeGetTime());
		Scene::Instance().CalcualteImage();
	}
	catch(const Error& err)
	{
		err.HandleError();
	}
	catch(...)
	{
		Error::HandleDefaultError();
	}
	return 0;
}

void redisplay()
{
	// Нарисовать текущее изображение
	glClear(GL_COLOR_BUFFER_BIT);
	Scene::Instance().DrawPicture();
	glFlush();

	// Обновить окно
	glutShowWindow();
}

void keyboard(unsigned char key, int, int) {
	switch (key) {
	case 27:
		exit(0);
		break;
	default:
		Scene::Instance().CameraHandler(key);
	}
}

void specialKeyboard(int key, int, int) 
{
	Scene::Instance().SpecialKeysCameraHandler(key);
}

int main(int argc, char** argv)
{
	try
	{
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);

		glutInitWindowSize((GLint)CAMERA_PROPERTIES::width, (GLint)CAMERA_PROPERTIES::height);
		glutCreateWindow("Photon mapping");

		glutDisplayFunc(redisplay);
		glutKeyboardFunc(keyboard);
		glutSpecialFunc (specialKeyboard);
		
		Scene::Instance();

		CreateThread(NULL, 0, imageCalculation, NULL, 0, NULL);

		glutMainLoop();
	}
	catch(const Error& err)
	{
		err.HandleError();
	}
	catch(...)
	{
		Error::HandleDefaultError();
	}
	return 0;
}
