#include "EntityConfettiParticles.h"
#include "EntityFirework.h"
#include "ItemConfetti.h"
#include "utils.h"

using namespace fdm;

EntityConfettiParticles::~EntityConfettiParticles()
{
	Entity::~Entity();

	if (particles)
	{
		delete particles;
		particles = nullptr;
	}
}

stl::string EntityConfettiParticles::getName()
{
	return "Confetti";
}
void EntityConfettiParticles::update(World* world, double dt)
{
	if (particles)
	{
		particles->update(dt);
	}
	timer += dt;
	if (timer >= 10.0f)
		dead = true;
}

inline static const glm::vec3 confettiColors[]
{
	{ 1.0f, 0.2f, 0.2f },
	{ 0.2f, 1.0f, 0.5f },
	{ 0.2f, 0.5f, 1.0f },
	{ 1.0f, 0.2f, 1.0f },
	{ 1.0f, 0.2f, 0.5f },
	{ 1.0f, 1.0f, 0.2f },
	{ 1.0f, 0.5f, 0.2f },
};

void EntityConfettiParticles::render(const World* world, const m4::Mat5& MV, bool glasses)
{
	if (!particles)
	{
		particles = new FX::ParticleSystem
		(
			pos,
			glm::vec4{ 0, -2.5f, 0, 0 },
			glm::vec4{ 0.4f },
			{ 6.0f, 9.0f },
			pVel,
			{ glm::vec4{ -0.5f, 0, -0.5f, -0.5f }, glm::vec4{ 0.5f, 0, 0.5f, 0.5f } },
			{ glm::vec4{ 0.1f, 0.001f, 0.1f, 0.1f }, glm::vec4{ 0.2f, 0.001f, 0.2f, 0.2f } },
			glm::vec4{ 0.0f },
			m4::Rotor{ },
			m4::Rotor{ },
			false,
			glm::vec4{ 1.0f },
			glm::vec4{ 1.0f },
			FX::ParticleSystem::GLOBAL,
			500
		);
		particles->initRenderer(&EntityFirework::particleMesh, FX::ParticleSystem::defaultShader, FX::TrailRenderer::defaultShader);

		particles->emitFunc = [](FX::ParticleSystem* ps, FX::ParticleSystem::Particle& p, FX::ParticleSystem::ParticleData& pData, size_t i)
			{
				int colorIndex = FX::utils::random(0, 6);
				p.startColor = glm::vec4{ confettiColors[colorIndex], 1.0f };
				p.endColor = p.startColor;

				fdm::m4::BiVector4 plane
				{
					FX::utils::random(-1.0f, 1.0f),
					FX::utils::random(-1.0f, 1.0f),
					FX::utils::random(-1.0f, 1.0f),
					FX::utils::random(-1.0f, 1.0f),
					FX::utils::random(-1.0f, 1.0f),
					FX::utils::random(-1.0f, 1.0f)
				};
				plane.normalize();

				float angle = FX::utils::random(-glm::pi<float>(), glm::pi<float>());
				p.mat = fdm::m4::Rotor{ plane, angle };
				p.vel = ps->startVelocity.evalValue();
				p.vel += coneRandom(glm::normalize(p.vel), glm::pi<float>() * 0.25f) * FX::utils::random(1.0f, 3.0f);
			};

		particles->evalFunc = [](FX::ParticleSystem* ps, FX::ParticleSystem::Particle& p, FX::ParticleSystem::ParticleData& pData, size_t i, double dt)
			{
				p.vel += ps->gravity * (float)dt;
				p.vel += p.velDeviation * (float)dt * pData.t;
				p.vel += -ps->drag * p.vel * (float)dt;
				pData.scale = FX::utils::lerp(p.startScale, p.endScale, pData.t);
				pData.color = FX::utils::lerp(p.startColor, p.endColor, pData.t);

				fdm::m4::BiVector4 tumblePlane
				{
					glm::sin(p.time * 2.3f + i * 0.1f),
					glm::cos(p.time * 1.7f + i * 0.2f),
					glm::sin(p.time * 3.1f + i * 0.15f),
					glm::cos(p.time * 2.7f + i * 0.25f),
					glm::sin(p.time * 1.9f + i * 0.3f),
					glm::cos(p.time * 2.1f + i * 0.35f)
				};
				tumblePlane.normalize();

				float spinSpeed = 3.0f;
				fdm::m4::Rotor tumbleRotor{ tumblePlane, spinSpeed * (float)dt };
				p.mat = fdm::m4::Mat5{ tumbleRotor } * p.mat;
			};

		particles->emit(particles->getMaxParticles());
		ItemConfetti::playConfettiSound(pos);
	}

	particles->render(MV);
}
nlohmann::json EntityConfettiParticles::saveAttributes()
{
	return
	{
		{ "pos", m4::vec4ToJson(pos) },
		{ "pVel", m4::vec4ToJson(pVel) },
	};
}
nlohmann::json EntityConfettiParticles::getServerUpdateAttributes()
{
	nlohmann::json serverUpdate = saveAttributes();
	serverUpdate["dead"] = dead;
	return serverUpdate;
}
void EntityConfettiParticles::applyServerUpdate(const nlohmann::json& j, World* world)
{
	pos = m4::vec4FromJson<float>(j["pos"]);
	pVel = m4::vec4FromJson<float>(j["pVel"]);
	dead = j["dead"];
}
glm::vec4 EntityConfettiParticles::getPos()
{
	return pos;
}
void EntityConfettiParticles::setPos(const glm::vec4& pos)
{
	this->pos = pos;
}

$hookStatic(std::unique_ptr<Entity>, Entity, instantiateEntity, const stl::string& entityName, const stl::uuid& id, const glm::vec4& pos, const stl::string& type, const nlohmann::json& attributes)
{
	if (type == "confetti")
	{
		auto result = std::make_unique<EntityConfettiParticles>();

		result->id = id;
		result->setPos(pos);
		if (attributes.contains("pVel"))
			result->pVel = m4::vec4FromJson<float>(attributes["pVel"]);

		return result;
	}
	return original(entityName, id, pos, type, attributes);
}

$hookStatic(bool, Entity, loadEntityInfo)
{
	bool result = original();

	static bool loaded = false;
	if (loaded) return result;
	loaded = true;

	Entity::blueprints["Confetti"] =
	{
		{ "type", "confetti" },
		{ "baseAttributes", nlohmann::json::object() }
	};

	return result;
}
