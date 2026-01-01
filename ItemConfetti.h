#pragma once

#include <4dm.h>
#include <fxlib/FXLib.h>

class ItemConfetti : public fdm::Item
{
public:
	bool used = false;

	static const fdm::Tex2D* itemTexture;
	static const fdm::Tex2D* texture;
	static fdm::MeshRenderer renderer;
	static const fdm::Shader* shader;
	static fdm::stl::string confettiSound;
	inline static const char* voiceGroup = "ambience";

	static void renderInit();
	static void soundInit();
	static void playConfettiSound(const glm::vec4& position);

	bool action(fdm::World* world, fdm::Player* player, int action) override;
	bool isCompatible(const std::unique_ptr<fdm::Item>& other) override;
	fdm::stl::string getName() override;
	bool isDeadly() override;
	uint32_t getStackLimit() override;
	void render(const glm::ivec2& pos) override;
	void renderEntity(const fdm::m4::Mat5& MV, bool inHand, const glm::vec4& lightDir) override;
	std::unique_ptr<fdm::Item> clone() override;
	nlohmann::json saveAttributes() override;
};
