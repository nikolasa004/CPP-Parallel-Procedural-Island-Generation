#include <SFML/Graphics.hpp>
#include <SFML/Window/VideoMode.hpp>
#include "Map.h"
#include "TileType.h"
#include <unordered_map>
#include <filesystem>
#include <iostream>
#include "Tree.h"
#include <tbb/tbb.h>
#include <tbb/parallel_for.h>

int main()
{
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Ecosystem", sf::Style::Fullscreen);

    int mapWidth = 120;
    int mapHeight = 1080 * mapWidth / 1920;

	// Parameters: width, height, targetMapPercent, maxWalkers, targetTreesPercent, forestChance, targetBushesPercent, bushClusterChance
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

            std::vector<sf::Sprite> preparedSprites(map.GetWidth() * map.GetHeight());
            tbb::parallel_for(tbb::blocked_range2d<int>(0, map.GetWidth(), 0, map.GetHeight()),[&](const tbb::blocked_range2d<int>& r)
            {
                for (int x = r.rows().begin(); x != r.rows().end(); ++x) {
                    for (int y = r.cols().begin(); y != r.cols().end(); ++y) {
                        sf::Sprite sprite = map.GetMapTiles()[x][y];
                        sprite.setPosition(x * tileSize, y * tileSize);
                        sf::Vector2u texSize = sprite.getTexture()->getSize();
                        sprite.setScale(float(tileSize) / texSize.x,float(tileSize) / texSize.y);

                        preparedSprites[x * map.GetHeight() + y] = sprite;
                    }
                }
            });
            for (auto& sprite : preparedSprites) {
                window.draw(sprite);
            }

			//can't parallelize drawing vegetation, must be in order for correct layering
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
                        vegSprite.setPosition(x * tileSize + tileSize / 2.f, (y + 1) * tileSize);
                    }
                    else
                    {
                        if (partIndex == vegetationTextures.size() - 1)
                        {
                            float scaleFactor = 1.5f + static_cast<float>(rand()) / RAND_MAX * (3.0f - 1.5f);
                            vegSprite.setScale((tileSize * scaleFactor) / texSize.x, (tileSize * scaleFactor) / texSize.y);
                            vegSprite.setOrigin(texSize.x / 2.f, texSize.y);
                            vegSprite.setPosition(x * tileSize + tileSize / 2.f, (y - partIndex + 1) * tileSize);
                        }
                        else
                        {
                            vegSprite.setScale(tileSize / texSize.x, tileSize / texSize.y);
                            vegSprite.setPosition(x * tileSize, (y - partIndex) * tileSize);
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