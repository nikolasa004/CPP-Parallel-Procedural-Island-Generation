#pragma once
#include <SFML/System.hpp>

struct Walker
{
    sf::Vector2f position;
    sf::Vector2f direction;
	bool alive = true;
    float changeChance;

    Walker(sf::Vector2f position, sf::Vector2f direction, float chance) : position(position), direction(direction), changeChance(chance) {}
};
