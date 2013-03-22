#pragma once
#include "LightSource.h"
#include "KDPhotonsTree.h"
#include "TraceLib.h"
#include "Shapes.h"
#include "KDObjectTree.h"

class PhotonTraceEngine
{
public:
	typedef KDObjectTree ObjectsKD;
	typedef std::vector<LightSource*> Lights;
	typedef std::vector<Photon> Photons;

	PhotonTraceEngine() {}
	
	void SetObjects(ObjectsKD* okdp);
	void SetLights(Lights* ls);
	Photons* Emitting(const size_t numberOfEmittedPhotons = GLOBAL_PROPERTIES::numberOfPhotonsPerIteration) const;

private:
	struct PhotonTraceThreadArguments;

	static DWORD WINAPI ThreadEmitting(LPVOID lp);
	void Trace(Photons& result, DirectedPhoton& comingPhoton) const;
	double ReflectShrink(const double teta1, const double teta2, const double tetarelsq, double cosx, const bool TIR) const;	
	
	ObjectsKD *m_objKD;
	Lights* m_lights;
};

/////////////////////////////////////////////////////////
// Вспомогательные структуры
/////////////////////////////////////////////////////////
struct PhotonTraceEngine::PhotonTraceThreadArguments
{
	typedef PhotonTraceEngine::Photons Photons;
	typedef PhotonTraceEngine::Lights  Lights;

		  Photons									&photons;
	const PhotonTraceEngine							&engine;
	const size_t									 numberForEach;
	const Lights::iterator							 it;
	const Color										 powerForEach;
	const size_t									 threadID;
	const size_t									 randomSeed;

	PhotonTraceThreadArguments(
				  Photons									&photons,
			const PhotonTraceEngine							&engine,
			const size_t									 numberForEach,
			const Lights::iterator						 	 it,
			const Color										 powerForEach,
			const size_t									 threadID,
			const size_t									 randomSeed
		)
	:	photons(photons),
		engine(engine),
		numberForEach(numberForEach),
		it(it),
		powerForEach(powerForEach),
		threadID(threadID),
		randomSeed(randomSeed)
	{}
};
/////////////////////////////////////////////////////////