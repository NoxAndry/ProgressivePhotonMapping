#include "RayTracerEngine.h"

void RayTraceEngine::SetHPVector( HitPointVector* hpv)
{
	m_HPVector = hpv;		
}

void RayTraceEngine::SetObjects(ObjectsKD* okdp)
{
	m_objKD = okdp;
}
	
void RayTraceEngine::SetLights(Lights* lvec)
{
	m_LVector = lvec;
}

void RayTraceEngine::TracePixel (const Ray& incoming, const int x, const int y, const Color& coeficient,const size_t threadID, std::vector<HitPoint> &hitPoints) const
{
	double cutCoeficent = 0.001;
	if (coeficient.R < cutCoeficent && coeficient.G < cutCoeficent && coeficient.B < cutCoeficent)
		return;

	const KDObjectTree::KDIntersectQueryReturn intersect = m_objKD->FindIntersection(incoming, 0, GLOBAL_PROPERTIES::INFINITY);

	Shape* intersectedShape = intersect.intersectShape;

	if (intersectedShape == NULL) return;

	Point bestMatch = incoming.GetPointOnRay(intersect.t_intersect);

	const Material* currentMaterial = intersectedShape->GetMaterial(bestMatch);
	const Texture * currentTexture  = intersectedShape->GetTexture (bestMatch);

	Vector Normal = intersectedShape->GetNormal(bestMatch);

	const Vector dir = incoming.GetDirection();

	//diffuse material
	if (currentMaterial->IsDiffuse())
	{
		hitPoints.push_back(HitPoint(bestMatch, 
										 x, 
										 y, 
										 coeficient*currentMaterial->diffuse, 
										 currentMaterial->ambient*coeficient,
										 CalculateSpecular(bestMatch,Normal,currentMaterial->specular,incoming.GetDirection())*coeficient,
										 (currentTexture 
												? (currentTexture->GetTextureCoeficent(intersectedShape->CalculateTextureCoordinates(incoming))) 
												: Color(0,0,0)) * coeficient,
										 intersectedShape->index,
										 intersectedShape->GetNormal(Point(0,0,0))
										 ));
	}

	//refract material
	if (currentMaterial->IsRefracting() && incoming.GetMass() > GLOBAL_PROPERTIES::attenuationThreshold)
	{

		bool isFromExtern = intersectedShape->IsFromExternal(incoming, bestMatch);

		const double teta1 = isFromExtern ? currentMaterial->externalRefCoeficient : currentMaterial->internalRefCoeficient;
		const double teta2 = isFromExtern ? currentMaterial->internalRefCoeficient : currentMaterial->externalRefCoeficient;

		if (!isFromExtern) 
			Normal = -Normal;

		const Vector OffsetVector = Normal*GLOBAL_PROPERTIES::EPS;

		const Vector incidentDir = incoming.GetDirection();

		const double cosi = (-incidentDir)%Normal;

		const double sinisq = (1 - cosi*cosi);

		const double tetarel = teta1/teta2;

		const double tetarelsq = tetarel*tetarel;

		bool TIR = tetarelsq*sinisq > 1;

		double transmit, reflect;
			
		double cost = sqrt(1 - tetarelsq*sinisq);
		if (TIR)
		{
			reflect = 1;
		}
		else
		{
			double tmp1 = (teta1*cosi - teta2*cost)/(teta1*cosi + teta2*cost);

			double tmp2 = (teta2*cosi - teta1*cost)/(teta2*cosi + teta1*cost);

			reflect = (tmp1*tmp1 + tmp2*tmp2)/2.;
		}

		transmit = 1 - reflect;

		double newMass = incoming.GetMass()* GLOBAL_PROPERTIES::attenuationCoeficient;
			
		if (transmit > GLOBAL_PROPERTIES::EPS)
		{			
			Vector transmittedDir = incidentDir * tetarel + Normal* (tetarel*cosi - sqrt(1 - tetarelsq*(1-cosi*cosi)));

			TracePixel(Ray(bestMatch - OffsetVector, transmittedDir, newMass), x, y, currentMaterial->transmit*coeficient * (float)transmit, threadID, hitPoints);
		}
			
		if (reflect > GLOBAL_PROPERTIES::EPS)
		{
			Vector reflectedDir = incidentDir + Normal*2*cosi;
			
			TracePixel(Ray(bestMatch + OffsetVector, reflectedDir, newMass), x, y,  currentMaterial->transmit * coeficient * (float)reflect   , threadID, hitPoints); 
		}

	}
	//reflect material
	if (currentMaterial->IsReflecting() && incoming.GetMass() > GLOBAL_PROPERTIES::attenuationThreshold )
	{
		const Vector OffsetVector = Normal*GLOBAL_PROPERTIES::EPS;
		Vector reflectedDir = incoming.GetDirection() + Normal*2*((-incoming.GetDirection())%Normal);
		TracePixel(Ray(bestMatch + OffsetVector, reflectedDir, incoming.GetMass()* GLOBAL_PROPERTIES::attenuationCoeficient), x, y, currentMaterial->reflect * coeficient, threadID, hitPoints);
	} 
}

void RayTraceEngine::FillHPVector(const HitPointVector *vectors)
{
	size_t memCount = 0;
	for(size_t i = 0; i<GLOBAL_PROPERTIES::numberOfThreads; ++i)
	{
		memCount += vectors[i].size();
	}
	m_HPVector->reserve(memCount);

	for(size_t i = 0; i<GLOBAL_PROPERTIES::numberOfThreads; ++i)
	{
		m_HPVector->insert(m_HPVector->begin(), vectors[i].begin(), vectors[i].end());
	}
}

double RayTraceEngine::ReflectFreshnel(const double& teta1, const double& teta2, const double& tetarelsq, const double& sinxsq, double cosx, const bool TIR) const
{
	if (TIR) return 1.0;
	double tetarel = sqrt(tetarelsq);
	double sin2t = tetarel*tetarel*sinxsq;
	double cost = sqrt(1-sin2t*sin2t);
	double t1 = (teta1*cosx - teta2*cost)/(teta1*cosx + teta2*cost);
	double t2 = (teta2*cosx - teta1*cost)/(teta2*cosx + teta1*cost);
	return (t1*t1 + t2*t2)/2.0;
}

Color RayTraceEngine::CalculateSpecular(const Point& bestMatch, const Vector& Normal, const Color& specular, const Vector& traceDirection) const
{
	Color ret;
	for (Lights::iterator it = m_LVector->begin(); it != m_LVector->end(); ++it)
	{
		Vector toSource = !( (*it)->GetCenter()- bestMatch );
		Vector sum = !(toSource - traceDirection);
		ret += specular * pow(std::max(0.f,float(sum%Normal) ), (*it)->GetShiness());
	}
	return ret;
}