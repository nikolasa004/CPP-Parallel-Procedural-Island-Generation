#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Vegetation
{
public:
    virtual ~Vegetation() = default;
    virtual const std::vector<sf::Texture*>& GetTextures() = 0;
};
