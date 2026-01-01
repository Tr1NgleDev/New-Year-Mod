#include "Config.h"
#include <fstream>

Config config{};

void loadConfig()
{
	std::ifstream configFile{ std::filesystem::path(fdm::getModPath(fdm::modID)) / "config.json" };
	if (!configFile.is_open())
	{
		saveConfig();
		return;
	}

	nlohmann::json j = nlohmann::json::parse(configFile);

	if (j.contains("nightMode") && j["nightMode"].is_boolean())
		config.nightMode = j["nightMode"];
	else
		config.nightMode = true;

	if (j.contains("bloomIntensity") && j["bloomIntensity"].is_number_float())
		config.bloomIntensity = glm::max(j["bloomIntensity"].get<float>(), 0.0f);
	else
		config.bloomIntensity = 1.0f;

	if (j.contains("fireworkParticleCount") && j["fireworkParticleCount"].is_number_integer())
		config.fireworkParticleCount = glm::clamp(j["fireworkParticleCount"].get<int>(), 1000, 5000);
	else
		config.fireworkParticleCount = 2000;

	if (j.contains("presentChests") && j["presentChests"].is_boolean())
		config.presentChests = j["presentChests"];
	else
		config.presentChests = true;

	configFile.close();

	saveConfig();
}

void saveConfig()
{
	std::ofstream configFile{ std::filesystem::path(fdm::getModPath(fdm::modID)) / "config.json" };
	if (!configFile.is_open()) return;

	nlohmann::json j
	{
		{ "nightMode", config.nightMode },
		{ "bloomIntensity", config.bloomIntensity },
		{ "fireworkParticleCount", config.fireworkParticleCount },
		{ "presentChests", config.presentChests },
	};
	configFile << j.dump(4);

	configFile.close();
}
