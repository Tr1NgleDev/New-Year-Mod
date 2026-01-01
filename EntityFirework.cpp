#include "EntityFirework.h"
#include "hexadecachoron.h"
#include "Config.h"
#include "utils.h"

using namespace fdm;

stl::string EntityFirework::launchSound = "";
stl::string EntityFirework::explosionSound = "";

MeshRenderer EntityFirework::fireworkBase;
MeshRenderer EntityFirework::fireworkTop;
MeshBuilder EntityFirework::particleMesh
{
	BlockInfo::HYPERCUBE_FULL_INDEX_COUNT,
	{
		{
			BlockInfo::hypercube_full_verts, sizeof(BlockInfo::hypercube_full_verts),
			{
				{ GL_UNSIGNED_BYTE, 4, sizeof(glm::u8vec4) }
			}
		}
	},
	BlockInfo::hypercube_full_indices, sizeof(BlockInfo::hypercube_full_indices)
};
const Shader* EntityFirework::tetTexNormalShader = nullptr;
const Shader* EntityFirework::tetSolidColorVertNormalShader = nullptr;
glm::vec3 EntityFirework::lightColor{ 1 };
glm::vec3 EntityFirework::worldColorAdd{ 0 };
const fdm::Tex2D* EntityFirework::fireworkTexture = nullptr;

void EntityFirework::soundInit()
{
	launchSound = loadSound("assets/sfx/launch.ogg");
	explosionSound = loadSound("assets/sfx/explosion.ogg");
}
void EntityFirework::renderInit()
{
	tetTexNormalShader = ShaderManager::load("tr1ngledev.new-year-mod.tetTexNormalShader",
		"../../assets/shaders/tetNormal.vs",
		"assets/shaders/tetTexNormal.frag",
		"../../assets/shaders/tetNormal.gs");
	tetSolidColorVertNormalShader = ShaderManager::load("tr1ngledev.new-year-mod.tetSolidColorVertNormalShader",
		"assets/shaders/tetSolidColorVertNormal.vert",
		"assets/shaders/tetSolidColorVertNormal.frag",
		"assets/shaders/tetSolidColorVertNormal.geom");

	MeshBuilder fireworkBase{ BlockInfo::HYPERCUBE_FULL_INDEX_COUNT };
	fireworkBase.addBuff(BlockInfo::hypercube_full_verts, sizeof(BlockInfo::hypercube_full_verts));
	fireworkBase.addAttr(GL_UNSIGNED_BYTE, 4, sizeof(glm::u8vec4));
	fireworkBase.addBuff(BlockInfo::hypercube_full_tuvs, sizeof(BlockInfo::hypercube_full_tuvs));
	fireworkBase.addAttr(GL_UNSIGNED_BYTE, 3, sizeof(glm::u8vec3));
	fireworkBase.addBuff(BlockInfo::hypercube_full_normals, sizeof(BlockInfo::hypercube_full_normals));
	fireworkBase.addAttr(GL_FLOAT, 1, sizeof(GLfloat));
	fireworkBase.setIndexBuff(BlockInfo::hypercube_full_indices, sizeof(BlockInfo::hypercube_full_indices));

	MeshBuilder fireworkTop{ 64 / 2 };
	fireworkTop.addBuff(&hexadecachoron_verts, sizeof(hexadecachoron_verts));
	fireworkTop.addAttr(GL_FLOAT, 4, sizeof(glm::vec4));
	fireworkTop.addBuff(&hexadecachoron_normals, sizeof(hexadecachoron_normals));
	fireworkTop.addAttr(GL_FLOAT, 4, sizeof(glm::vec4));

	EntityFirework::fireworkBase = &fireworkBase;
	EntityFirework::fireworkTop = &fireworkTop;

	std::string fireworkPath = std::format("../../{}/assets/textures/firework.png", getModPath(modID));
	ResourceManager::loadArrayTexture(fireworkPath, 1, 8);
	fireworkTexture = ResourceManager::get(fireworkPath);

	particleMesh = MeshBuilder{ BlockInfo::HYPERCUBE_FULL_INDEX_COUNT };
	particleMesh.addBuff(BlockInfo::hypercube_full_verts, sizeof(BlockInfo::hypercube_full_verts));
	particleMesh.addAttr(GL_UNSIGNED_BYTE, 4, sizeof(glm::u8vec4));
	particleMesh.setIndexBuff(BlockInfo::hypercube_full_indices, sizeof(BlockInfo::hypercube_full_indices));
}
void EntityFirework::playLaunchSound(const glm::vec4& position, const glm::vec4& velocity)
{
	if (fdm::isServer()) return;
	glm::vec4 pos = position;
	correctSoundPos(pos);
	SoLoud::handle soundHandle
		= AudioManager::playSound4D(launchSound, voiceGroup, pos, velocity);
	AudioManager::soloud.setRelativePlaySpeed(soundHandle, glm::linearRand(0.9f, 1.1f));
}
void EntityFirework::playExplosionSound(const glm::vec4& position)
{
	if (fdm::isServer()) return;
	glm::vec4 pos = position;
	correctSoundPos(pos);
	SoLoud::handle soundHandle
		= AudioManager::playSound4D(explosionSound, voiceGroup, pos, { 0,0,0,0 });
	AudioManager::soloud.setRelativePlaySpeed(soundHandle, glm::linearRand(0.9f, 1.1f));
}

stl::string EntityFirework::getName()
{
	return "Firework";
}
void EntityFirework::update(World* world, double dt)
{
	bool isServer = world->getType() == World::TYPE_SERVER;

	glm::vec4 lastPos = hitbox.pos;
	if (!exploded && launched)
	{
		hitbox.deltaVel = deviation * ((float)(lifetime) * 0.75f + 0.2f) * (float)dt;
		hitbox.update(world, dt);
	}

	if (!isServer)
	{
		explosion->update(dt);
		smoke->update(dt);
		smoke->origin = glm::mix(lastPos, hitbox.pos, 0.5f) - glm::vec4{ 0, 0.5f, 0, 0 };
		explosion->origin = hitbox.pos;
	}

	if (!exploded && launched)
	{
		if (lifetime < explosionTime)
		{
			if (!isServer)
			{
				smoke->emit(2);
				smoke->origin = hitbox.pos - glm::vec4{ 0, 0.5f, 0, 0 };
				smoke->emit(2);
			}
		}
		else
		{
			if (!isServer)
			{
				explosion->emit(explosion->getMaxParticles());
				playExplosionSound(hitbox.pos);

				worldColorAdd += glm::vec3{ startColor.r, startColor.g, startColor.b } * 0.1f;
			}

			exploded = true;
			hitbox.vel = glm::vec4{ 0 };
			hitbox.deltaVel = glm::vec4{ 0 };
		}
	}

	if (exploded && lifetime - explosionTime > 4.0f)
	{
		dead = true;
	}

	if (launched)
	{
		lifetime += dt;
	}

	totalLifetime += dt;

	if (!launched && totalLifetime > 5.0f)
	{
		launch();
	}
}
void EntityFirework::render(const World* world, const m4::Mat5& MV, bool glasses)
{
	if (!exploded)
	{
		m4::Mat5 MVr = MV;
		MVr.translate(hitbox.pos);
		if (launched)
		{
			MVr *= m4::Rotor({ 0,1,0,0,0,0 }, glm::pi<float>() * 2.f * lifetime); // XZ 360 degrees per second
		}
		MVr.scale(glm::vec4{ 1.5f });

		renderModel(MVr, lightColor, glm::normalize(glm::vec4{ 0.1f,1.f,0.25f,0.2f }));
	}

	if (launched)
	{
		smoke->render(MV);
		if (exploded)
		{
			explosion->render(MV);
		}
	}
}
nlohmann::json EntityFirework::saveAttributes()
{
	return
	{
		{ "pos", m4::vec4ToJson(hitbox.pos) },
		{ "vel", m4::vec4ToJson(hitbox.vel) },
		{ "deviation", m4::vec4ToJson(deviation) },
		{ "type", (int)type },
		{ "startColor", m4::vec4ToJson(startColor) },
		{ "endColor", m4::vec4ToJson(endColor) },
		{ "startColor2", m4::vec4ToJson(startColor2) },
		{ "endColor2", m4::vec4ToJson(endColor2) },
		{ "lifetime", lifetime },
		{ "totalLifetime", totalLifetime },
		{ "launched", launched },
		{ "explosionTime", explosionTime },
		{ "exploded", exploded },
	};
}
nlohmann::json EntityFirework::getServerUpdateAttributes()
{
	return saveAttributes();
}
void EntityFirework::applyServerUpdate(const nlohmann::json& j, World* world)
{
	hitbox.pos = m4::vec4FromJson<float>(j["pos"]);
	hitbox.vel = m4::vec4FromJson<float>(j["vel"]);
	deviation = m4::vec4FromJson<float>(j["deviation"]);
	type = j["type"];
	startColor = m4::vec4FromJson<float>(j["startColor"]);
	endColor = m4::vec4FromJson<float>(j["endColor"]);
	startColor2 = m4::vec4FromJson<float>(j["startColor2"]);
	endColor2 = m4::vec4FromJson<float>(j["endColor2"]);
	lifetime = j["lifetime"];
	totalLifetime = j["totalLifetime"];
	if (!launched && j["launched"])
	{
		playLaunchSound(hitbox.pos, hitbox.vel);
	}
	launched = j["launched"];
	explosionTime = j["explosionTime"];
	if (!exploded && j["exploded"])
	{
		explosion->emit(explosion->getMaxParticles());
		playExplosionSound(hitbox.pos);
	}
	exploded = j["exploded"];
}
glm::vec4 EntityFirework::getPos()
{
	return hitbox.pos;
}
void EntityFirework::setPos(const glm::vec4& pos)
{
	hitbox.pos = pos;
}
void EntityFirework::launch()
{
	if (launched) return;

	setup();
	
	lifetime = 0.0f;
	launched = true;
	exploded = false;
	explosionTime = 3.0f + FX::utils::random(-0.5f, 1.0f);
	hitbox.vel = glm::vec4{ 0, glm::linearRand(50, 80), 0 ,0 };
	deviation = glm::normalize(FX::utils::random(glm::vec4{ -1, 0, -1, -1 }, glm::vec4{ 1, 0, 1, 1 })) * 2.f;
	hitbox.pos += glm::vec4{ 0, 0.5f, 0, 0 };
	playLaunchSound(hitbox.pos, hitbox.vel);
}
bool EntityFirework::action(World* world, Entity* actor, int action, const nlohmann::json& details)
{
	if (launched) return false;
	if (totalLifetime <= 0.5) return false;
	
	launch();

	return true;
}

bool EntityFirework::isIntersectingRay(const Entity::Ray& ray)
{
	Hitbox hitbox{};
	hitbox.pos = this->hitbox.pos;
	hitbox.halfDimensions = glm::vec4{ 0.5f * 0.5f };
	return hitbox.isIntersectingRay(ray);
}

EntityFirework::~EntityFirework()
{
	Entity::~Entity();
	if (explosion)
	{
		delete explosion;
		explosion = nullptr;
	}
	if (smoke)
	{
		delete smoke;
		smoke = nullptr;
	}
}

float easeInOutQuart(float t)
{
	t = glm::clamp(t, 0.0f, 1.0f);
	return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - glm::pow(-2.0f * t + 2.0f, 4.0f) * 0.5f;
}

void EntityFirework::fireworkEval(FX::ParticleSystem* ps, FX::ParticleSystem::Particle& p, FX::ParticleSystem::ParticleData& pData, size_t i, double dt)
{
	EntityFirework* firework = (EntityFirework*)ps->user;

	const int max = ps->getMaxParticles();
	const int halfMax = max / 2;
	if (firework->type == EntityFirework::EXPLOSION)
		p.vel -= p.vel * (i <= halfMax ? 0.95f : 0.8f) * (float)dt * (pData.t + 1.0f);
	else
		p.vel -= p.vel * 0.95f * (float)dt * (pData.t + 1.0f);

	p.vel += p.velDeviation * (float)dt * pData.t;
	p.vel += ps->gravity * (float)dt;
	pData.scale = FX::utils::lerp(p.startScale, p.endScale, easeInOutQuart(pData.t));
	pData.color = FX::utils::lerp(p.startColor, p.endColor, easeInOutQuart(pData.t));
}
void EntityFirework::fireworkEmit(FX::ParticleSystem* ps, FX::ParticleSystem::Particle& p, FX::ParticleSystem::ParticleData& pData, size_t i)
{
	EntityFirework* firework = (EntityFirework*)ps->user;

	const int max = ps->getMaxParticles();
	const int halfMax = max / 2;

	switch (firework->type)
	{
	case EntityFirework::EXPLOSION:
		p.vel = glm::normalize(glm::linearRand(glm::vec4{ -1 }, glm::vec4{ 1 })) * FX::utils::random(80.f, 120.f) * (i > halfMax ? 0.4f : 1.f);
		p.velDeviation = FX::utils::random(glm::vec4{ -5 }, glm::vec4{ 5 });
		break;
	case EntityFirework::FOUNTAIN:
		p.vel = glm::normalize(glm::linearRand(glm::vec4{ -0.25f, 2.0f, -0.25f, -0.25f }, glm::vec4{ 0.25f, 2.0f, 0.25f, 0.25f })) * FX::utils::random(120.f, 160.f) * (i > halfMax ? 0.4f : 1.f);
		p.velDeviation = FX::utils::random(glm::vec4{ -10 }, glm::vec4{ 10 });
		break;
	case EntityFirework::GLOME:
		p.vel = pointOnGlome(i) * 100.0f;
		p.velDeviation = glm::vec4{ 0 };
		break;
	default:
		break;
	}

	if (i > halfMax)
	{
		pData.color = p.startColor = firework->startColor2;
		p.endColor = firework->endColor2;
		pData.scale = p.startScale *= 0.4f;
		p.endScale = glm::vec4{ 0.f };
	}
}

void EntityFirework::smokeEval(FX::ParticleSystem* ps, FX::ParticleSystem::Particle& p, FX::ParticleSystem::ParticleData& pData, size_t i, double dt)
{
	p.vel = FX::utils::ilerp(p.vel, glm::vec4{ 0 }, 0.01f, dt); // drag(?)
	p.pos += ps->gravity * ((float)dt * p.time * 0.5f); // false gravity
	pData.scale = FX::utils::lerp(p.startScale, p.endScale, easeInOutQuart(pData.t)) * 0.5f;
	pData.color = FX::utils::lerp(p.startColor, p.endColor, easeInOutQuart(pData.t));
}
void EntityFirework::smokeEmit(FX::ParticleSystem* ps, FX::ParticleSystem::Particle& p, FX::ParticleSystem::ParticleData& pData, size_t i)
{
	p.vel = glm::normalize(glm::linearRand(glm::vec4{ -0.2f, 1, -0.2f, -0.2f }, glm::vec4{ 0.2f, 1, 0.2f, 0.2f })) * 0.2f;

	// random rotation
	m4::Rotor rot = m4::Rotor(
		m4::wedge(
			glm::normalize(glm::linearRand(glm::vec4{ -1 }, glm::vec4{ 1 })),
			glm::normalize(glm::linearRand(glm::vec4{ -1 }, glm::vec4{ 1 }))
		),
		glm::linearRand(-glm::pi<float>(), glm::pi<float>())
	);
	rot.normalize();
	p.mat *= rot;

	if (glm::linearRand(0.f, 1.f) > 0.7f) // fire
	{
		pData.color = glm::vec4{ 255, 207, 94, 220 } / 255.f;

		p.startScale = glm::vec4{ 0.6f };
		p.endScale = glm::vec4{ 0.0f };
		p.lifetime -= 1.9f;
	}
	else if (glm::linearRand(0.f, 1.f) > 0.5f) // white
	{
		pData.color = glm::vec4{ 255, 255, 255, 100 } / 255.f;

		p.startScale = glm::vec4{ 0.55f };
		p.endScale = glm::vec4{ 0.1f };
		p.lifetime -= 0.5f;
	}
	else // smoke
	{
		pData.color = glm::vec4{ 20, 20, 20, 200 } / 255.f;

		p.startScale = glm::vec4{ 0.5f };
		p.endScale = glm::vec4{ 0.2f };
	}
	p.startColor = pData.color;
	p.endColor = pData.color * glm::vec4{ 1, 1, 1, 0 };
	pData.scale = p.startScale;
}

void EntityFirework::renderModel(const m4::Mat5& MV, const glm::vec3& sunlightColor, const glm::vec4& lightDir)
{
	glm::vec4 lightDirA = glm::normalize(MV.multiply(lightDir, 0));
	const m4::Mat5 topRot = m4::Rotor({ 0,0,1,0,0,1 }, glm::pi<float>() * 0.25f);

	m4::Mat5 MVbase = MV;
	MVbase.scale({ 0.225f, 0.4f, 0.225f, 0.225f });
	MVbase.translate(glm::vec4{ -0.5001f });

	m4::Mat5 MVtop = MV;
	MVtop.translate({ 0.001f, 0.1751f, 0.001f, 0.001f });
	MVtop *= topRot;
	MVtop.scale({ 0.35f, 0.45f, 0.35f, 0.35f });

	tetTexNormalShader->use();

	fireworkTexture->use();

	((const FX::Shader*)tetTexNormalShader)->setUniform("lightDir", lightDirA);
	((const FX::Shader*)tetTexNormalShader)->setUniform("imageZSize", 8);
	((const FX::Shader*)tetTexNormalShader)->setUniform("image", 0);
	((const FX::Shader*)tetTexNormalShader)->setUniform("sunlightColor", sunlightColor);
	((const FX::Shader*)tetTexNormalShader)->setUniform("MV", MVbase);

	fireworkBase.render(); // hypercube base

	tetSolidColorVertNormalShader->use();

	glm::vec4 color = glm::vec4{ glm::vec3{ 255.0f / 255.0f, 100.0f / 255.0f, 100.0f / 255.0f } * glm::mix(sunlightColor, glm::vec3{ 1 }, 0.7f), 1.0f };
	((const FX::Shader*)tetSolidColorVertNormalShader)->setUniform("lightDir", lightDirA);
	((const FX::Shader*)tetSolidColorVertNormalShader)->setUniform("inColor", color);
	((const FX::Shader*)tetSolidColorVertNormalShader)->setUniform("MV", MVtop);

	fireworkTop.render(); // 16-cell half top

	m4::Mat5 MVtop2 = MVtop;
	MVtop2.scale({ 1.0f, 0.125f, 1.0f, 1.0f }); // squish
	color.x *= 0.5f;
	color.y *= 0.5f;
	color.z *= 0.5f;
	glm::vec4 lightDirN = lightDirA * -1.f;
	((const FX::Shader*)tetSolidColorVertNormalShader)->setUniform("lightDir", lightDirN);
	((const FX::Shader*)tetSolidColorVertNormalShader)->setUniform("inColor", color);
	((const FX::Shader*)tetSolidColorVertNormalShader)->setUniform("MV", MVtop2);
	fireworkTop.render(); // 16-cell half bottom of the top
}
void EntityFirework::setup()
{
	hitbox.userPtr = this;
	hitbox.halfDimensions = glm::vec4{ 0.2f * 0.5f };
	hitbox.collisionCallback = [](void* user, Hitbox* hitbox, World* world, const glm::ivec4& collisionBlock, int collisionComp, const glm::vec4& prevVel)
		{
			EntityFirework* firework = (EntityFirework*)user;
			if (collisionComp == 1)
			{
				firework->explosionTime = firework->lifetime;
			}
		};

	if (fdm::isServer()) return;

	if (explosion) delete explosion;
	explosion = new FX::ParticleSystem(glm::vec4{ 0 }, { 1.f, 2.f }, FX::ParticleSystem::GLOBAL, config.fireworkParticleCount);

	explosion->initRenderer(&particleMesh, FX::ParticleSystem::defaultShader, FX::TrailRenderer::defaultShader);
	switch (type)
	{
	case EntityFirework::EXPLOSION:
	case EntityFirework::FOUNTAIN:
		explosion->gravity = { 0, World::gravity * 1.5f, 0, 0 };
		break;
	case EntityFirework::GLOME:
		explosion->gravity = { 0, World::gravity * 0.2f, 0, 0 };
		break;
	}
	explosion->user = this;
	explosion->startColor = startColor;
	explosion->endColor = endColor;
	explosion->startScale = { glm::vec4{ 2.5f, 2.5f, 3.4f, 2.5f }, glm::vec4{ 1.5f, 1.5f, 2.5f, 1.5f } };
	explosion->angleTowardsVelocity = true;
	explosion->endScale = { glm::vec4{ 0.05f }, glm::vec4{ 0.2f } };
	explosion->evalFunc = fireworkEval;
	explosion->emitFunc = fireworkEmit;
	explosion->trails = true;
	explosion->setTrailBillboard(true);
	explosion->setTrailLifetime(1.5f);
	explosion->setMinTrailPointDist(8.0f);
	explosion->setTrailColorFunc([](float t, float p, size_t trailID, void* user)
		{
			EntityFirework* f = (EntityFirework*)user;
			return FX::utils::lerp(f->startColor2, f->endColor2, 1.0f - p * (1.f - t));
		});
	explosion->setTrailWidthFunc([](float t, float p, size_t trailID, void* user)
		{
			EntityFirework* f = (EntityFirework*)user;
			return (2.1f * p * (1.f - t) + 0.1f) * (1.0 - (glfwGetTime() - f->explosion->lastEmitTime) / 4.f);
		});
	explosion->trailRenderer.user = this;

	if (smoke) delete smoke;
	smoke = new FX::ParticleSystem(glm::vec4{ 0 }, { 2.f, 3.f }, FX::ParticleSystem::GLOBAL, 500);
	smoke->initRenderer(&particleMesh, FX::ParticleSystem::defaultShader, FX::TrailRenderer::defaultShader);
	smoke->gravity = { 0, World::gravity * 0.05f, 0, 0 };
	smoke->user = this;
	smoke->evalFunc = smokeEval;
	smoke->emitFunc = smokeEmit;
}

$hookStatic(std::unique_ptr<Entity>, Entity, instantiateEntity, const stl::string& entityName, const stl::uuid& id, const glm::vec4& pos, const stl::string& type, const nlohmann::json& attributes)
{
	if (type == "firework")
	{
		auto result = std::make_unique<EntityFirework>();

		result->id = id;
		result->setPos(pos);
		result->hitbox.vel = m4::vec4FromJson<float>(attributes["vel"]);
		result->deviation = m4::vec4FromJson<float>(attributes["deviation"]);
		result->lifetime = attributes["lifetime"];
		result->totalLifetime = attributes["totalLifetime"];
		result->launched = attributes["launched"];
		result->exploded = attributes["exploded"];
		if (attributes.contains("explosionTime"))
			result->explosionTime = attributes["explosionTime"];
		if (attributes.contains("type"))
			result->type = attributes["type"];
		if (attributes.contains("startColor"))
			result->startColor = m4::vec4FromJson<float>(attributes["startColor"]);
		if (attributes.contains("endColor"))
			result->endColor = m4::vec4FromJson<float>(attributes["endColor"]);
		if (attributes.contains("startColor2"))
			result->startColor2 = m4::vec4FromJson<float>(attributes["startColor2"]);
		if (attributes.contains("endColor2"))
			result->endColor2 = m4::vec4FromJson<float>(attributes["endColor2"]);
		
		result->setup();

		return result;
	}
	return original(entityName, id, pos, type, attributes);
}

$hook(void, StateGame, updateProjection, int width, int height)
{
	original(self, width, height);

	((const FX::Shader*)EntityFirework::tetSolidColorVertNormalShader)->setUniform("P", self->projection3D);
	((const FX::Shader*)EntityFirework::tetTexNormalShader)->setUniform("P", self->projection3D);
}

$hookStatic(bool, Entity, loadEntityInfo)
{
	bool result = original();

	static bool loaded = false;
	if (loaded) return result;
	loaded = true;

	Entity::blueprints["Firework"] =
	{
		{ "type", "firework" },
		{ "baseAttributes",
			{
				{ "vel", std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f } },
				{ "deviation", std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f } },
				{ "lifetime", 0.0f },
				{ "totalLifetime", 0.0f },
				{ "launched", false },
				{ "exploded", false },
			}
		}
	};

	return result;
}
