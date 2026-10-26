#pragma once
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include "Vegetation.h"

class Tree : public Vegetation
{
private:
	sf::Texture rootTexture;
	std::vector<sf::Texture> trunkTextures;
	sf::Texture leavesTexture;
	std::vector<sf::Texture*> textures;
public:
	Tree(sf::Texture& rootTexture, std::vector<sf::Texture>& trunkTextures, sf::Texture& leavesTexture): rootTexture(rootTexture), trunkTextures(trunkTextures), leavesTexture(leavesTexture)
	{
		textures.push_back(&this->rootTexture);
		for (sf::Texture& tex : this->trunkTextures)
		{
			textures.push_back(&tex);
		}
		textures.push_back(&this->leavesTexture);
	}
	
	const std::vector<sf::Texture*>& GetTextures() override;
};