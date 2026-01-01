#pragma once

#include <4dm.h>

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

inline static double radicalInverse(uint64_t index, uint32_t base)
{
	double invBase = 1.0 / (double)base;
	double result = 0.0;
	double f = invBase;
	while (index > 0)
	{
		uint32_t digit = index % base;
		result += (double)digit * f;
		index /= base;
		f *= invBase;
	}
	return result;
}

inline glm::vec4 pointOnGlome(size_t index, double radius = 1.0)
{
	uint64_t idx = (uint64_t)index + 1;
	double u1 = radicalInverse(idx, 2);
	double u2 = radicalInverse(idx, 3);
	double u3 = radicalInverse(idx, 5);

	const double TWO_PI = 6.283185307179586476925286766559;
	double theta1 = TWO_PI * u1;
	double theta2 = TWO_PI * u2;
	double s1 = glm::sqrt(1.0 - u3);
	double s2 = glm::sqrt(u3);

	double x = s1 * glm::cos(theta1);
	double y = s1 * glm::sin(theta1);
	double z = s2 * glm::cos(theta2);
	double w = s2 * glm::sin(theta2);

	glm::dvec4 dpt{ x * radius, y * radius, z * radius, w * radius };

	return glm::vec4{ dpt };
}

inline glm::vec4 coneRandom(glm::vec4 dir, float theta)
{
	const float eps = 1e-6f;

	if (theta <= 0.0f)
	{
		return dir;
	}

	float cosTheta = std::cos((double)theta);
	float cosPhi = FX::utils::random(cosTheta, 1.0f);
	float sinPhi = std::sqrt(1.0f - cosPhi * cosPhi);

	glm::vec4 v = FX::utils::random(
		glm::vec4{ -1.0f, -1.0f, -1.0f, -1.0f },
		glm::vec4{ 1.0f,  1.0f,  1.0f,  1.0f }
	);

	v -= glm::dot(v, dir) * dir;

	float len = glm::length(v);
	if (len < glm::epsilon<float>())
	{
		v = glm::vec4{ dir.y, -dir.x, dir.w, -dir.z };
		v -= glm::dot(v, dir) * dir;
		len = glm::length(v);
		if (len < glm::epsilon<float>())
		{
			return dir;
		}
	}

	v /= len;

	return glm::normalize(cosPhi * dir + sinPhi * v);
}

inline fdm::stl::string loadSound(const std::string& path)
{
	fdm::stl::string target = std::format("../../{}/{}", fdm::getModPath(fdm::modID), path);
	if (!fdm::AudioManager::loadSound(target)) fdm::Console::printLine("Couldn't load \"", target, "\"");
	return target;
}

// to make sure the audio engine doesn't die
inline void correctSoundPos(glm::vec4& pos)
{
	glm::vec4 d = pos - fdm::StateGame::instanceObj.player.pos;
	if (glm::dot(d, d) <= 0.01f * 0.01f)
		pos += glm::normalize(d + glm::vec4{ 0, 1, 0, 0 }) * 0.1f;
}
