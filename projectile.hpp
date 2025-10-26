#pragma once

#include "raylib.h"
#include <vector>
#include "entities.hpp"

namespace projectile {

void SpawnArrow(const Vector2 &pos, const Vector2 &dir, ArrowType type);

// Update arrows, apply damage to zombies, create pickups when zombies die and add wood to player
void UpdateProjectiles(float dt, std::vector<Zombie> &zombies, Player &player, std::vector<Pickup> &pickups);

void DrawProjectiles();

// Optional: clear/initialize
void ClearProjectiles();

} // namespace projectile
