#pragma once

#include <4dm.h>

struct Config
{
	bool nightMode = true;
	float bloomIntensity = 1.0f;
	int fireworkParticleCount = 2000;
	bool presentChests = true;
};

extern Config config;

void loadConfig();
void saveConfig();
