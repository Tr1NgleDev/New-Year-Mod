#pragma once

#include <4dm.h>

#include <fxlib/FXLib.h>

class EntityFirework : public fdm::Entity
{
public:
	enum Type : int
	{
		EXPLOSION,
		FOUNTAIN,
		GLOME,
		LAST = GLOME
	};

	fdm::Hitbox hitbox{};
	glm::vec4 deviation{ 0 };
	double totalLifetime = 0;
	double lifetime = 0;
	double explosionTime = 3.0f;
	bool launched = false;
	bool exploded = false;
	FX::ParticleSystem* explosion = nullptr;
	FX::ParticleSystem* smoke = nullptr;
	glm::vec4 startColor{}, endColor{};
	glm::vec4 startColor2{}, endColor2{};
	Type type = EXPLOSION;

	static fdm::stl::string launchSound;
	static fdm::stl::string explosionSound;
	inline static const char* voiceGroup = "ambience";

	static fdm::MeshRenderer fireworkBase;
	static fdm::MeshRenderer fireworkTop;
	static fdm::MeshBuilder particleMesh;
	static const fdm::Shader* tetTexNormalShader;
	static const fdm::Shader* tetSolidColorVertNormalShader;
	static const fdm::Tex2D* fireworkTexture;
	static glm::vec3 lightColor;
	static glm::vec3 worldColorAdd;
	static void soundInit();
	static void renderInit();
	static void playLaunchSound(const glm::vec4& position, const glm::vec4& velocity);
	static void playExplosionSound(const glm::vec4& position);

	~EntityFirework() override;

	fdm::stl::string getName() override;
	void update(fdm::World* world, double dt) override;
	void render(const fdm::World* world, const fdm::m4::Mat5& MV, bool glasses) override;
	nlohmann::json saveAttributes() override;
	nlohmann::json getServerUpdateAttributes() override;
	void applyServerUpdate(const nlohmann::json& j, fdm::World* world) override;
	glm::vec4 getPos() override;
	void setPos(const glm::vec4& pos) override;
	bool action(fdm::World* world, fdm::Entity* actor, int action, const nlohmann::json& details) override;
	bool isIntersectingRay(const Entity::Ray& ray) override;
	bool shouldSave() override { return true; }
	bool isClickable() override { return true; }
	void setup();
	void launch();

	static void fireworkEval(FX::ParticleSystem* ps, FX::ParticleSystem::Particle& p, FX::ParticleSystem::ParticleData& pData, size_t i, double dt);
	static void fireworkEmit(FX::ParticleSystem* ps, FX::ParticleSystem::Particle& p, FX::ParticleSystem::ParticleData& pData, size_t i);

	static void smokeEval(FX::ParticleSystem* ps, FX::ParticleSystem::Particle& p, FX::ParticleSystem::ParticleData& pData, size_t i, double dt);
	static void smokeEmit(FX::ParticleSystem* ps, FX::ParticleSystem::Particle& p, FX::ParticleSystem::ParticleData& pData, size_t i);
	static void renderModel(const fdm::m4::Mat5& MV, const glm::vec3& sunlightColor, const glm::vec4& lightDir);
};
