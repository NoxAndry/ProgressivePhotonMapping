#include "Scene.h"

Scene::Scene()
	: photonEngine()
{
	InitPixelMatrix(pixelMatrix);

	std::vector<Shape*> objects;
	HPVector = new std::vector<HitPoint>;

	BuildCornellBoxScene(objects);

	separate();
		
	std::cout << "Number of shapes in the scene: " << objects.size() << std::endl;

	DWORD startTime = timeGetTime();
	objectsTree = new KDObjectTree(objects, 
		BoundingBox(	 Point(GLOBAL_PROPERTIES::INFINITY, GLOBAL_PROPERTIES::INFINITY, GLOBAL_PROPERTIES::INFINITY), 
						-Point(GLOBAL_PROPERTIES::INFINITY, GLOBAL_PROPERTIES::INFINITY, GLOBAL_PROPERTIES::INFINITY)) );

	std::cout << "Objects KDtree build was over in " << (timeGetTime() - startTime) << " ms" << std::endl;
	photonEngine.SetObjects(objectsTree);
	photonEngine.SetLights(&lights);

	traceEngine.SetLights(&lights);
	traceEngine.SetObjects(objectsTree);
	traceEngine.SetHPVector(HPVector);

	separate();

	std::cout << "Start to calculate hit points" << std::endl;
	startTime = timeGetTime();
	CalculateHitPoints();
	std::cout << "Hit points calculation was over in " << (timeGetTime() - startTime) << " ms, with " << HPVector->size() << " points" << std::endl;
	separate();
}

Scene& Scene::Instance()
{
	static Scene scene;
	return scene;
}

Scene::~Scene()
{
	delete HPVector;
	delete objectsTree;
}

void Scene::CalcualteImage()
{
	DWORD calcImageStart = timeGetTime();

	// Loop through iterations
	for (size_t it = 0; it < GLOBAL_PROPERTIES::NumberPhotonTraseSteps; ++it)
	{
		size_t currentPhotonNumber = it ? GLOBAL_PROPERTIES::numberOfPhotonsPerIteration : GLOBAL_PROPERTIES::numberOfPhotonsFirstIteration;

		DWORD wholeTimeStart = timeGetTime();

		GLOBAL_PROPERTIES::numberOfEmittedPhotons += currentPhotonNumber;

		std::cout << "-- Iteration " << (it+1) << std::endl;

		std::cout << "Start to update hit points" << std::endl;
		DWORD startTime = timeGetTime();
		UpdateHitPointVector(new KDPhotonsTree(photonEngine.Emitting(currentPhotonNumber)));

		std::cout << "Start to update pixel matrix" << std::endl;
		startTime = timeGetTime();
		UpdatePixelMap();
		std::cout << "Pixel matrix update was over in " << (timeGetTime() - startTime) << " ms" << std::endl;
			
		std::cout << "-- Whole time for iteration: " << (timeGetTime() - wholeTimeStart) << " ms" << std::endl;

		separate();

		glutPostRedisplay();
	}
		
	std::cout << std::endl;
	separate();
	std::cout << "-- Whole time for all iterations: " << (timeGetTime() - calcImageStart)/1000 << " seconds" << std::endl;
	separate();
	std::cout << "-- Whole number of processed photons: " << GLOBAL_PROPERTIES::numberOfProcessedPhotons << std::endl;
	separate();
}

void Scene::CameraHandler(const unsigned char key) const
{
	switch(key)
	{
	case 'w':
		camera->Move_forward(CAMERA_PROPERTIES::camera_speed);
		break;
	case 'a':
		camera->Move_left(CAMERA_PROPERTIES::camera_speed);
		break;
	case 's':
		camera->Move_forward(-CAMERA_PROPERTIES::camera_speed);
		break;
	case 'd':
		camera->Move_left(-CAMERA_PROPERTIES::camera_speed);
		break;
	}
}

void Scene::SpecialKeysCameraHandler(const int key) const
{
	switch(key)
	{
	case 102:
		camera->Rotate(CAMERA_PROPERTIES::camera_rotation_speed);
		break;
	case 100:
		camera->Rotate(-CAMERA_PROPERTIES::camera_rotation_speed);
		break;
	case 103:
		camera->Move_forward(-CAMERA_PROPERTIES::camera_speed);
		break;
	case 101:
		camera->Move_forward(CAMERA_PROPERTIES::camera_speed);
		break;
	}
}

void Scene::DrawPicture() const
{
	unsigned char* pict = OpenGLAdapter();

	glDrawPixels(CAMERA_PROPERTIES::width,CAMERA_PROPERTIES::height,GL_RGB, GL_UNSIGNED_BYTE,pict);

	delete[] pict;
}

void Scene::BuildCornellBoxScene(std::vector<Shape*> &objects)
{
	camera = new Camera(
		Point(-260, 0, 90),
		Point(-199, 0, 90)
		);

	float lightEnergy = 35e8;

	lights.push_back(new SphereLight(Point(0, 0, 160), 3, Color(lightEnergy,lightEnergy,lightEnergy),1));

	Color walls(0.1f,0.1f,0.1f);
	Material* white = new Material(Ambient(0.3,0.3,0.3),Diffuse(0.9, 0.9, 0.9),walls);

	AddObject("files\\cornell_box\\cornell_box_floor.3ds", new Material(Color(0.3,0.3,0.3),Color(0.9,0.9,0.9),walls, Reflect(0.2, 0.2, 0.2)), objects);
	AddObject("files\\cornell_box\\cornell_box_ceil.3ds", white, objects);
	AddObject("files\\cornell_box\\cornell_box_right_wall.3ds", new Material(Color(0.,0.1,0.),Color(0.4,1,0.4),walls), objects);
	AddObject("files\\cornell_box\\cornell_box_back.3ds", white, objects);
	AddObject("files\\cornell_box\\cornell_box_left_wall.3ds", new Material(Color(0.1,0.,0.),Color(1,0.4,0.4),walls), objects);

	AddObject("files\\cornell_box\\cornell_box_box1.3ds", white, objects);
	AddObject("files\\cornell_box\\cornell_box_box2.3ds", white, objects);

	objects.push_back(new Sphere(Point(-35,-45,20),15,new Material(Ambient(0,0,0),Diffuse(0,0,0),Specular(0,0,0), Reflect(0,0,0), Refract(1,1,1), 1.51, 1), Shape::GetUniqueID()));
	objects.push_back(new Sphere(Point(-50,50,15),15,new Material(Ambient(0,0,0),Diffuse(0,0,0),Specular(0,0,0), Reflect(1,1,1)), Shape::GetUniqueID() ));
}

void Scene::UpdatePixelMap()
{
	Pixel **tmp;
	InitPixelMatrix(tmp);

	for (std::vector<HitPoint>::iterator it = HPVector->begin(); it != HPVector->end(); ++it)
	{
		tmp[it->x][it->y] += it->CalculateRadiance();
	}

	for (size_t i = 0; i < CAMERA_PROPERTIES::height; i++)
	{
		memcpy(pixelMatrix[i], tmp[i] ,CAMERA_PROPERTIES::width*sizeof(Pixel));
		delete[] tmp[i];
	}
	delete[] tmp;
}

void Scene::CalculateHitPoints()
{
	/////////////////////////////////////////////////////////////
	// parallelization
	/////////////////////////////////////////////////////////////
	std::vector<HitPoint> *vectors
		= new std::vector<HitPoint>[GLOBAL_PROPERTIES::numberOfThreads];
		
	DWORD *tID		= new DWORD [GLOBAL_PROPERTIES::numberOfThreads];
	HANDLE *handles = new HANDLE[GLOBAL_PROPERTIES::numberOfThreads];

	for(size_t i = 0; i<GLOBAL_PROPERTIES::numberOfThreads; i++)
	{
		HitPointsCalculateThreadArguments *arguments
			= new HitPointsCalculateThreadArguments(&traceEngine, camera, i, vectors[i]);

		handles[i] = CreateThread(
			NULL,
			0,
			CalculateHitPointsThread,
			(LPVOID)arguments,
			0,
			&tID[i]);
	}

	WaitForMultipleObjects(GLOBAL_PROPERTIES::numberOfThreads, handles, TRUE, INFINITE);

	delete tID;
	delete handles;

	traceEngine.FillHPVector(vectors);

	delete[] vectors;
	/////////////////////////////////////////////////////////////
}

void Scene::UpdateHitPointVector(KDPhotonsTree* stepTree)
{
	DWORD startTime = timeGetTime();

	/////////////////////////////////////////////////////////////
	// parallelization
	/////////////////////////////////////////////////////////////
	DWORD *tID		= new DWORD [GLOBAL_PROPERTIES::numberOfThreads];
	HANDLE *handles = new HANDLE[GLOBAL_PROPERTIES::numberOfThreads];

	for(size_t i = 0; i<GLOBAL_PROPERTIES::numberOfThreads; i++) {
		HitPointsUpdateThreadArgument *arguments =
			new HitPointsUpdateThreadArgument(stepTree, i, HPVector);

		handles[i] = CreateThread(
			NULL,
			0,
			UpdateHitPointVectorThread,
			(LPVOID)arguments,
			0,
			&tID[i]);
	}

	WaitForMultipleObjects(GLOBAL_PROPERTIES::numberOfThreads, handles, TRUE, INFINITE);

	delete tID;
	delete handles;
	/////////////////////////////////////////////////////////////
		
	delete stepTree;

	std::cout << "Hit points update was over in " << (timeGetTime() - startTime) << " ms" << std::endl;
}
	
DWORD WINAPI Scene::CalculateHitPointsThread(LPVOID lp)
{
	try
	{
		HitPointsCalculateThreadArguments *arguments
			= (HitPointsCalculateThreadArguments*)lp;

		for (size_t i = arguments->threadID; i < CAMERA_PROPERTIES::height; i+=GLOBAL_PROPERTIES::numberOfThreads)
			for (size_t j = 0; j < CAMERA_PROPERTIES::width; j++)
			{	
#ifdef SUPERSAMPLING
				arguments->rayTracerEngine->TracePixel(arguments->camera->getRayForTracing(j + 0.25,i + 0.25),i,j,Color(0.25,0.25,0.25),0,arguments->hitPoints);
				arguments->rayTracerEngine->TracePixel(arguments->camera->getRayForTracing(j + 0.25,i - 0.25),i,j,Color(0.25,0.25,0.25),0,arguments->hitPoints);
				arguments->rayTracerEngine->TracePixel(arguments->camera->getRayForTracing(j - 0.25,i + 0.25),i,j,Color(0.25,0.25,0.25),0,arguments->hitPoints);
				arguments->rayTracerEngine->TracePixel(arguments->camera->getRayForTracing(j - 0.25,i - 0.25),i,j,Color(0.25,0.25,0.25),0,arguments->hitPoints);
#else
				arguments->rayTracerEngine->TracePixel(arguments->camera->GetRayForTracing(j, i),i,j,Color(1,1,1),0, arguments->hitPoints);
#endif
			}
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

DWORD WINAPI Scene::UpdateHitPointVectorThread(LPVOID lp)
{
	try
	{
		HitPointsUpdateThreadArgument *arguments = 
			(HitPointsUpdateThreadArgument *)lp;
		DWORD startTime = timeGetTime();

		for(size_t i = arguments->threadID; i<arguments->HPVector->size(); i+=GLOBAL_PROPERTIES::numberOfThreads)
		{
			(*arguments->HPVector)[i].UpdateHitPoint(arguments->stepTree);
		}

		std::cout << "Thread " << arguments->threadID << " finished its work in " << (timeGetTime() - startTime) << " ms" << std::endl;
		delete arguments;
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

unsigned char* Scene::OpenGLAdapter() const
{
	unsigned char* ret = new unsigned char[CAMERA_PROPERTIES::height *  CAMERA_PROPERTIES::width * 3+1];
	int curP = 0;

	for (int i = CAMERA_PROPERTIES::height-1; i >=0; i--)
		for (size_t j = 0; j< CAMERA_PROPERTIES::width  ; j++)
		{
			Color A = pixelMatrix[i][j].ScaledForOpenGL();

			ret[curP++] = static_cast<unsigned char>(A.R*255);
			ret[curP++] = static_cast<unsigned char>(A.G*255);
			ret[curP++] = static_cast<unsigned char>(A.B*255);
		}
		return ret;
}

void Scene::InitPixelMatrix(Pixel** &pixels)
{
	pixels = new Pixel*[CAMERA_PROPERTIES::height];
	for (size_t i = 0; i < CAMERA_PROPERTIES::height; i++)
	{
		pixels[i] = new Pixel[CAMERA_PROPERTIES::width];
		memset(pixels[i], 0 ,CAMERA_PROPERTIES::width*sizeof(Pixel));
	}
}

void Scene::AddObject(const std::string& fname,Material* material, Shapes &objects, const Point &ofs) const
{
	size_t index = Shape::GetUniqueID();

	// —читывание меша из файла
	L3DS *l3ds = new L3DS(fname.c_str());
	if(!l3ds || !l3ds->GetMeshCount())
		throw Error("Error in loading extern files");

	for(int i = 0; i<l3ds->GetMeshCount(); i++) {
		LMesh *mesh = l3ds->GetMesh(i);

		for(int j = 0; j<mesh->GetTriangleCount(); j++) {
			LTriangle tr = mesh->GetTriangle(j);

			Point a(mesh->GetVertex(tr.a).x, mesh->GetVertex(tr.a).y, mesh->GetVertex(tr.a).z);
			Point b(mesh->GetVertex(tr.b).x, mesh->GetVertex(tr.b).y, mesh->GetVertex(tr.b).z);
			Point c(mesh->GetVertex(tr.c).x, mesh->GetVertex(tr.c).y, mesh->GetVertex(tr.c).z);	

			if (Triangle::IsValidTrangle(a,b,c))
				objects.push_back(new Triangle(a-ofs, b-ofs, c-ofs, material, index));
		}
	}
}

void Scene::AddTexturedObject(const std::string fname, Material* material, Shapes &objects, const std::string textName, const Point &ofs) const
{
	size_t index = Shape::GetUniqueID();

	// —читывание меша из файла
	L3DS *l3ds = new L3DS(fname.c_str());
	if(!l3ds || !l3ds->GetMeshCount())
		throw Error("Error in loading extern files");

	for(int i = 0; i<l3ds->GetMeshCount(); i++) {
		LMesh *mesh = l3ds->GetMesh(i);

		Texture *texture = new Texture(textName.c_str());

		for(int j = 0; j<mesh->GetTriangleCount(); j++) {
			LTriangle tr = mesh->GetTriangle(j);

			Point a(mesh->GetVertex(tr.a).x, mesh->GetVertex(tr.a).y, mesh->GetVertex(tr.a).z);
			Point b(mesh->GetVertex(tr.b).x, mesh->GetVertex(tr.b).y, mesh->GetVertex(tr.b).z);
			Point c(mesh->GetVertex(tr.c).x, mesh->GetVertex(tr.c).y, mesh->GetVertex(tr.c).z);	

			TextureCoords tA(mesh->GetUV(tr.a).u, mesh->GetUV(tr.a).v);
			TextureCoords tB(mesh->GetUV(tr.b).u, mesh->GetUV(tr.b).v);
			TextureCoords tC(mesh->GetUV(tr.c).u, mesh->GetUV(tr.c).v);

			tA.InvertU();
			tB.InvertU();
			tC.InvertU();

			tA*=8;
			tB*=8;
			tC*=8;

			if (Triangle::IsValidTrangle(a,b,c))
				objects.push_back(new Triangle(a-ofs, b-ofs, c-ofs, material, index, texture, tA, tB, tC));
		}
	}
}
