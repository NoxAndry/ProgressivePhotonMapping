#pragma once
#include <string>
#include <vector>
#include "TraceLib.h"
#include "GeometryLib.h"
#include "Shapes.h"
#include "LightSource.h"
#include "RayTracerEngine.h"
#include "PhotonTraceEngine.h"
#include "Camera.h"
#include "KDObjectTree.h"
#include "freeglut\freeglut.h"
#include "l3ds.h"

class Scene
{
	friend class RayTraceEngine;
	friend class PhotonTraceEngine;

	Scene();
	Scene& operator=(const Scene&);
	Scene(const Scene&);

public:
	typedef std::vector<Photon> Photons;
	typedef std::vector<Shape*> Shapes;

	static Scene& Instance();

	~Scene();

	void CalcualteImage();

	void CameraHandler(const unsigned char key) const;
	void SpecialKeysCameraHandler(const int key) const;
	void DrawPicture() const;

private:
	struct HitPointsCalculateThreadArguments;
	struct HitPointsUpdateThreadArgument;

	void BuildCornellBoxScene(std::vector<Shape*> &objects);
	void UpdatePixelMap();
	void CalculateHitPoints();
	void UpdateHitPointVector(KDPhotonsTree* stepTree);
	
	static DWORD WINAPI CalculateHitPointsThread(LPVOID lp);
	static DWORD WINAPI UpdateHitPointVectorThread(LPVOID lp);

	unsigned char* OpenGLAdapter() const;
	void InitPixelMatrix(Pixel** &pixels);
	void AddObject(const std::string& fname,Material* material, Shapes &objects, const Point &ofs = Point(0,0,0)) const;
	void AddTexturedObject(const std::string fname, Material* material, Shapes &objects, const std::string textName, const Point &ofs = Point(0,0,0)) const;

	Pixel** pixelMatrix;
	std::vector<LightSource*> lights;
	KDObjectTree *objectsTree;

	std::vector<HitPoint> *HPVector;
	RayTraceEngine traceEngine;
	PhotonTraceEngine photonEngine;
	Camera *camera;
};

/////////////////////////////////////////////////////////
// Вспомогательные структуры
/////////////////////////////////////////////////////////
struct Scene::HitPointsCalculateThreadArguments
{
	RayTraceEngine			 *rayTracerEngine;
	Camera					 *camera;
	size_t					  threadID;
	std::vector<HitPoint>	 &hitPoints;

	HitPointsCalculateThreadArguments (
		RayTraceEngine			 *rayTracerEngine,
		Camera					 *camera,
		size_t					  ID,
		std::vector<HitPoint>	 &hitPoints
		)
		:	rayTracerEngine(rayTracerEngine),
		camera(camera),
		threadID(ID),
		hitPoints(hitPoints)
	{ }
};

struct Scene::HitPointsUpdateThreadArgument
{
	KDPhotonsTree			*stepTree;
	size_t					 threadID;
	std::vector<HitPoint>	*HPVector;

	HitPointsUpdateThreadArgument (
		KDPhotonsTree			*stepTree,
		size_t					 threadID,
		std::vector<HitPoint>	*HPVector
		)
	:	stepTree(stepTree),
		threadID(threadID),
		HPVector(HPVector)
	{ }
};
/////////////////////////////////////////////////////////