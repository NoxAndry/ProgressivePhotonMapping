#pragma once
#include "KDPhotonsTree.h"
#include "TraceLib.h"
#include "Shapes.h"
#include "KDObjectTree.h"
#include "LightSource.h"
#include "math.h"
#include "stdlib.h"

class RayTraceEngine
{
	typedef KDObjectTree ObjectsKD;
	typedef std::vector<HitPoint> HitPointVector; 
	typedef std::vector<LightSource*> Lights;

public:
	void SetHPVector( HitPointVector* hpv);
	void SetObjects(ObjectsKD* okdp);
	void SetLights(Lights* lvec);

	void TracePixel (const Ray& incoming, const int x, const int y, const Color& coeficient,const size_t threadID, std::vector<HitPoint> &hitPoints) const;
	void FillHPVector(const HitPointVector *vectors);

private:

	double ReflectFreshnel(const double& teta1, const double& teta2, const double& tetarelsq, const double& sinxsq, double cosx, const bool TIR) const;
	Color CalculateSpecular(const Point& bestMatch, const Vector& Normal, const Color& specular, const Vector& traceDirection) const;
	
	ObjectsKD *m_objKD;
	HitPointVector *m_HPVector;
	Lights* m_LVector;
};