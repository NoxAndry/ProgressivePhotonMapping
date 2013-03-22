#include "PhotonTraceEngine.h"

void PhotonTraceEngine::SetObjects(ObjectsKD* okdp)
{
	m_objKD = okdp;		
}
	
void PhotonTraceEngine::SetLights(Lights* s)
{
	m_lights = s;
}

PhotonTraceEngine::Photons* PhotonTraceEngine::Emitting(const size_t numberOfEmittedPhotons) const
{
	DWORD startTime = timeGetTime();
	Photons *result = new Photons();
	if (m_lights->empty())
		throw Error("No lights in scene");
	const size_t numberForEach = numberOfEmittedPhotons/(m_lights->size());
	
	for(std::vector<LightSource*>::iterator it = m_lights->begin(); it != m_lights->end(); ++it)
	{
		const Color PowerEachPhoton = (*it)->GetPower()/static_cast<float>(numberOfEmittedPhotons);

		/////////////////////////////////////////////////////////////
		// parallelization
		/////////////////////////////////////////////////////////////
		Photons *photons = new Photons[GLOBAL_PROPERTIES::numberOfThreads];

		DWORD *tID		= new DWORD [GLOBAL_PROPERTIES::numberOfThreads];
		HANDLE *handles = new HANDLE[GLOBAL_PROPERTIES::numberOfThreads];

		for(size_t i = 0; i<GLOBAL_PROPERTIES::numberOfThreads; i++)
		{
			PhotonTraceThreadArguments *arguments
				= new PhotonTraceThreadArguments(photons[i], *this, numberForEach, it, PowerEachPhoton, i, rand());

			handles[i] = CreateThread(
				NULL,
				0,
				ThreadEmitting,
				(LPVOID)arguments,
				0,
				&tID[i]);
		}

		WaitForMultipleObjects(GLOBAL_PROPERTIES::numberOfThreads, handles, TRUE, INFINITE);
		
		size_t memCount = 0;
		for(size_t i = 0; i<GLOBAL_PROPERTIES::numberOfThreads; ++i)
		{
			memCount += photons[i].size();
		}
		result->reserve(memCount);

		for(size_t i = 0; i<GLOBAL_PROPERTIES::numberOfThreads; ++i)
		{
			result->insert(result->end(), photons[i].begin(), photons[i].end());
		}
		delete[] photons;

		delete tID;
		delete handles;
		/////////////////////////////////////////////////////////////
	}

	std::cout << "Photons emitting was over in " << (timeGetTime() - startTime) << " ms" << std::endl;
	return result;
}


DWORD WINAPI PhotonTraceEngine::ThreadEmitting(LPVOID lp)
{
	try
	{
		DWORD begTime = timeGetTime();

		PhotonTraceThreadArguments* arguments = (PhotonTraceThreadArguments*)lp;
		
		srand(arguments->randomSeed);
		
		for(size_t i = 0; i < arguments->numberForEach/GLOBAL_PROPERTIES::numberOfThreads; i++)
		{
			arguments->engine.Trace(arguments->photons, DirectedPhoton((*arguments->it)->GetRay(), arguments->powerForEach));
		}

		std::cout << "Thread " << arguments->threadID << " finished its work in " << (timeGetTime() - begTime) << " ms" << std::endl;

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

void PhotonTraceEngine::Trace(Photons& result, DirectedPhoton &comingPhoton) const
{
	if (comingPhoton.hitNumber > GLOBAL_PROPERTIES::hitNumberThreshold)
		return;

	comingPhoton.hitNumber++;

	Ray curRay(comingPhoton.position, comingPhoton.direction);
		
	KDObjectTree::KDIntersectQueryReturn intersect = m_objKD->FindIntersection(curRay, 0, GLOBAL_PROPERTIES::INFINITY);//+
	Point intersection  = curRay.GetPointOnRay(intersect.t_intersect);
	Shape *intersectedShape = intersect.intersectShape;
		
	if(!intersectedShape) return;

	Material *current = intersectedShape->GetMaterial(intersection);

	double randomValueBetweenZeroAndOne = (rand()^rand())/(double)RAND_MAX;

	if(randomValueBetweenZeroAndOne < current->Pd) 
	{
		comingPhoton.position = intersection;

		result.push_back(Photon(comingPhoton, intersectedShape->index));
		
		Vector N = intersectedShape->GetNormal(intersection);
		Vector direction(0,0,0);

		do 
		{
			direction = Vector::GetRandomVector();
		} while((direction%N)*(comingPhoton.direction%N) > GLOBAL_PROPERTIES::EPS);

		DirectedPhoton newPhoton(intersection, direction, comingPhoton.power * current->diffuse, comingPhoton.hitNumber+1);	
			
		Trace(result,newPhoton);
	} 
	else if(randomValueBetweenZeroAndOne < current->Pd + current->Ps) 
	{
		Vector Normal = intersectedShape->GetNormal(intersection);
		Vector reflectedRay = (comingPhoton.direction - (Normal *( 2 * (Normal%comingPhoton.direction))));
		DirectedPhoton newPhoton(intersection, reflectedRay, comingPhoton.power * current->spcc, comingPhoton.hitNumber+1);	
			
		Trace(result,newPhoton);	
	}
	else if(randomValueBetweenZeroAndOne < current->Pd + current->Ps + current->Pt)
	{
		Vector Normal = intersectedShape->GetNormal(intersection);
		bool isFromExtern = intersectedShape->IsFromExternal(curRay, intersection);

		const double teta1 = isFromExtern ? current->externalRefCoeficient : current->internalRefCoeficient;
		const double teta2 = isFromExtern ? current->internalRefCoeficient : current->externalRefCoeficient;

		if (!isFromExtern) 
			Normal = -Normal;
		Vector OffsetVector = Normal*GLOBAL_PROPERTIES::EPS;
		const Vector incidentDir = comingPhoton.direction;

		const double cosi = (-incidentDir)%Normal;

		const double sinisq = (1 - cosi*cosi);

		const double tetarel = teta1/teta2;

		const double tetarelsq = tetarel*tetarel;

		bool TIR = tetarelsq*sinisq > 1;

		double reflect = ReflectShrink(teta1, teta2, tetarelsq, cosi, TIR);

		double transmit = 1 - reflect;

		if (transmit > GLOBAL_PROPERTIES::EPS)
		{
			Vector transmittedDir = incidentDir * tetarel + Normal*(tetarel*cosi - sqrt(1 - tetarelsq*(1-cosi*cosi)));
			Trace(result,DirectedPhoton(intersection - OffsetVector, transmittedDir, comingPhoton.power * current->transmit * transmit, comingPhoton.hitNumber+1));
		}

		if (reflect > GLOBAL_PROPERTIES::EPS)
		{
			Vector reflectedDir = incidentDir + Normal*2*cosi;
			Trace(result,DirectedPhoton(intersection + OffsetVector, reflectedDir, comingPhoton.power * reflect, comingPhoton.hitNumber+1));
		}
	}
	else
	{
		comingPhoton.position = intersection;
		result.push_back(Photon(comingPhoton, intersectedShape->index));
	}
}

double PhotonTraceEngine::ReflectShrink(const double teta1, const double teta2, const double tetarelsq, double cosx, const bool TIR) const
{
	if (teta1 > teta2 && TIR)
		return 1.0;

	double r0 = (teta1-teta2)/(teta1+teta2);
	r0 *= r0;

	if (teta1 > teta2)
		cosx = sqrt(1.0 - tetarelsq*(1 - cosx*cosx));

	const double x = 1.0 - cosx;
	return r0 + (1.0 - r0)*x*x*x*x*x;
}