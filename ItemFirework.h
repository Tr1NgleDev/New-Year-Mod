#pragma once

#include <4dm.h>

using namespace fdm;

#include <fxlib/FXLib.h>

#include <glm/gtc/random.hpp>

struct Firework
{
	enum Type
	{
		EXPLOSION,
		FOUNTAIN, // i don't think i ever implemented this
		LAST = FOUNTAIN
	};

	glm::vec4 pos{ 0 };
	glm::vec4 vel{ 0 };
	glm::vec4 deviation{ 0 };
	double launchTime = 0;
	bool exploded = false;
	FX::ParticleSystem* explosion = nullptr;
	FX::ParticleSystem* smoke = nullptr;
	double lastSmokeEmission = 0;
	glm::vec4 startColor2, endColor2;
	Type type = EXPLOSION;

	Firework();
	~Firework();
};

class ItemFirework
{
public:
	static MeshBuilder fireworkBase;
	static MeshBuilder fireworkTop;
	static MeshBuilder particleMesh;
	static MeshRenderer baseRenderer;
	static MeshRenderer topRenderer;
	static const Shader* tetTexNormalShader;
	static const Shader* tetSolidColorNormalShader;
	static const Tex2D* fireworkTexture;
	static const Tex2D* fireworkItemTexture;
	static glm::vec3 sunlightColor;
	static std::vector<Firework*> activeFireworks;

	static stl::string the_fire_of_the_work_ogg;
	static stl::string explosion_ogg;

	static void renderModel(const m4::Mat5& MV, const glm::vec3& sunlightColor, const glm::vec4& lightDir);
	static void init();

	// this is probably incorrect but it works so idc
	inline static const glm::vec4 hexadecachoron_verts[8] = {
		glm::vec4(1,  0,  0,  0),
		glm::vec4(-1,  0,  0,  0),
		glm::vec4(0,  1,  0,  0),
		glm::vec4(0, -1,  0,  0),
		glm::vec4(0,  0,  1,  0),
		glm::vec4(0,  0, -1,  0),
		glm::vec4(0,  0,  0,  1),
		glm::vec4(0,  0,  0, -1)
	};
	inline static const glm::vec4 hexadecachoron_normals[4] = {
		glm::normalize(glm::vec4(1, -1, -1, -1)),
		glm::normalize(glm::vec4(-1, 1, -1, -1)),
		glm::normalize(glm::vec4(-1, -1, 1, -1)),
		glm::normalize(glm::vec4(-1, -1, -1, 1)),
	};
	inline static const int hexadecachoron_indices[64] = {
		// top half (32)
		0, 2, 4, 6,
		0, 2, 4, 7,
		0, 2, 5, 6,
		0, 2, 5, 7,

		1, 2, 4, 6,
		1, 2, 4, 7,
		1, 2, 5, 6,
		1, 2, 5, 7,

		// bottom half (32)
		0, 3, 4, 6,
		0, 3, 4, 7,
		0, 3, 5, 6,
		0, 3, 5, 7,

		1, 3, 4, 6,
		1, 3, 4, 7,
		1, 3, 5, 6,
		1, 3, 5, 7
	};

	static void fireworkEval(FX::ParticleSystem* ps, FX::ParticleSystem::Particle& p, FX::ParticleSystem::ParticleData& pData, size_t i, double dt);
	static void fireworkEmit(FX::ParticleSystem* ps, FX::ParticleSystem::Particle& p, FX::ParticleSystem::ParticleData& pData, size_t i);

	static void smokeEval(FX::ParticleSystem* ps, FX::ParticleSystem::Particle& p, FX::ParticleSystem::ParticleData& pData, size_t i, double dt);
	static void smokeEmit(FX::ParticleSystem* ps, FX::ParticleSystem::Particle& p, FX::ParticleSystem::ParticleData& pData, size_t i);
};
