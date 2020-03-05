#pragma once

#include <string>

class Material;

class IRenderer
{
public:
	virtual Material* CreateMaterial(std::string base = "default-fwd") = 0;


};