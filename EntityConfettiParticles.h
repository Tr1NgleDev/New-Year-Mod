#pragma once

#include <4dm.h>
#include <fxlib/FXLib.h>

class EntityConfettiParticles : public fdm::Entity
{
public:
	double timer = 0.0;
	glm::vec4 pos;
	glm::vec4 pVel;
	FX::ParticleSystem* particles;

	static void renderInit();
	~EntityConfettiParticles() override;

	fdm::stl::string getName() override;
	void update(fdm::World* world, double dt) override;
	void render(const fdm::World* world, const fdm::m4::Mat5& MV, bool glasses) override;
	nlohmann::json saveAttributes() override;
	nlohmann::json getServerUpdateAttributes() override;
	void applyServerUpdate(const nlohmann::json& j, fdm::World* world) override;
	glm::vec4 getPos() override;
	void setPos(const glm::vec4& pos) override;
};
