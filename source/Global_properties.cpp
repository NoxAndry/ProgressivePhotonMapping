#include "Global_properties.h"

const int NUMBER_OF_CORES = 2;

const size_t GLOBAL_PROPERTIES::kNearest = 100;
const double GLOBAL_PROPERTIES::EPS = 1e-9;
const double GLOBAL_PROPERTIES::INFINITY = 1e20;
const double GLOBAL_PROPERTIES::PI = 3.14159265;
const size_t GLOBAL_PROPERTIES::mappingTime = 1000;
const double GLOBAL_PROPERTIES::initMaxDist = 1000;
const size_t GLOBAL_PROPERTIES::numberOfPhotonsFirstIteration = 100000;
const size_t GLOBAL_PROPERTIES::numberOfPhotonsPerIteration = 100000;
	  size_t GLOBAL_PROPERTIES::numberOfEmittedPhotons = 0;
	  size_t GLOBAL_PROPERTIES::numberOfProcessedPhotons = 0;
const double GLOBAL_PROPERTIES::attenuationThreshold = 0.01;
const double GLOBAL_PROPERTIES::attenuationCoeficient = 0.9;
const unsigned char GLOBAL_PROPERTIES::diffuseReflectionCoefficient = 50;
const size_t GLOBAL_PROPERTIES::NumberPhotonTraseSteps = 500;
const double GLOBAL_PROPERTIES::start_radius = 3;
const double GLOBAL_PROPERTIES::PPMAlpha = 0.7;
const double GLOBAL_PROPERTIES::ErrorCoeficent = 0.1;
const size_t GLOBAL_PROPERTIES::hitNumberThreshold = 10;

#ifdef SUPERSAMPLING
const float GLOBAL_PROPERTIES::SamplingCoefficient = 0.25f;
#else
const float GLOBAL_PROPERTIES::SamplingCoefficient = 1.f;
#endif

#ifdef _DEBUG
const size_t GLOBAL_PROPERTIES::numberOfThreads = 1;
#else
const size_t GLOBAL_PROPERTIES::numberOfThreads = NUMBER_OF_CORES;
#endif


	  size_t CAMERA_PROPERTIES::width = 800;
	  size_t CAMERA_PROPERTIES::height = 800;
const double CAMERA_PROPERTIES::focus_distance = 5000;
const double CAMERA_PROPERTIES::view_angle = GLOBAL_PROPERTIES::PI/3;
const double CAMERA_PROPERTIES::camera_speed = 7;
const double CAMERA_PROPERTIES::camera_rotation_speed = 0.1;

void separate()
{
	std::cout << "----------------------------------------------------" << std::endl;
}