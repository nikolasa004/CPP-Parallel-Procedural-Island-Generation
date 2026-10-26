#pragma once
#include <SFML/System.hpp>

struct BorderWalker
{
    sf::Vector2i position;

    BorderWalker(sf::Vector2i position) : position(position) {}
};
