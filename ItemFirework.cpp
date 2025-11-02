#include "ItemFirework.h"

MeshBuilder ItemFirework::fireworkBase{};
MeshBuilder ItemFirework::fireworkTop{};
MeshBuilder ItemFirework::particleMesh{};
MeshRenderer ItemFirework::baseRenderer{};
MeshRenderer ItemFirework::topRenderer{};
const Shader* ItemFirework::tetTexNormalShader = nullptr;
const Shader* ItemFirework::tetSolidColorNormalShader = nullptr;
const Tex2D* ItemFirework::fireworkTexture = nullptr;
const Tex2D* ItemFirework::fireworkItemTexture = nullptr;
glm::vec3 ItemFirework::sunlightColor{ 1 };
std::vector<Firework*> ItemFirework::activeFireworks{};

stl::string ItemFirework::the_fire_of_the_work_ogg = "";
stl::string ItemFirework::explosion_ogg = "";

void launchFirework(double time, const glm::vec4& pos)
{
	Firework* firework = new Firework();
	firework->launchTime = time + FX::utils::random(-0.2f, 0.2f);
	firework->pos = pos;
	firework->vel = glm::vec4{ 0, glm::linearRand(50, 80), 0 ,0 };
	firework->deviation = glm::normalize(FX::utils::random(glm::vec4{ -1, 0, -1, -1 }, glm::vec4{ 1, 0, 1, 1 })) * 2.f;
	ItemFirework::activeFireworks.push_back(firework);
	SoLoud::handle soundHandle
		= AudioManager::playSound4D(ItemFirework::the_fire_of_the_work_ogg, "ambience", firework->pos, firework->vel);
	AudioManager::soloud.setRelativePlaySpeed(soundHandle, glm::linearRand(0.9f, 1.1f));
}

$hook(bool, ItemMaterial, action, World* world, Player* player, int action)
{
	original(self, world, player, action);

	if (self->name != "Firework") return false; // if not a firework, return.
	if (!action) return false; // if not right click, return.
	if (!player->targetingBlock) return false; // if not targeting a block, return.

	static double lastTimeUsed = 0;

	double time = glfwGetTime();

	if (self->count > 0)
	{
		lastTimeUsed = time;

		launchFirework(time, player->reachEndpoint);

		--self->count;

		return true;
	}

	return false;
}

$hook(void, ItemMaterial, renderEntity, const m4::Mat5& mat, bool inHand, const glm::vec4& lightDir)
{
	if (self->name != "Firework")
		return original(self, mat, inHand, lightDir);

	m4::Mat5 m = mat;
	m.scale(glm::vec4{ 2 });
	ItemFirework::renderModel(m, ItemFirework::sunlightColor, lightDir);
}

$hook(void, ItemMaterial, render, const glm::ivec2& pos)
{
	if (self->name != "Firework")
		return original(self, pos);

	TexRenderer& tr = ItemTool::tr;
	const Tex2D* ogTex = tr.texture;
	tr.texture = ItemFirework::fireworkItemTexture;
	tr.setClip(0, 0, 35, 36);
	tr.setPos(pos.x, pos.y, 70, 72);
	tr.render();
	tr.texture = ogTex;
}

void ItemFirework::renderModel(const m4::Mat5& MV, const glm::vec3& sunlightColor, const glm::vec4& lightDir)
{
	glm::vec4 lightDirA = MV.multiply(lightDir, 0);
	m4::Mat5 topRot = m4::Rotor({ 0,0,1,0,0,1 }, glm::pi<float>() * 0.25f);

	m4::Mat5 MVbase = MV;
	MVbase.scale(glm::vec4{ 0.225f, 0.4f, 0.225f, 0.225f });
	MVbase.translate(glm::vec4{ -0.5f });

	m4::Mat5 MVtop = MV;
	MVtop.translate({ 0.f, 0.175f, 0.f, 0.f });
	MVtop *= m4::Rotor({ 0,0,1,0,0,1 }, glm::pi<float>() * 0.25f); // XW plane 45 degrees
	MVtop.scale({ 0.35f, 0.45f, 0.35f, 0.35f });

	tetTexNormalShader->use();

	glBindTexture(GL_TEXTURE_2D_ARRAY, fireworkTexture->ID);
	glActiveTexture(GL_TEXTURE0);

	glUniform4fv(glGetUniformLocation(tetTexNormalShader->id(), "lightDir"), 1, &lightDirA.x);
	glUniform1i(glGetUniformLocation(tetTexNormalShader->id(), "imageZSize"), 8);
	glUniform1i(glGetUniformLocation(tetTexNormalShader->id(), "image"), 0);
	glUniform3fv(glGetUniformLocation(tetTexNormalShader->id(), "sunlightColor"), 1, &sunlightColor.x);
	glUniform1fv(glGetUniformLocation(tetTexNormalShader->id(), "MV"), sizeof(m4::Mat5) / sizeof(float), &MVbase[0][0]);

	baseRenderer.render(); // hypercube base

	tetSolidColorNormalShader->use();

	glm::vec4 color = glm::vec4(glm::vec3(230.f / 255.f, 60.f / 255.f, 60.f / 255.f) * sunlightColor, 1.f);
	glUniform4fv(glGetUniformLocation(tetSolidColorNormalShader->id(), "lightDir"), 1, &lightDirA.x);
	glUniform4fv(glGetUniformLocation(tetSolidColorNormalShader->id(), "inColor"), 1, &color.x);
	glUniform1fv(glGetUniformLocation(tetSolidColorNormalShader->id(), "MV"), sizeof(m4::Mat5) / sizeof(float), &MVtop[0][0]);

	topRenderer.render(); // 16-cell half top

	m4::Mat5 MVtop2 = MVtop;
	MVtop2.scale({ 1.0f, 0.125f, 1.0f, 1.0f }); // squish
	color.x *= 0.5f;
	color.y *= 0.5f;
	color.z *= 0.5f;
	glm::vec4 lightDirN = lightDirA * -1.f;
	glUniform4fv(glGetUniformLocation(tetSolidColorNormalShader->id(), "lightDir"), 1, &lightDirN.x);
	glUniform4fv(glGetUniformLocation(tetSolidColorNormalShader->id(), "inColor"), 1, &color.x);
	glUniform1fv(glGetUniformLocation(tetSolidColorNormalShader->id(), "MV"), sizeof(m4::Mat5) / sizeof(float), &MVtop2[0][0]);
	topRenderer.render(); // 16-cell half bottom of the top
}

void ItemFirework::init()
{
	ItemFirework::tetTexNormalShader = ShaderManager::load("tetTexNormalShader",
		"../../assets/shaders/tetNormal.vs",
		"assets/shaders/tetTexNormal.fs",
		"../../assets/shaders/tetNormal.gs");

	std::string fireworkPath = std::format("../../{}/assets/textures/firework.png", getModPath(modID));
	ResourceManager::loadArrayTexture(fireworkPath, 1, 8);
	ResourceManager::textures["firework.png"] = ResourceManager::textures[fireworkPath];
	ResourceManager::textures.erase(fireworkPath);
	ItemFirework::fireworkTexture = ResourceManager::get("firework.png");

	fireworkItemTexture = ResourceManager::get("assets/textures/fireworkItem.png", true);

	ItemFirework::fireworkBase = MeshBuilder{ BlockInfo::HYPERCUBE_FULL_INDEX_COUNT };
	ItemFirework::fireworkBase.addBuff(BlockInfo::hypercube_full_verts, sizeof(BlockInfo::hypercube_full_verts));
	ItemFirework::fireworkBase.addAttr(GL_UNSIGNED_BYTE, 4, sizeof(glm::u8vec4));
	ItemFirework::fireworkBase.addBuff(BlockInfo::hypercube_full_tuvs, sizeof(BlockInfo::hypercube_full_tuvs));
	ItemFirework::fireworkBase.addAttr(GL_UNSIGNED_BYTE, 3, sizeof(glm::u8vec3));
	ItemFirework::fireworkBase.addBuff(BlockInfo::hypercube_full_normals, sizeof(BlockInfo::hypercube_full_normals));
	ItemFirework::fireworkBase.addAttr(GL_FLOAT, 1, sizeof(GLfloat));
	ItemFirework::fireworkBase.setIndexBuff(BlockInfo::hypercube_full_indices, sizeof(BlockInfo::hypercube_full_indices));

	ItemFirework::fireworkTop = MeshBuilder{ 64 / 2 };
	ItemFirework::fireworkTop.addBuff(&hexadecachoron_verts, sizeof(hexadecachoron_verts));
	ItemFirework::fireworkTop.addAttr(GL_FLOAT, 4, sizeof(glm::vec4));
	ItemFirework::fireworkTop.addBuff(&hexadecachoron_normals, sizeof(hexadecachoron_normals));
	ItemFirework::fireworkTop.addAttr(GL_FLOAT, 1, sizeof(GLfloat));
	ItemFirework::fireworkTop.setIndexBuff(&hexadecachoron_indices, sizeof(hexadecachoron_indices));

	particleMesh = MeshBuilder{ BlockInfo::HYPERCUBE_FULL_INDEX_COUNT };
	particleMesh.addBuff(BlockInfo::hypercube_full_verts, sizeof(BlockInfo::hypercube_full_verts));
	particleMesh.addAttr(GL_UNSIGNED_BYTE, 4, sizeof(glm::u8vec4));
	particleMesh.setIndexBuff(BlockInfo::hypercube_full_indices, sizeof(BlockInfo::hypercube_full_indices));

	ItemFirework::baseRenderer.setMesh(&ItemFirework::fireworkBase);
	ItemFirework::topRenderer.setMesh(&ItemFirework::fireworkTop);

	ItemFirework::tetSolidColorNormalShader = ShaderManager::get("tetSolidColorNormalShader");

	// add the firework item (ItemFirework is technically an ItemMaterial because i don't want to make a custom class because why would i)
	Item::blueprints["Firework"] =
	{
		{ "type", "material" },
		{ "baseAttributes", nlohmann::json::object() }
	};

	ItemFirework::activeFireworks.reserve(10);

	the_fire_of_the_work_ogg = std::format("../../{}/assets/sfx/the fire of the work.ogg", getModPath(modID));
	explosion_ogg = std::format("../../{}/assets/sfx/explosion.ogg", getModPath(modID));

	AudioManager::loadSound(the_fire_of_the_work_ogg);
	AudioManager::loadSound(explosion_ogg);
}

inline constexpr void addRecipe(const fdm::stl::string& resultName, int resultCount, const nlohmann::json& recipe)
{
	assert(recipe.is_array());
	if (!std::any_of(fdm::CraftingMenu::recipes.begin(), fdm::CraftingMenu::recipes.end(), [&](auto& x)
		{ return x["recipe"] == recipe && x["result"]["name"] == (std::string)resultName && x["result"]["count"] == resultCount; }))
		fdm::CraftingMenu::recipes.push_back(
			nlohmann::json{
			{"recipe", recipe},
			{"result", {{"name", resultName}, {"count", resultCount}}}
			}
		);
}

$hookStatic(bool, CraftingMenu, loadRecipes)
{
	bool result = original();

	addRecipe("Firework", 16,
		{
			{{"name", "Stick"}, {"count", 1}},
			{{"name", "Red Flower"}, {"count", 1}},
			{{"name", "Blue Flower"}, {"count", 1}},
			{{"name", "White Flower"}, {"count", 1}}
		});

	return result;
}

inline static glm::vec3 colors[8]
{
	glm::vec3{ 255, 103, 92 } / 255.f,
	glm::vec3{ 207, 117, 255 } / 255.f,
	glm::vec3{ 138, 255, 177 } / 255.f,
	glm::vec3{ 227, 132, 175 } / 255.f,
	glm::vec3{ 240, 224, 134 } / 255.f,
	glm::vec3{ 114, 232, 220 } / 255.f,
	glm::vec3{ 255, 255, 255 } / 255.f,
	glm::vec3{ 24, 155, 204 } / 255.f // weezer blue
};

Firework::Firework()
{
	explosion = new FX::ParticleSystem(glm::vec4{ 0 }, { 1.f, 2.f }, FX::ParticleSystem::GLOBAL, 2500);
	
	explosion->initRenderer(&ItemFirework::particleMesh, FX::ParticleSystem::defaultShader, FX::TrailRenderer::defaultShader);
	explosion->gravity = { 0, World::gravity * 1.25f, 0, 0 };
	explosion->user = this;
	explosion->startColor = glm::vec4(colors[glm::linearRand(0, 8 - 1)], 1);
	explosion->endColor = glm::vec4(colors[glm::linearRand(0, 8 - 1)], 0.5f);
	this->startColor2 = glm::vec4(colors[glm::linearRand(0, 8 - 1)], 1);
	this->endColor2 = glm::vec4(colors[glm::linearRand(0, 8 - 1)], 0.5f);
	explosion->startScale = { glm::vec4{2.5f, 2.5f, 3.f, 2.5f}, glm::vec4{2.f, 2.f, 2.75f, 2.f} };
	explosion->angleTowardsVelocity = true;
	explosion->endScale = { glm::vec4{0.1f}, glm::vec4{0.5f} };
	explosion->evalFunc = ItemFirework::fireworkEval;
	explosion->emitFunc = ItemFirework::fireworkEmit;
	explosion->trails = true;
	explosion->setTrailBillboard(true);
	explosion->setTrailLifetime(1.5f);
	explosion->setMinTrailPointDist(8.f);
	explosion->setTrailColorFunc([](float t, float p, size_t trailID, void* user)
		{
			Firework* f = (Firework*)user;
			return FX::utils::lerp(f->startColor2, f->endColor2, 1.0f - p * (1.f - t));
		});
	explosion->setTrailWidthFunc([](float t, float p, size_t trailID, void* user)
		{
			Firework* f = (Firework*)user;
			return (2.1f * p * (1.f - t) + 0.1f) * (1.0 - (glfwGetTime() - f->explosion->lastEmitTime) / 4.f);
		});
	explosion->trailRenderer.user = this;

	smoke = new FX::ParticleSystem(glm::vec4{ 0 }, { 2.f, 3.f }, FX::ParticleSystem::GLOBAL, 500);
	smoke->initRenderer(&ItemFirework::particleMesh, FX::ParticleSystem::defaultShader, FX::TrailRenderer::defaultShader);
	smoke->gravity = { 0, World::gravity * 0.05f, 0, 0 };
	smoke->user = this;
	smoke->evalFunc = ItemFirework::smokeEval;
	smoke->emitFunc = ItemFirework::smokeEmit;

	type = (Type)FX::utils::random((int)EXPLOSION, (int)LAST);
}

Firework::~Firework()
{
	if (explosion)
		delete explosion;
	if (smoke)
		delete smoke;

	explosion = nullptr;
	smoke = nullptr;
}

float easeInOutQuart(float t)
{
	t = glm::clamp(t, 0.0f, 1.0f);
	return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - glm::pow(-2.0f * t + 2.0f, 4.0f) * 0.5f;
}

void ItemFirework::fireworkEval(FX::ParticleSystem* ps, FX::ParticleSystem::Particle& p, FX::ParticleSystem::ParticleData& pData, size_t i, double dt)
{
	p.vel -= p.vel * (i <= 2000 ? 0.95f : 0.8f) * (float)dt * (pData.t + 1.0f);
	p.vel += p.velDeviation * (float)dt * pData.t;
	p.vel += ps->gravity * (float)dt;
	pData.scale = FX::utils::lerp(p.startScale, p.endScale, easeInOutQuart(pData.t));
	pData.color = FX::utils::lerp(p.startColor, p.endColor, easeInOutQuart(pData.t));
}

void ItemFirework::fireworkEmit(FX::ParticleSystem* ps, FX::ParticleSystem::Particle& p, FX::ParticleSystem::ParticleData& pData, size_t i)
{
	p.vel = glm::normalize(glm::linearRand(glm::vec4{ -1 }, glm::vec4{ 1 })) * FX::utils::random(80.f, 120.f) * (i > 2000 ? 0.4f : 1.f);

	p.velDeviation = FX::utils::random(glm::vec4{ -5 }, glm::vec4{ 5 });

	if (i > 2000)
	{
		pData.color = p.startColor = ((Firework*)ps->user)->startColor2;
		p.endColor = ((Firework*)ps->user)->endColor2;
		pData.scale = p.startScale *= 0.4f;
		p.endScale = glm::vec4{ 0.f };
	}
}

void ItemFirework::smokeEval(FX::ParticleSystem* ps, FX::ParticleSystem::Particle& p, FX::ParticleSystem::ParticleData& pData, size_t i, double dt)
{
	p.vel = FX::utils::ilerp(p.vel, glm::vec4{ 0 }, 0.01f, dt); // drag(?)
	p.pos += ps->gravity * ((float)dt * p.time * 0.5f); // false gravity
	pData.scale = FX::utils::lerp(p.startScale, p.endScale, easeInOutQuart(pData.t)) * 0.5f;
	pData.color = FX::utils::lerp(p.startColor, p.endColor, easeInOutQuart(pData.t));
}

void ItemFirework::smokeEmit(FX::ParticleSystem* ps, FX::ParticleSystem::Particle& p, FX::ParticleSystem::ParticleData& pData, size_t i)
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
	p.endColor = pData.color * glm::vec4(1, 1, 1, 0);
	pData.scale = p.startScale;
}


$hook(void, StateGame, addChatMessage, Player* player, const stl::string& message, uint32_t color)
{
	if (message.find("/new_year_real") != stl::string::npos)
	{
		for (int i = 0; i < 3; i++)
		{
			launchFirework(glfwGetTime() + i * 0.1f + glm::linearRand(-0.7f, 0.7f), {3.5f + i * 15, 32.f, 51.5f, 1.5f + i * 2});
		}

		for (auto& e : self->world->entities.entities) 
		{
			if (e.second->getName() == "Player")
			{
				launchFirework(glfwGetTime() + glm::linearRand(-0.7f, 0.7f), e.second->getPos() + glm::vec4{0,2,0,0});
			}
		}
	}
	original(self, player, message, color);
}
