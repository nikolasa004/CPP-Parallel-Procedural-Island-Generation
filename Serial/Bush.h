#pragma once
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include "Vegetation.h"

class Bush : public Vegetation
{
private:
	sf::Texture leavesTexture;
	std::vector<sf::Texture*> textures;
public:
	Bush(sf::Texture& leavesTexture) : leavesTexture(leavesTexture)
	{
		textures.push_back(&this->leavesTexture);
	}

	const std::vector<sf::Texture*>& GetTextures() override;
};