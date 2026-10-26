#include <SFML/Graphics.hpp>
#include <SFML/Window/VideoMode.hpp>
#include "Map.h"
#include "TileType.h"
#include <unordered_map>
#include <filesystem>
#include <iostream>
#include "Tree.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Ecosystem", sf::Style::Fullscreen);

    int mapWidth = 120;
	int mapHeight = 1080 * mapWidth / 1920;

    Map map(mapWidth, mapHeight, 0.3f, 100, 0.025f, 0.8f, 0.015f, 0.65f);

    bool displayedMap = false;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (!displayedMap)
        {
            window.clear();


            float tileSizeX = static_cast<float>(window.getSize().x) / mapWidth;
            float tileSizeY = static_cast<float>(window.getSize().y) / mapHeight;
            float tileSize = std::min(tileSizeX, tileSizeY);

            sf::Sprite tileSprite;
            for (int x = 0; x < map.GetWidth(); x++)
            {
                for (int y = 0; y < map.GetHeight(); y++)
                {
                    tileSprite = map.GetMapTiles()[x][y];
                    tileSprite.setPosition(x * tileSize, y * tileSize);
                    sf::Vector2u texSize = tileSprite.getTexture()->getSize();
                    tileSprite.setScale(tileSize / texSize.x, tileSize / texSize.y);

                    window.draw(tileSprite);
                }
            }


            for (auto& [pos, veg] : map.GetVegetation())
            {
                int x = pos->x;
                int y = pos->y;
                std::vector<sf::Texture*> vegetationTextures = veg->GetTextures();

                int partIndex = 0;
                for (sf::Texture* tex : vegetationTextures)
                {
                    sf::Sprite vegSprite;
                    vegSprite.setTexture(*tex);

                    sf::Vector2u texSize = tex->getSize();

                    if (vegetationTextures.size() < 2)
                    {
						float scaleFactor = 1.5f + static_cast<float>(rand()) / RAND_MAX * (2 - 1.5f);
                        vegSprite.setScale((tileSize * scaleFactor) / texSize.x, (tileSize * scaleFactor) / texSize.y);
                        vegSprite.setOrigin(texSize.x / 2.f, texSize.y);
                        vegSprite.setPosition(x * tileSize + tileSize / 2.f,(y + 1) * tileSize);
                    }
                    else
                    {
                        if (partIndex == vegetationTextures.size() - 1)
                        {
                            float scaleFactor = 1.5f + static_cast<float>(rand()) / RAND_MAX * (3.0f - 1.5f);
                            vegSprite.setScale((tileSize * scaleFactor) / texSize.x, (tileSize * scaleFactor) / texSize.y);
                            vegSprite.setOrigin(texSize.x / 2.f, texSize.y);
                            vegSprite.setPosition(x * tileSize + tileSize / 2.f,(y - partIndex + 1) * tileSize);
                        }
                        else
                        {
                            vegSprite.setScale(tileSize / texSize.x, tileSize / texSize.y);
                            vegSprite.setPosition(x * tileSize,(y - partIndex) * tileSize);
                        }
                        partIndex++;
                    }

                    window.draw(vegSprite);
                }
            }

            displayedMap = true;
            window.display();
        }
    }

    return 0;
}
