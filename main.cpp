#include <4dm.h>

using namespace fdm;

#include <fxlib/FXLib.h>

#include <glm/gtc/random.hpp>

#include "EntityFirework.h"
#include "ItemFirework.h"
#include "ItemSparkler.h"
#include "ItemConfetti.h"
#include "Config.h"

initDLL

bool loadingTiles = false;
$hook(bool, Tex2D, loadArrayTexture, const uint8_t* data, GLint texWidth, int texHeight, int channels, int cols, int rows)
{
	if (!data) return false;

	if (config.presentChests && loadingTiles)
	{
		int w, h, c;
		uint8_t* tex = SOIL_load_image(std::format("{}/assets/textures/present_chest.png", fdm::getModPath(fdm::modID)).c_str(), &w, &h, &c, channels);
		if (!tex) return original(self, data, texWidth, texHeight, channels, cols, rows);

		for (int x = 0; x < w; ++x)
		{
			for (int y = 0; y < h; ++y)
			{
				for (int i = 0; i < channels; ++i)
				{
					((uint8_t*)data)[(160 + x + y * texWidth) * channels + i] = tex[(x + y * w) * channels + i];
				}
			}
		}

		SOIL_free_image_data(tex);
	}

	return original(self, data, texWidth, texHeight, channels, cols, rows);
}

$hook(void, StateGame, init, StateManager& s)
{
	FX::applyPostProcessing(self->renderFramebuffer, [](uint32_t fbo, uint32_t colorTex, uint32_t depthTex, int width, int height, std::vector<FX::PostPassGroup>& passes)
		{
			if (config.bloomIntensity <= 0.0f) return;

			const Shader* brightFilterShader = FX::PostPass::loadPassShader("tr1ngledev.new-year-mod.brightFilter", std::format("{}/{}", fdm::getModPath(fdm::modID),
				"assets/shaders/brightFilter.frag"));
			const Shader* downsampleShader = FX::PostPass::loadPassShader("tr1ngledev.new-year-mod.downsample", std::format("{}/{}", fdm::getModPath(fdm::modID),
				"assets/shaders/downsample.frag"));
			const Shader* upsampleShader = FX::PostPass::loadPassShader("tr1ngledev.new-year-mod.upsample", std::format("{}/{}", fdm::getModPath(fdm::modID),
				"assets/shaders/upsample.frag"));
			const Shader* bloomShader = FX::PostPass::loadPassShader("tr1ngledev.new-year-mod.bloom", std::format("{}/{}", fdm::getModPath(fdm::modID),
				"assets/shaders/bloom.frag"));

			FX::PostPassGroup brightFilter{ };
			{
				brightFilter.clearColor = false;
				brightFilter.passes.emplace_back(brightFilterShader);
			}
			passes.emplace_back(brightFilter);

			constexpr int mipLevels = 6;
			FX::PostPassGroup downsample{ };
			{
				downsample.clearColor = false;
				int div = 1;
				for (int i = 0; i < mipLevels; ++i)
				{
					div *= 2;
					if (width / div < 1 || height / div < 1) break;
					FX::PostPass& pass = downsample.passes.emplace_back(downsampleShader, div);
					pass.passFormat = FX::PostPass::RGB;
				}
			}
			passes.emplace_back(downsample);

			FX::PostPassGroup upsample{ };
			{
				upsample.copyLastGroup = true;
				upsample.clearColor = false;
				upsample.iteration.dir = FX::PostPassGroup::PassIteration::BACKWARD;
				upsample.iteration.count = FX::PostPassGroup::PassIteration::SKIP_FIRST;
				upsample.viewportMode = FX::PostPassGroup::NEXT_PASS_SIZE;

				upsample.blending.mode = FX::PostPassGroup::Blending::ADD;
				upsample.blending.func.srcFactor = FX::PostPassGroup::Blending::Func::ONE;
				upsample.blending.func.dstFactor = FX::PostPassGroup::Blending::Func::ONE;

				int div = 1;
				for (int i = 0; i < mipLevels; ++i)
				{
					div *= 2;
					if (width / div < 1 || height / div < 1) break;
					FX::PostPass& pass = upsample.passes.emplace_back(upsampleShader, div);
					pass.passFormat = FX::PostPass::RGB;
				}
			}
			passes.emplace_back(upsample);

			FX::PostPassGroup bloom{ };
			{
				bloom.clearColor = false;
				bloom.uniforms.emplace_back(FX::Uniform::FLOAT, "intensity", &config.bloomIntensity);
				bloom.passes.emplace_back(bloomShader);
			}
			passes.emplace_back(bloom);
		});

	loadingTiles = true;
	int w, h, c;
	uint8_t* tex = SOIL_load_image("assets/textures/tiles.png", &w, &h, &c, 4);
	if (tex)
	{
		ResourceManager::textures["tiles.png"]->loadArrayTexture(tex, w, h, 4, 96, 16);
		SOIL_free_image_data(tex);
	}
	loadingTiles = false;

	original(self, s);
}

$hook(void, StateGame, update, StateManager& s, double dt)
{
	original(self, s, dt);

	if (config.nightMode)
	{
		self->time = 1000;
	}

	EntityFirework::worldColorAdd = FX::utils::ilerp(EntityFirework::worldColorAdd, glm::vec3{ 0 }, 0.02f, dt);
}

$hook(void, WorldManager, render, const m4::Mat5& MV, bool glasses, glm::vec3 worldColor)
{
	worldColor += EntityFirework::worldColorAdd;
	EntityFirework::lightColor = glm::mix(worldColor, glm::vec3{ 1 }, 0.6f);

	original(self, MV, glasses, worldColor);
}

$hook(void, StateIntro, init, StateManager& s)
{
	original(self, s);

	// initialize opengl stuff
	glewExperimental = true;
	glewInit();
	glfwInit();

	glfwSetWindowTitle(s.window, "4D Miner by Mashpoe (new year edition)");

	loadConfig();
	ItemFirework::renderInit();
	ItemSparkler::renderInit();
	EntityFirework::renderInit();
	EntityFirework::soundInit();
	ItemConfetti::renderInit();
	ItemConfetti::soundInit();
}

$hook(void, StateGame, updateProjection, int width, int height)
{
	original(self, width, height);

	FX::ParticleSystem::defaultShader->setUniform("P", self->projection3D);
	FX::TrailRenderer::defaultShader->setUniform("P", self->projection3D);
}

inline static int getY(gui::Element* element)
{
	// im comparing typeid name strings instead of using dynamic_cast because typeids of 4dminer and typeids of 4dm.h are different
	if (0 == strcmp(typeid(*element).name(), "class gui::Button"))
		return ((gui::Button*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class gui::CheckBox"))
		return ((gui::CheckBox*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class gui::Image"))
		return ((gui::Image*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class gui::Slider"))
		return ((gui::Slider*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class gui::Text"))
		return ((gui::Text*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class gui::TextInput"))
		return ((gui::TextInput*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Button"))
		return ((gui::Button*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::CheckBox"))
		return ((gui::CheckBox*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Image"))
		return ((gui::Image*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Slider"))
		return ((gui::Slider*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Text"))
		return ((gui::Text*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::TextInput"))
		return ((gui::TextInput*)element)->yOffset;
	return 0;
}

static bool initializedSettings = false;
$hook(void, StateSettings, init, StateManager& s)
{
	initializedSettings = false;
	original(self, s);
}

$hook(void, StateSettings, render, StateManager& s)
{
	original(self, s);
	if (initializedSettings) return;

	initializedSettings = true;
	int lowestY = 0;
	for (auto& e : self->mainContentBox.elements)
	{
		if (e == &self->secretButton) // skip the secret button
			continue;

		lowestY = std::max(getY(e), lowestY);
	}
	int oldLowest = lowestY;

	static gui::Text title{};
	title.setText("New-Year-Mod:");
	title.alignX(gui::ALIGN_CENTER_X);
	title.offsetY(lowestY += 100);
	title.size = 2;
	self->mainContentBox.addElement(&title);

	static gui::CheckBox nightMode{};
	nightMode.setText("Night-Mode");
	nightMode.alignX(gui::ALIGN_CENTER_X);
	nightMode.offsetY(lowestY += 50);
	nightMode.checked = config.nightMode;
	nightMode.callback = [](void* user, bool checked)
		{
			config.nightMode = checked;
		};
	self->mainContentBox.addElement(&nightMode);

	static gui::CheckBox presentChests{};
	presentChests.setText("Present Chests");
	presentChests.alignX(gui::ALIGN_CENTER_X);
	presentChests.offsetY(lowestY += 50);
	presentChests.checked = config.presentChests;
	presentChests.callback = [](void* user, bool checked)
		{
			config.presentChests = checked;
		};
	self->mainContentBox.addElement(&presentChests);

	static gui::Slider bloomIntensity{};
	bloomIntensity.setText(config.bloomIntensity <= 0.0f ? "Bloom Intensity: OFF" : std::format("Bloom Intensity: {}%", (int)(config.bloomIntensity * 100)));
	bloomIntensity.alignX(gui::ALIGN_CENTER_X);
	bloomIntensity.offsetY(lowestY += 50);
	bloomIntensity.value = config.bloomIntensity * 100;
	bloomIntensity.range = 200;
	bloomIntensity.width = 400;
	bloomIntensity.callback = [](void* user, int value)
		{
			config.bloomIntensity = value / 100.0f;
			bloomIntensity.setText(config.bloomIntensity <= 0.0f ? "Bloom Intensity: OFF" : std::format("Bloom Intensity: {}%", (int)(config.bloomIntensity * 100)));
		};
	self->mainContentBox.addElement(&bloomIntensity);

	static gui::Slider fireworkParticleCount{};
	fireworkParticleCount.setText(std::format("Firework Particle Count: {}", config.fireworkParticleCount));
	fireworkParticleCount.alignX(gui::ALIGN_CENTER_X);
	fireworkParticleCount.offsetY(lowestY += 50);
	fireworkParticleCount.value = config.fireworkParticleCount - 1000;
	fireworkParticleCount.range = 4000;
	fireworkParticleCount.width = 500;
	fireworkParticleCount.callback = [](void* user, int value)
		{
			config.fireworkParticleCount = value + 1000;
			fireworkParticleCount.setText(std::format("Firework Particle Count: {}", config.fireworkParticleCount));
		};
	self->mainContentBox.addElement(&fireworkParticleCount);
}

$hook(void, StateSettings, load, GLFWwindow* window)
{
	loadConfig();

	original(self, window);
}

$hook(void, StateSettings, save)
{
	saveConfig();

	original(self);
}
