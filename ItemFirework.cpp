#include "ItemFirework.h"
#include "EntityFirework.h"
#include "utils.h"

using namespace fdm;

const fdm::Tex2D* ItemFirework::texture = nullptr;

void ItemFirework::renderInit()
{
	texture = ResourceManager::get("assets/textures/fireworkItem.png", true);
}

bool ItemFirework::isCompatible(const std::unique_ptr<fdm::Item>& other)
{
	auto* other_ = dynamic_cast<ItemFirework*>(other.get());
	return other_;
}
fdm::stl::string ItemFirework::getName()
{
	return "Firework";
}
bool ItemFirework::isDeadly()
{
	return false;
}
uint32_t ItemFirework::getStackLimit()
{
	return ItemMaterial::STACK_MAX;
}
void ItemFirework::render(const glm::ivec2& pos)
{
	TexRenderer& tr = ItemTool::tr;
	const Tex2D* ogTex = tr.texture; // remember the original texture

	tr.texture = texture; // set to custom texture
	tr.setClip(0, 0, 35, 36);
	tr.setPos(pos.x, pos.y, 70, 72);
	tr.render();

	tr.texture = ogTex;
}
void ItemFirework::renderEntity(const fdm::m4::Mat5& MV, bool inHand, const glm::vec4& lightDir)
{
	m4::Mat5 m = MV;
	m.scale(glm::vec4{ 2 });
	EntityFirework::renderModel(m, EntityFirework::lightColor, lightDir);
}
std::unique_ptr<fdm::Item> ItemFirework::clone()
{
	auto result = std::make_unique<ItemFirework>();
	return result;
}
nlohmann::json ItemFirework::saveAttributes()
{
	return nlohmann::json::object();
}
inline static const glm::vec3 colors[8]
{
	glm::vec3{ 255, 103, 92 } / 255.f * 1.5f,
	glm::vec3{ 207, 117, 255 } / 255.f * 1.5f,
	glm::vec3{ 138, 255, 177 } / 255.f * 1.5f,
	glm::vec3{ 227, 132, 175 } / 255.f * 1.5f,
	glm::vec3{ 240, 224, 134 } / 255.f * 1.5f,
	glm::vec3{ 114, 232, 220 } / 255.f * 1.5f,
	glm::vec3{ 255, 255, 255 } / 255.f * 1.5f,
	glm::vec3{ 24, 155, 204 } / 255.f * 1.5f // weezer blue
};
bool ItemFirework::action(fdm::World* world, fdm::Player* player, int action)
{
	if (!action) return false;
	if (!player->targetingBlock) return false;

	double time = glfwGetTime();

	if (count > 0 && time - lastTimeUsed > 0.5f)
	{
		lastTimeUsed = time;

		player->updateCameraPos(world);
		glm::vec4 pos = player->cameraPos;
		glm::ivec4 posBlock = pos;
		glm::vec4 end = pos + player->forward * 16.0f;
		glm::ivec4 endBlock = end;
		world->castRay(pos, posBlock, endBlock, end);

		pos += glm::vec4{ 0, 0.3f, 0, 0 };

		auto firework = Entity::createWithAttributes("Firework", pos,
			{
				{ "type", FX::utils::random(0, EntityFirework::Type::LAST) },
				{ "startColor", m4::vec4ToJson(glm::vec4{ colors[glm::linearRand(0, 8 - 1)], 1 }) },
				{ "endColor", m4::vec4ToJson(glm::vec4{ colors[glm::linearRand(0, 8 - 1)], 0.5f }) },
				{ "startColor2", m4::vec4ToJson(glm::vec4{ colors[glm::linearRand(0, 8 - 1)], 1 }) },
				{ "endColor2", m4::vec4ToJson(glm::vec4{ colors[glm::linearRand(0, 8 - 1)], 0.5f }) },
			});

		if (firework)
		{
			Chunk* chunk = world->getChunkFromCoords(pos.x, pos.z, pos.w);
			if (chunk)
			{
				EntityFirework* fireworkE = (EntityFirework*)firework.get();
				if (world->addEntityToChunk(firework, chunk))
				{
					if (player->crouching)
					{

					}
					else
					{
						fireworkE->launch();
					}
				}
			}
		}

		--count;

		return true;
	}

	return false;
}

$hookStatic(bool, Item, loadItemInfo)
{
	bool result = original();

	static bool loaded = false;
	if (loaded) return result;
	loaded = true;

	Item::blueprints["Firework"] =
	{
		{ "type", "firework" },
		{ "baseAttributes", nlohmann::json::object() }
	};

	return result;
}

$hookStatic(bool, CraftingMenu, loadRecipes)
{
	bool result = original();

	addRecipe("Firework", 16,
		{
			{{"name", "Deadly Bars"}, {"count", 1}},
			{{"name", "Red Flower"}, {"count", 1}},
			{{"name", "Blue Flower"}, {"count", 1}},
			{{"name", "White Flower"}, {"count", 1}}
		});

	return result;
}

$hookStatic(std::unique_ptr<Item>, Item, instantiateItem, const stl::string& itemName, uint32_t count, const stl::string& type, const nlohmann::json& attributes)
{
	if (type != "firework") return original(itemName, count, type, attributes);

	auto result = std::make_unique<ItemFirework>();
	result->count = count;
	return result;
}
