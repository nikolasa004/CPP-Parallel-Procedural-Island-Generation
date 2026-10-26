#include <vector>  
#include <SFML/System.hpp>  
#include <SFML/Graphics.hpp>  
#include "Bush.h"

const std::vector<sf::Texture*>& Bush::GetTextures()
{
	return textures;
}