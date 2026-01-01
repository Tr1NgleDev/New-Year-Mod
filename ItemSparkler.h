#pragma once

#include <4dm.h>

#include <fxlib/FXLib.h>

#include "TickingItem.h"

class ItemSparkler : public TickingItem
{
private:
	double lastEmission = 0.0;

public:
	float progress = 0.0f;
	bool burning = false;
	FX::ParticleSystem* particles;

	static const fdm::Tex2D* texture;
	static fdm::MeshRenderer renderer;
	static const fdm::Shader* shader;
	static const fdm::Shader* handleShader;

	static void renderInit();

	~ItemSparkler() override;

	void update(fdm::World* world, double dt) override;
	bool action(fdm::World* world, fdm::Player* player, int action) override;
	void postAction(fdm::World* world, fdm::Player* player, int action) override;
	bool isCompatible(const std::unique_ptr<fdm::Item>& other) override;
	fdm::stl::string getName() override;
	bool isDeadly() override;
	uint32_t getStackLimit() override;
	void render(const glm::ivec2& pos) override;
	void renderEntity(const fdm::m4::Mat5& MV, bool inHand, const glm::vec4& lightDir) override;
	std::unique_ptr<fdm::Item> clone() override;
	nlohmann::json saveAttributes() override;
	void initParticles();
};
