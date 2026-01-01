#include "ItemSparkler.h"
#include "EntityFirework.h"
#include "utils.h"

using namespace fdm;

const Tex2D* ItemSparkler::texture = nullptr;
MeshRenderer ItemSparkler::renderer;
const Shader* ItemSparkler::shader = nullptr;
const Shader* ItemSparkler::handleShader = nullptr;

void ItemSparkler::renderInit()
{
	texture = ResourceManager::get("assets/textures/sparklerItem.png", true);
	shader = ShaderManager::load("tr1ngledev.new-year-mod.sparklerShader",
		"assets/shaders/sparkler.vert",
		"assets/shaders/sparkler.frag",
		"assets/shaders/sparkler.geom");
	handleShader = ShaderManager::get("tetSolidColorNormalShader");

	MeshBuilder hypercube{ BlockInfo::HYPERCUBE_FULL_INDEX_COUNT };
	hypercube.addBuff(BlockInfo::hypercube_full_verts, sizeof(BlockInfo::hypercube_full_verts));
	hypercube.addAttr(GL_UNSIGNED_BYTE, 4, sizeof(glm::u8vec4));
	hypercube.addBuff(BlockInfo::hypercube_full_normals, sizeof(BlockInfo::hypercube_full_normals));
	hypercube.addAttr(GL_FLOAT, 1, sizeof(GLfloat));
	hypercube.setIndexBuff(BlockInfo::hypercube_full_indices, sizeof(BlockInfo::hypercube_full_indices));
	renderer = &hypercube;
}

bool ItemSparkler::isCompatible(const std::unique_ptr<Item>& other)
{
	auto* other_ = dynamic_cast<ItemSparkler*>(other.get());
	if (!other_) return false;
	bool burnt = !burning && progress >= 1.0f;
	bool burntOther = !other_->burning && other_->progress >= 1.0f;
	return burnt == burntOther;
}
stl::string ItemSparkler::getName()
{
	bool burnt = !burning && progress >= 1.0f;
	return burnt ? "Burnt Sparkler" : "Sparkler";
}
bool ItemSparkler::isDeadly()
{
	return false;
}
uint32_t ItemSparkler::getStackLimit()
{
	bool burnt = !burning && progress >= 1.0f;
	return burnt ? 4096 : 1;
}
void ItemSparkler::render(const glm::ivec2& pos)
{
	bool burnt = !burning && progress >= 1.0f;

	TexRenderer& tr = ItemTool::tr;
	const Tex2D* ogTex = tr.texture; // remember the original texture

	tr.texture = texture; // set to custom texture
	tr.setClip(35 * burnt, 0, 35, 36);
	tr.setPos(pos.x, pos.y, 70, 72);
	tr.render();

	tr.texture = ogTex;
}
void ItemSparkler::update(World* world, double dt)
{
	if (!fdm::isServer())
	{
		initParticles();
		particles->update(dt);
	}
	if (burning)
	{
		progress += dt / 60.0f;

		double time = glfwGetTime();
		if (!fdm::isServer() && time - lastEmission >= 0.01)
		{
			particles->emit(5);
			lastEmission = time;
		}

		if (progress >= 1.0f)
		{
			progress = 1.0f;
			burning = false;
		}
	}
}
void ItemSparkler::renderEntity(const m4::Mat5& MV, bool inHand, const glm::vec4& lightDir)
{
	glm::vec4 lightDirA = glm::normalize(MV.multiply(lightDir, 0));

	constexpr float width = 0.1f;
	constexpr float widthH = 0.05f;
	constexpr float height = 1.3f;
	constexpr float heightH = 0.5f;
	float offsetY = (heightH + height) * 0.5f + (!inHand ? 0.2f : 0.0f);
	constexpr float halfHeight = height * 0.5f;

	m4::Mat5 off = MV;
	off.translate(glm::vec4{ 0, offsetY, 0, 0 });

	m4::Mat5 m = off;
	m.scale(glm::vec4{ width, height, width, width });
	m.translate(glm::vec4{ -0.50001f });

	m4::Mat5 mH = off;
	mH.translate(glm::vec4{ 0, height * -0.5f, 0, 0 });
	mH.translate(glm::vec4{ 0, heightH * -0.5f, 0, 0 });
	mH.scale(glm::vec4{ widthH, heightH, widthH, widthH });
	mH.translate(glm::vec4{ -0.50001f });

	shader->use();
	((const FX::Shader*)shader)->setUniform("MV", m);
	((const FX::Shader*)shader)->setUniform("progress", progress);
	((const FX::Shader*)shader)->setUniform("lightDir", lightDirA);
	renderer.render();

	handleShader->use();
	((const FX::Shader*)handleShader)->setUniform("MV", mH);
	((const FX::Shader*)handleShader)->setUniform("inColor", 0.45f, 0.45f, 0.45f, 1.0f);
	((const FX::Shader*)handleShader)->setUniform("lightDir", lightDirA);
	renderer.render();

	initParticles();
	if (particles)
	{
		m4::Mat5 mP = off;
		mP.translate(glm::vec4{ 0, glm::mix(halfHeight, -halfHeight, progress), 0, 0 });
		particles->render(mP);
	}
}
std::unique_ptr<Item> ItemSparkler::clone()
{
	auto result = std::make_unique<ItemSparkler>();
	result->burning = burning;
	result->progress = progress;
	return result;
}
nlohmann::json ItemSparkler::saveAttributes()
{
	return
	{
		{ "burning", burning },
		{ "progress", progress },
	};
}

void ItemSparkler::initParticles()
{
	if (fdm::isServer()) return;

	if (!particles)
	{
		particles = new FX::ParticleSystem
		(
			{ 0, 0, 0, 0 },												// origin
			{ 0, -8, 0, 0 },											// gravity
			{ 0.5f, 0.5f, 0.5f, 0.5f },									// drag
			{ 0.3f, 0.8f },												// lifetime
			{ glm::vec4{ -3, -3, -3, -3 }, glm::vec4{ 3, 3, 3, 3 } },	// startVelocity
			glm::vec4{ 0 },												// velocityDeviation
			{ glm::vec4{ 0.04f }, glm::vec4{ 0.12f } },					// startScale
			{ glm::vec4{ 0.0f }, glm::vec4{ 0.02f } },					// endScale
			m4::Rotor{},												// startRot
			m4::Rotor{},												// endRot
			false,														// angleTowardsVelocity
			glm::vec4{ 1.0f, 0.9f, 0.5f, 1.0f } * 1.5f,					// startColor
			glm::vec4{ 1.0f, 0.5f, 0.1f, 0.0f } * 1.1f,					// endColor
			FX::ParticleSystem::LOCAL,									// particleSpace
			500															// maxParticles
		);
		particles->initRenderer(&EntityFirework::particleMesh, FX::ParticleSystem::defaultShader, FX::TrailRenderer::defaultShader);

		particles->trails = true;
		particles->trailRenderer.tesseractal = true;
		particles->setTrailBillboard(true);
		particles->setTrailLifetime(0.3f);
		particles->setMinTrailPointDist(0.05f);

		particles->setTrailColorFunc([](float t, float p, size_t id, void* user)
			{
				auto& pData = ((FX::ParticleSystem*)user)->getParticleData()[id];
				return pData.color * glm::vec4(1, 1, 1, p * (1.f - t));
			});

		particles->setTrailWidthFunc([](float t, float p, size_t id, void* user)
			{
				auto& pData = ((FX::ParticleSystem*)user)->getParticleData()[id];
				return pData.scale.x * p * (1.f - t);
			});

		particles->spawnMode = FX::ParticleSystem::SPHERE;
		particles->spawnSphereParams.radius = 0.1f;
		particles->spawnSphereParams.force = 5.0f;
	}
}

bool ItemSparkler::action(World* world, Player* player, int action)
{
	if (!action) return false;
	if (burning) return false;
	if (progress >= 0.99f) return false;

	burning = true;
	progress = 0.0f;

	return true;
}

void ItemSparkler::postAction(World* world, Player* player, int action)
{
	this->action(world, player, action);
}

ItemSparkler::~ItemSparkler()
{
	Item::~Item();

	if (particles)
	{
		delete particles;
		particles = nullptr;
	}
}

$hookStatic(bool, Item, loadItemInfo)
{
	bool result = original();

	static bool loaded = false;
	if (loaded) return result;
	loaded = true;

	Item::blueprints["Sparkler"] =
	{
		{ "type", "sparkler" },
		{ "baseAttributes", { { "burning", false }, { "progress", 0.0f } } }
	};
	Item::blueprints["Burnt Sparkler"] =
	{
		{ "type", "sparkler" },
		{ "baseAttributes", { { "burning", false }, { "progress", 1.0f } } }
	};

	return result;
}

$hookStatic(bool, CraftingMenu, loadRecipes)
{
	bool result = original();

	addRecipe("Sparkler", 1,
		{
			{{"name", "Stick"}, {"count", 1}},
			{{"name", "Deadly Bars"}, {"count", 1}},
			{{"name", "Iron Bars"}, {"count", 1}}
		});

	return result;
}

$hookStatic(std::unique_ptr<Item>, Item, instantiateItem, const stl::string& itemName, uint32_t count, const stl::string& type, const nlohmann::json& attributes)
{
	if (type != "sparkler") return original(itemName, count, type, attributes);

	auto result = std::make_unique<ItemSparkler>();
	result->count = count;
	result->burning = attributes["burning"];
	result->progress = attributes["progress"];
	return result;
}

$hook(void, StateGame, updateProjection, int width, int height)
{
	original(self, width, height);

	((const FX::Shader*)ItemSparkler::shader)->setUniform("P", self->projection3D);
	((const FX::Shader*)ItemSparkler::handleShader)->setUniform("P", self->projection3D);
}
