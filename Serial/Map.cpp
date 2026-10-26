#include <SFML/System.hpp>
#include <vector>
#include <algorithm>
#include "Map.h"
#include "TileType.h"
#include "Walker.h"
#include "BorderWalker.h"
#include <cstdlib>
#include <ctime>
#include <unordered_map>
#include <filesystem>
#include <iostream>
#include <random>
#include <chrono>

std::mt19937 randomEngine(std::random_device{}());

auto startTime = std::chrono::high_resolution_clock::now();
auto checkpointTime = startTime;
std::chrono::duration<double> GetElapsedTime(std::chrono::high_resolution_clock::time_point& fromTime)
{
    auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = currentTime - fromTime;
	fromTime = currentTime;
	return elapsed;
}

void Map::LoadTextures()
{
    std::string folder = "../Assets/Grass";
	GetTextureFiles(folder);
    folder = "../Assets/Sand";
    GetTextureFiles(folder);
    folder = "../Assets/Ocean";
    GetTextureFiles(folder);
    folder = "../Assets/Water";
    GetTextureFiles(folder);
    folder = "../Assets/Trees";
    GetTextureFiles(folder);
	folder = "../Assets/Bushes";
	GetTextureFiles(folder);

    for (auto& pair : textures)
    {
        pair.second.setSmooth(false);
    }
}

void Map::GetTextureFiles(std::string folder)
{
    for (const auto& entry : std::filesystem::directory_iterator(folder))
    {
        if (entry.is_regular_file())
        {
            std::string path = entry.path().string();
            std::string name = entry.path().stem().string();

            sf::Texture texture;
            if (!texture.loadFromFile(path))
            {
                std::cerr << "Failed to load texture: " << path << "\n";
                continue;
            }

            textures[name] = std::move(texture);
        }
    }
}

std::vector<Walker> walkers;
void Map::InitializeMap()
{
	std::cout << "Starting serial map generation!\n";
    sf::Vector2i center(width / 2, height / 2);
    Walker walker(sf::Vector2f(center.x, center.y), Direction(), 0.5f);
    map[center.x][center.y] = TileType::Grass;
    walkers.push_back(walker);
    numOfTiles++;
}

sf::Vector2f Map::Direction()
{
    int randNum = RandomValue(0, 4);
    switch (randNum) {
        case 0: return sf::Vector2f(-1, 0); // left
        case 1: return sf::Vector2f(1, 0);  // right
        case 2: return sf::Vector2f(0, -1); // up
        case 3: return sf::Vector2f(0, 1);  // down
    }
}

void Map::MapGeneration() {
    while ((float)numOfTiles / (width*height) < targetMapPercent)
    {
        bool madeTile = false;

        for (auto& walker : walkers) {
            int x = static_cast<int>(walker.position.x);
            int y = static_cast<int>(walker.position.y);

            if (map[x][y] == TileType::Ocean) {
                map[x][y] = TileType::Grass;
                ++numOfTiles;
                madeTile = true;
            }
        }

        ChanceForDelete();
        ChanceForBirth();
        DirectionChange();
        PositionChange();
    }

	std::cout << "Grass generation completed in " << GetElapsedTime(checkpointTime).count() << " seconds\n";
	SandGeneration();
}

void Map::ChanceForDelete()
{
    for (int i = walkers.size() - 1; i >= 0; i--)
    {
        if (RandomValue() < walkers[i].changeChance && walkers.size() > 1)
        {
            walkers.erase(walkers.begin() + i);
        }
    }

}

void Map::ChanceForBirth() {
    size_t originalSize = walkers.size();
    for (size_t i = 0; i < originalSize; i++) {
        if (RandomValue() < walkers[i].changeChance && walkers.size() < maxWalkers) {
            Walker w(walkers[i].position, Direction(), 0.5f);
            walkers.push_back(w);
        }
    }
}

void Map::DirectionChange() {
    for (auto& walker : walkers) {
        if (RandomValue() < walker.changeChance) {
            walker.direction = Direction();
        }
    }
}

void Map::PositionChange()
{
    for (auto& walker : walkers)
    {
        walker.position.x += walker.direction.x;
        walker.position.y += walker.direction.y;

        walker.position.x = std::max(3, static_cast<int>(std::min(walker.position.x, width - 4)));
        walker.position.y = std::max(3, static_cast<int>(std::min(walker.position.y, height - 4)));
    }
}

float Map::RandomValue()
{
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(randomEngine);
}

int Map::RandomValue(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max - 1);
    return dist(randomEngine);
}

void Map::SandGeneration()
{
    bool foundStart = false;
    int x = 0;
    int y = 0;

    for (int i = 0; i < width - 2; i++)
    {
        for (int j = 2; j < height - 2; j++)
        {
            if (map[i][j] == TileType::Ocean && map[i][j + 1] == TileType::Grass)
            {
                map[i][j] = TileType::Sand;
				sandPositions.push_back(sf::Vector2i(i, j));
                x = i;
                y = j;
                foundStart = true;
                break;
            }
        }
        if (foundStart)
        {
            break;
        }
    }

    BorderWalker borderWalker (sf::Vector2i(x, y));

    borderWalkers.push_back(borderWalker);

    while (borderWalkers.size() > 0)
    {
        for (int i = 0; i < borderWalkers.size(); i++)
        {
            borderWalkers[i].position = MoveBorderWalker(borderWalkers[i].position.x, borderWalkers[i].position.y);
            if (borderWalkers[i].position.x == -1)
            {
				borderWalkers.erase(borderWalkers.begin() + i);
            }
        }
    }

	std::cout << "Sand generation completed in " << GetElapsedTime(checkpointTime).count() << " seconds\n";
    WaterGeneration();
}

sf::Vector2i Map::MoveBorderWalker(int x, int y)
{
    int filled = 0;
    int xx = x, yy = y;

    static const std::vector<sf::Vector2i> directions = {
        {0, 1}, {0, -1}, {1, 0}, {-1, 0},
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
    };

    for (auto [dx, dy] : directions) {
        int nx = x + dx;
        int ny = y + dy;

        if (IsInside(nx, ny) && map[nx][ny] == TileType::Ocean && HasAdjacentGrass(nx, ny)) {
            map[nx][ny] = TileType::Sand;
            sandPositions.emplace_back(nx, ny);

            if (filled == 0) {
                xx = nx;
                yy = ny;
                filled++;
            }
            else {
                borderWalkers.emplace_back(sf::Vector2i(nx, ny));
            }
        }
    }

    if (filled == 0) {
        return { -1, -1 };
    }

    return { xx, yy };
}

bool Map::IsInside(int x, int y) {
    return x >= 0 && x < map.size() && y >= 0 && y < map[0].size();
}

bool Map::HasAdjacentGrass(int x, int y) {
    static const std::vector<sf::Vector2i> neighborOffsets = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1},
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1},
        {2, 0}, {-2, 0}, {0, 2}, {0, -2},
        {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
        {1, 2}, {-1, 2}, {1, -2}, {-1, -2}
    };

    for (auto [dx, dy] : neighborOffsets) {
        int nx = x + dx;
        int ny = y + dy;

        if (IsInside(nx, ny) && map[nx][ny] == TileType::Grass) {
            return true;
        }
    }
    return false;
}


void Map::WaterGeneration()
{
    const std::vector<std::vector<TileType>> initialMap(map.begin(), map.end());
    for (int i = 2; i < width - 2; i++)
    {
        for (int j = 2; j < height - 2; j++)
        {
            if (initialMap[i][j] == TileType::Ocean)
            {
                int x = i;
                int y = j;
                bool notWater = true;
                bool sandHole = false;

                while (x < width - 2)
                {
                    x++;
                    if (initialMap[x][y] == TileType::Sand)
                    {
                        notWater = false;
                        sandHole = true;
                        break;
                    }
                    if (initialMap[x][y] == TileType::Grass)
                    {
                        notWater = false;
                        break;
                    }
                }
                if (notWater)
                {
                    continue;
                }

                x = i;
                y = j;
                notWater = true;
                while (x > 2)
                {
                    x--;
                    if (initialMap[x][y] == TileType::Sand)
                    {
                        notWater = false;
                        break;
                    }
                }
                if (notWater)
                {
                    continue;
                }

                x = i;
                y = j;
                notWater = true;
                while (y < height - 2)
                {
                    y++;
                    if (initialMap[x][y] == TileType::Sand)
                    {
                        notWater = false;
                        break;
                    }
                }
                if (notWater)
                {
                    continue;
                }

                x = i;
                y = j;
                notWater = true;
                while (y > 2)
                {
                    y--;
                    if (initialMap[x][y] == TileType::Sand)
                    {
                        notWater = false;
                        break;
                    }
                }
                if (notWater)
                {
                    continue;
                }

                if (sandHole)
                {
					map[i][j] = TileType::Sand;
                    sandPositions.push_back(sf::Vector2i(i, j));
                }
                else
                {
                    map[i][j] = TileType::Water;
                    waterPositions.push_back(sf::Vector2i(i, j));
                }
            }
        }
    }

	std::cout << "Water generation completed in " << GetElapsedTime(checkpointTime).count() << " seconds\n";
    DrawMap();
    EnchantSandTiles();
}

void Map::DrawMap()
{
    sf::Sprite tileSprite;
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            if (map[x][y] == TileType::Grass)
            {
                int variant = rand() % 16 + 1;
                tileSprite.setTexture(textures.at("Grass" + std::to_string(variant)));
            }
            else if (map[x][y] == TileType::Sand)
            {
                int variant = rand() % 11 + 1;
                tileSprite.setTexture(textures.at("Sand" + std::to_string(variant)));
            }
            else if (map[x][y] == TileType::Water)
            {
                tileSprite.setTexture(textures.at("Water"));
            }
            else if (map[x][y] == TileType::Ocean)
            {
                tileSprite.setTexture(textures.at("Ocean"));
            }

            mapTiles[x][y] = tileSprite;
        }
    }
}

const int dx[8] = { -1,  0,  1,  0, -1,  1,  1, -1 };
const int dy[8] = { 0, -1,  0,  1, -1, -1,  1,  1 };
uint8_t Map::GetTileMask(int x, int y, TileType tileType)
{
    int width = map.size();
    int height = map[0].size();

    uint8_t mask = 0;

    for (int i = 0; i < 8; i++)
    {
        int nx = x + dx[i];
        int ny = y + dy[i];

        if (nx >= 0 && nx < width && ny >= 0 && ny < height)
        {
            if (map[nx][ny] == tileType)
            {
                mask |= (1 << i);
            }
        }
    }

    return mask;
}

std::string Map::MaskToKey(uint8_t mask, std::string prefix)
{
    std::string key = prefix;

    if (mask & (1 << 0)) key += "l";
    if (mask & (1 << 1)) key += "u";
    if (mask & (1 << 2)) key += "r";
    if (mask & (1 << 3)) key += "d";
    key += "-";
    if (mask & (1 << 4) && !(mask & (1 << 0)) && !(mask & (1 << 1))) key += "ul";
    if (mask & (1 << 5) && !(mask & (1 << 1)) && !(mask & (1 << 2))) key += "ur";
    if (mask & (1 << 6) && !(mask & (1 << 2)) && !(mask & (1 << 3))) key += "dr";
    if (mask & (1 << 7) && !(mask & (1 << 3)) && !(mask & (1 << 0))) key += "dl";

    return key;
}

void Map::EnchantSandTiles()
{
    sf::Sprite tileSprite;
    for(sf::Vector2i pos:sandPositions)
    {
		int x = pos.x;
		int y = pos.y;
        uint8_t mask = GetTileMask(x, y, TileType::Grass);

        if (mask != 0)
        {
            std::string key = MaskToKey(mask,"Sand-");
            if (textures.find(key) != textures.end())
            {
                auto it = textures.find(key);
                if (it != textures.end())
                {
                    tileSprite.setTexture(it->second);
                    mapTiles[x][y] = tileSprite;
                }
            }
            else
            {
                std::cout << "Texture not found, key:" << key<<"\n";
            }
        }
    }

    EnchantOceanTiles();
}

void Map::EnchantOceanTiles()
{
    sf::Sprite tileSprite;
    for (int x = 0; x < mapTiles.size(); x++)
    {
        for (int y = 0; y < mapTiles[0].size(); y++)
        {
            if (map[x][y] == TileType::Ocean)
            {
                uint8_t mask = GetTileMask(x, y, TileType::Sand);

                if (mask != 0)
                {
                    std::string key = MaskToKey(mask, "Ocean-");
                    if (textures.find(key) != textures.end())
                    {
                        auto it = textures.find(key);
                        if (it != textures.end())
                        {
                            tileSprite.setTexture(it->second);
                            mapTiles[x][y] = tileSprite;
                        }
                    }
                    else
                    {
                        std::cout << "Texture not found, key:" << key << "\n";
                    }
                }
            }
        }
    }

    EnchantWaterTiles();
}

void Map::EnchantWaterTiles()
{
    sf::Sprite tileSprite;
    for (sf::Vector2i pos : waterPositions)
    {
        int x = pos.x;
        int y = pos.y;
        uint8_t mask = GetTileMask(x, y, TileType::Grass);

        if (mask != 0)
        {
            std::string key = MaskToKey(mask, "Water-");
            if (textures.find(key) != textures.end())
            {
                auto it = textures.find(key);
                if (it != textures.end())
                {
                    tileSprite.setTexture(it->second);
                    mapTiles[x][y] = tileSprite;
                }
            }
            else
            {
                std::cout << "Texture not found, key:" << key << "\n";
            }
        }
    }

	std::cout << "Tile enchanting completed in " << GetElapsedTime(checkpointTime).count() << " seconds\n";
    TreeGeneration();
}

void Map::TreeGeneration()
{
    std::string treeColors[] = {"","V","B","P","G","Y"};
    int treeCount = 0;
    bool forest = false;
    int xx = 0, yy = 0;
    std::string treeColor = "";

    while ((float)treeCount / (map.size() * map[0].size()) < targetTreesPercent)
    {
        int x, y;

        if (!forest) {
            x = RandomValue(0, map.size());
            y = RandomValue(0, map[0].size());
        }
        else {
            x = xx;
            y = yy;
        }

        if (!IsInside(x, y) || map[x][y] != TileType::Grass) {
            forest = false;
            continue;
        }

        
        if (!forest) {
            int type = RandomValue(0, 6);
			treeColor = treeColors[type];
        }

        int r = RandomValue(0, 10);
        int treeHeight = 1;
        if (r == 9) treeHeight = 3;
        else if (r > 4) treeHeight = 2;

        SetTree(x, y, treeColor, treeHeight);

        treeCount++;

        if (RandomValue() < forestChance) {
            forest = true;
            int dir = RandomValue(0, 6);
            switch (dir) {
            case 0: xx = x + 1; yy = y; break;
            case 1: xx = x - 1; yy = y; break;
            case 2: xx = x + 1; yy = y + 1; break;
            case 3: xx = x - 1; yy = y + 1; break;
            case 4: xx = x - 1; yy = y - 1; break;
            case 5: xx = x + 1; yy = y - 1; break;
            default: forest = false; break;
            }
        }
        else {
            forest = false;
        }
    }

    BushGeneration();
}
void Map::SetTree(int x, int y, std::string treeColor, int treeHeight)
{
    int r = RandomValue(1, 6);
    sf::Texture rootTexture=textures["TreeR" + std::to_string(r)];

	std::vector<sf::Texture> trunkTextures;
    for (int i = 1; i < treeHeight; i++)
    {
        r = RandomValue(1, 6);
        trunkTextures.push_back(textures["TreeT" + std::to_string(r)]);
    }

	sf::Texture leavesTexture;
    if (treeColor != "")
    {
        r = RandomValue(1,5);
        leavesTexture=textures["TreeL" + treeColor + std::to_string(r)];
    }
    else
    {
        leavesTexture=textures["TreeL"];
    }

	delete vegetation[sf::Vector2i(x, y)];
    vegetation[sf::Vector2i(x, y)] = new Tree(rootTexture, trunkTextures, leavesTexture);
}

void Map::BushGeneration()
{
    std::string bushColors[] = { "", "W", "B" };
    int bushCount = 0;
    bool bushCluster = false;
    int xx = 0, yy = 0;
    std::string bushColor = "";

    while ((float)bushCount / (map.size() * map[0].size()) < targetBushesPercent)
    {
        int x, y;

        if (!bushCluster) {
            x = RandomValue(0, map.size());
            y = RandomValue(0, map[0].size());
        }
        else {
            x = xx;
            y = yy;
        }

        if (!IsInside(x, y) || map[x][y] != TileType::Grass) {
            bushCluster = false;
            continue;
        }

        if (!bushCluster) {
            int type = RandomValue(0, 3);
            bushColor = bushColors[type];
        }

        int r = RandomValue(1, 5);
        sf::Texture bushTexture;
        if (bushColor != "")
            bushTexture = textures["Bush" + bushColor + std::to_string(r)];
        else
            bushTexture = textures["Bush"];

        delete vegetation[sf::Vector2i(x, y)];
        vegetation[sf::Vector2i(x, y)] = new Bush(bushTexture);

        bushCount++;

        if (RandomValue() < bushClusterChance) {
            bushCluster = true;
            int dir = RandomValue(0, 6);
            switch (dir) {
            case 0: xx = x + 1; yy = y; break;
            case 1: xx = x - 1; yy = y; break;
            case 2: xx = x + 1; yy = y + 1; break;
            case 3: xx = x - 1; yy = y + 1; break;
            case 4: xx = x - 1; yy = y - 1; break;
            case 5: xx = x + 1; yy = y - 1; break;
            default: bushCluster = false; break;
            }
        }
        else {
            bushCluster = false;
        }
    }

	std::cout << "Vegetation generation completed in " << GetElapsedTime(checkpointTime).count() << " seconds\n";
	std::cout << "Map generation completed in " << GetElapsedTime(startTime).count() << " seconds\n";
}