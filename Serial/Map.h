#pragma once
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <unordered_map>
#include "TileType.h"
#include "Walker.h"
#include "BorderWalker.h"
#include "Vegetation.h"
#include "Tree.h"
#include "Bush.h"
#include <functional>

struct Vector2iHash {
	std::size_t operator()(const sf::Vector2i& v) const noexcept {
		return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
	}
};
struct Vector2iEqual {
	bool operator()(const sf::Vector2i& lhs, const sf::Vector2i& rhs) const noexcept {
		return lhs.x == rhs.x && lhs.y == rhs.y;
	}
};

class Map
{
private:
    float width;
    float height;
    std::vector<std::vector<TileType>> map;
	std::vector<std::vector<sf::Sprite>> mapTiles;
	std::unordered_map<sf::Vector2i, Vegetation*, Vector2iHash, Vector2iEqual> vegetation;
	std::unordered_map<std::string, sf::Texture> textures;
    float targetMapPercent;
	float targetTreesPercent;
	float forestChance;
	float targetBushesPercent;
	float bushClusterChance;
    int numOfTiles = 0;

    std::vector<Walker> walkers;
	std::vector<BorderWalker> borderWalkers;
	int maxWalkers;

	std::vector<sf::Vector2i> sandPositions;
	std::vector<sf::Vector2i> waterPositions;

public:
	Map(float width, float height, float targetMapPercent, int maxWalkers, float targetTreesPercent, float forestChance, float targetBushesPercent, float bushClusterPercent) : width(width), height(height), map(width, std::vector<TileType>(height, TileType::Ocean)), targetMapPercent(targetMapPercent), maxWalkers(maxWalkers), targetTreesPercent(targetTreesPercent), forestChance(forestChance), targetBushesPercent(targetBushesPercent), bushClusterChance(bushClusterPercent)
    {
		mapTiles.resize(width, std::vector<sf::Sprite>(height));
		LoadTextures();
        InitializeMap();
		MapGeneration();
    }

	float GetWidth() const { return width; }
	float GetHeight() const { return height; }
	std::vector<std::vector<sf::Sprite>>& GetMapTiles() { return mapTiles; }
    std::vector<std::pair<const sf::Vector2i*, Vegetation*>> GetVegetation()
    {
        std::vector<std::pair<const sf::Vector2i*, Vegetation*>> sortedVegetation;
        sortedVegetation.reserve(vegetation.size());

        for (auto& [pos, veg] : vegetation) {
            sortedVegetation.push_back({ &pos, veg });
        }

		std::sort(sortedVegetation.begin(), sortedVegetation.end(),
			[](const auto& a, const auto& b) {

				if (a.first->y != b.first->y)
					return a.first->y < b.first->y;

				bool aIsBush = dynamic_cast<Bush*>(a.second) != nullptr;
				bool bIsBush = dynamic_cast<Bush*>(b.second) != nullptr;

				if (aIsBush != bIsBush)
					return aIsBush;

				return false;
			}
		);


        return sortedVegetation;
    }

	const std::vector<TileType>& operator[](int index) const { return map[index]; }

    sf::Vector2f Direction();

	void LoadTextures();
	void GetTextureFiles(std::string folder);

    void InitializeMap();
    void MapGeneration();
    void SandGeneration();
    void WaterGeneration();
	void TreeGeneration();
	void SetTree(int x, int y, std::string treeColor, int treeHeight);
	void BushGeneration();

	void EnchantSandTiles();
	void EnchantOceanTiles();
	void EnchantWaterTiles();

	void DrawMap();
	uint8_t GetTileMask(int x, int y, TileType tileType);
	std::string MaskToKey(uint8_t mask, std::string prefix);

	void ChanceForDelete();
	void ChanceForBirth();
	void DirectionChange();
	void PositionChange();
	sf::Vector2i MoveBorderWalker(int x, int y);
	bool HasAdjacentGrass(int x, int y);
	bool IsInside(int x, int y);

    float RandomValue();
	int RandomValue(int min, int max);

	~Map()
	{
		for (auto& [pos, veg] : vegetation) {
			delete veg;
		}
	}
};