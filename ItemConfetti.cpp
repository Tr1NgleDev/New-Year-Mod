#include "ItemConfetti.h"
#include "utils.h"
#include "EntityFirework.h"

using namespace fdm;

const Tex2D* ItemConfetti::itemTexture = nullptr;
const Tex2D* ItemConfetti::texture = nullptr;
MeshRenderer ItemConfetti::renderer;
const Shader* ItemConfetti::shader = nullptr;
stl::string ItemConfetti::confettiSound = "";

void ItemConfetti::renderInit()
{
	itemTexture = ResourceManager::get("assets/textures/confettiItem.png", true);
	std::string path = std::format("../../{}/assets/textures/confetti.png", getModPath(modID));
	ResourceManager::loadArrayTexture(path, 1, 8);
	texture = ResourceManager::get(path);
	shader = ShaderManager::get("tr1ngledev.new-year-mod.tetTexNormalShader");

	MeshBuilder hypercube{ BlockInfo::HYPERCUBE_FULL_INDEX_COUNT };
	hypercube.addBuff(BlockInfo::hypercube_full_verts, sizeof(BlockInfo::hypercube_full_verts));
	hypercube.addAttr(GL_UNSIGNED_BYTE, 4, sizeof(glm::u8vec4));
	hypercube.addBuff(BlockInfo::hypercube_full_tuvs, sizeof(BlockInfo::hypercube_full_tuvs));
	hypercube.addAttr(GL_UNSIGNED_BYTE, 3, sizeof(glm::u8vec3));
	hypercube.addBuff(BlockInfo::hypercube_full_normals, sizeof(BlockInfo::hypercube_full_normals));
	hypercube.addAttr(GL_FLOAT, 1, sizeof(GLfloat));
	hypercube.setIndexBuff(BlockInfo::hypercube_full_indices, sizeof(BlockInfo::hypercube_full_indices));

	renderer = &hypercube;
}
void ItemConfetti::soundInit()
{
	confettiSound = loadSound("assets/sfx/confetti.ogg");
}

void ItemConfetti::playConfettiSound(const glm::vec4& position)
{
	if (fdm::isServer()) return;
	glm::vec4 pos = position;
	correctSoundPos(pos);
	SoLoud::handle soundHandle
		= AudioManager::playSound4D(confettiSound, voiceGroup, pos, { 0,0,0,0 });
	AudioManager::soloud.setRelativePlaySpeed(soundHandle, glm::linearRand(0.95f, 1.125f));
}

bool ItemConfetti::action(World* world, Player* player, int action)
{
	if (!action) return false;
	if (used) return false;

	used = true;

	glm::vec4 pos = player->pos + player->forward * 0.6f + glm::vec4{ 0, 1.6f, 0, 0 };
	glm::vec4 vel = player->forward * 3.0f + player->up * 1.0f;
	auto particles = Entity::createWithAttributes("Confetti", pos,
		{
			{ "pVel", m4::vec4ToJson(vel) }
		});

	if (particles)
	{
		Chunk* chunk = world->getChunkFromCoords(pos.x, pos.z, pos.w);
		if (chunk)
		{
			world->addEntityToChunk(particles, chunk);
		}
	}

	return true;
}
bool ItemConfetti::isCompatible(const std::unique_ptr<Item>& other)
{
	auto* other_ = dynamic_cast<ItemConfetti*>(other.get());
	if (!other_) return false;
	return used == other_->used;
}
stl::string ItemConfetti::getName()
{
	return used ? "Empty Confetti Cannon" : "Confetti Cannon";
}
bool ItemConfetti::isDeadly()
{
	return false;
}
uint32_t ItemConfetti::getStackLimit()
{
	return used ? 4096 : 1;
}
void ItemConfetti::render(const glm::ivec2& pos)
{
	TexRenderer& tr = ItemTool::tr;
	const Tex2D* ogTex = tr.texture; // remember the original texture

	tr.texture = itemTexture; // set to custom texture
	tr.setClip(35 * used, 0, 35, 36);
	tr.setPos(pos.x, pos.y, 70, 72);
	tr.render();

	tr.texture = ogTex;
}
void ItemConfetti::renderEntity(const m4::Mat5& MV, bool inHand, const glm::vec4& lightDir)
{
	m4::Mat5 MVr = MV;
	MVr.scale(glm::vec4{ 4.0f });
	MVr.translate(glm::vec4{ 0, 0.3f, 0, 0 });
	MVr *= m4::Rotor{ m4::BiVector4{ 0, 1, 0, 0, 0, 0 }, glm::pi<float>() * 0.25f };
	MVr *= m4::Rotor{ m4::BiVector4{ 1, 0, 0, 0, 0, 0 }, glm::pi<float>() * 0.025f };

	glm::vec4 lightDirA = glm::normalize(MVr.multiply(lightDir, 0));

	const Shader* shader2 = ShaderManager::get("tetSolidColorNormalShader");

	shader->use();
	texture->use();
	((const FX::Shader*)shader)->setUniform("imageZSize", 8);
	((const FX::Shader*)shader)->setUniform("image", 0);
	((const FX::Shader*)shader)->setUniform("sunlightColor", EntityFirework::lightColor);
	((const FX::Shader*)shader)->setUniform("lightDir", lightDirA);
	((const FX::Shader*)shader2)->setUniform("lightDir", lightDirA);

	m4::Mat5 m = MVr;
	m.scale(glm::vec4{ 0.1f, 0.5f, 0.1f, 0.1f });
	m.translate(glm::vec4{ -0.50001f });
	((const FX::Shader*)shader)->setUniform("MV", m);
	renderer.render();

	shader2->use();

	m = MVr;
	m.translate(glm::vec4{ 0, 0.25f, 0, 0 });
	m.scale(glm::vec4{ 0.09f, 0.01f, 0.09f, 0.09f });
	m.translate(glm::vec4{ -0.50001f });
	((const FX::Shader*)shader2)->setUniform("MV", m);
	((const FX::Shader*)shader2)->setUniform("inColor", glm::vec4{ glm::vec3{ 1.0f, 1.0f, 1.0f } * EntityFirework::lightColor * (1.0f - used), 1.0f });
	renderer.render();

	m = MVr;
	m.translate(glm::vec4{ 0, -0.25f, 0, 0 });
	m.scale(glm::vec4{ 0.09f, 0.01f, 0.09f, 0.09f });
	m.translate(glm::vec4{ -0.50001f });
	((const FX::Shader*)shader2)->setUniform("MV", m);
	((const FX::Shader*)shader2)->setUniform("inColor", glm::vec4{ glm::vec3{ 0.7f, 0.09f, 0.26f } * EntityFirework::lightColor, 1.0f });
	renderer.render();
}
std::unique_ptr<Item> ItemConfetti::clone()
{
	auto result = std::make_unique<ItemConfetti>();
	result->used = used;
	return result;
}
nlohmann::json ItemConfetti::saveAttributes()
{
	return
	{
		{ "used", used }
	};
}

$hookStatic(bool, CraftingMenu, loadRecipes)
{
	bool result = original();

	addRecipe("Confetti Cannon", 1,
		{
			{{"name", "Stick"}, {"count", 1}},
			{{"name", "Iron Bars"}, {"count", 1}},
			{{"name", "Red Flower"}, {"count", 2}},
			{{"name", "Blue Flower"}, {"count", 2}},
			{{"name", "White Flower"}, {"count", 2}},
		});

	return result;
}

$hookStatic(bool, Item, loadItemInfo)
{
	bool result = original();

	static bool loaded = false;
	if (loaded) return result;
	loaded = true;

	Item::blueprints["Confetti Cannon"] =
	{
		{ "type", "confetti" },
		{ "baseAttributes", { { "used", false } } }
	};
	Item::blueprints["Empty Confetti Cannon"] =
	{
		{ "type", "confetti" },
		{ "baseAttributes", { { "used", true } } }
	};

	return result;
}

$hookStatic(std::unique_ptr<Item>, Item, instantiateItem, const stl::string& itemName, uint32_t count, const stl::string& type, const nlohmann::json& attributes)
{
	if (type != "confetti") return original(itemName, count, type, attributes);

	auto result = std::make_unique<ItemConfetti>();
	result->count = count;
	result->used = attributes["used"];
	return result;
}

$hook(void, StateGame, updateProjection, int width, int height)
{
	original(self, width, height);

	((const FX::Shader*)ItemConfetti::shader)->setUniform("P", self->projection3D);
}
