#include <vector>  
#include <SFML/System.hpp>  
#include <SFML/Graphics.hpp>  
#include "Tree.h"

const std::vector<sf::Texture*>& Tree::GetTextures()
{
	return textures;
}