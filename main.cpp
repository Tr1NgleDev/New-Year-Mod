#include <4dm.h>

using namespace fdm;

#include <fxlib/FXLib.h>

#include <glm/gtc/random.hpp>

#include "ItemFirework.h"

initDLL

$hook(void, StateGame, init, StateManager& s)
{
	original(self, s);
}

$hook(void, StateGame, update, StateManager& s, double dt)
{
	original(self, s, dt);

	self->time = 1000;

	double time = glfwGetTime();

	// i was too lazy to make an entity for this
	for (size_t i = 0; i < ItemFirework::activeFireworks.size();)
	{
		Firework* firework = ItemFirework::activeFireworks[i];

		firework->explosion->update(dt);
		firework->smoke->update(dt);
		
		if (!firework->exploded)
		{
			firework->vel += firework->deviation * (float)dt * ((float)(time - firework->launchTime) * 0.75f + 0.25f);
			firework->pos += firework->vel * (float)dt;
		}

		firework->smoke->origin = firework->pos - glm::vec4(0, 0.5f, 0, 0);
		firework->explosion->origin = firework->pos;

		if (!firework->exploded && time - firework->lastSmokeEmission > 0.01f)
		{
			firework->lastSmokeEmission = time;

			firework->smoke->emit(2);
		}

		if (!firework->exploded && time - firework->launchTime > 3.f)
		{
			firework->explosion->emit(firework->explosion->getMaxParticles());
			firework->exploded = true;
			SoLoud::handle soundHandle
				= AudioManager::playSound4D(ItemFirework::explosion_ogg, "ambience", firework->pos, {0,0,0,0});
			AudioManager::soloud.setRelativePlaySpeed(soundHandle, glm::linearRand(0.9f, 1.1f));
		}

		if (firework->exploded && firework->explosion->getAliveParticlesCount() == 0 && firework->smoke->getAliveParticlesCount() == 0)
		{
			ItemFirework::activeFireworks.erase(ItemFirework::activeFireworks.begin() + i);
			delete firework;
			continue;
		}
		i++;
	}
}

$hook(void, WorldManager, render, const m4::Mat5& MV, bool glasses, glm::vec3 worldColor)
{
	ItemFirework::sunlightColor = FX::utils::lerp(worldColor, glm::vec3{ 1 }, 0.7f);

	original(self, MV, glasses, worldColor);

	if (self->getType() == World::TYPE_TITLESCREEN) return;

	// i was too lazy to make an entity for this
	for (auto& firework : ItemFirework::activeFireworks)
	{
		if (!firework->exploded)
		{
			m4::Mat5 MVr = MV;
			MVr.translate(firework->pos);
			MVr *= m4::Rotor({ 0,1,0,0,0,0 }, glm::pi<float>() * 2.f * glfwGetTime()); // XZ 360 degrees per second
			MVr.scale(glm::vec4{ 1.5f });

			ItemFirework::renderModel(MVr, ItemFirework::sunlightColor, glm::normalize(glm::vec4{ 0.1f,1.f,0.25f,0.2f }));
		}

		firework->smoke->render(MV);
		if (firework->exploded)
		{
			firework->explosion->render(MV);
		}
	}
}

$hook(void, StateGame, updateProjection, GLsizei width, GLsizei height)
{
	original(self, width, height);

	FX::ParticleSystem::defaultShader->setUniform("P", self->projection3D);
	FX::TrailRenderer::defaultShader->setUniform("P", self->projection3D);
	((const FX::Shader*)ItemFirework::tetTexNormalShader)->setUniform("P", self->projection3D);
}

$hook(void, StateIntro, init, StateManager& s)
{
	original(self, s);

	// initialize opengl stuff
	glewExperimental = true;
	glewInit();
	glfwInit();

	glfwSetWindowTitle(s.window, "4D Miner by Mashpoe (new year edition)");

	ItemFirework::init();
}
