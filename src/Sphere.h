#pragma once

#include "TriangleMesh.h"


class Sphere : public TriangleMesh {

public:
	Sphere(float rad, GLuint sl, GLuint st);
};