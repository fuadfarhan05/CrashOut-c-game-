#pragma once

#include "raylib.h"

// Arrow types shared across modules
enum ArrowType { ARROW_NORMAL = 0, ARROW_FIRE = 1, ARROW_PIERCING = 2 };

struct Player {
    Vector2 pos;
    float speed;
    int wood = 0;
    int health = 100;
    bool hasBow = false;
    int ammo[3] = {10, 3, 2}; // Normal, Fire, Piercing
    ArrowType selectedArrow = ARROW_NORMAL;
};

struct Zombie {
    Vector2 pos;
    float speed = 1.5f;
    bool spawn = false;
    int health = 30;
    // burn/dot fields
    float burnTimer = 0.0f;
    float burnDPS = 0.0f;
};

// Simple pickup dropped in world to replenish ammo
struct Pickup {
    Vector2 pos;
    ArrowType type;
    int amount;
    bool active = true;
    bool givesBow = false; // if true, this pickup is a bow object that grants the bow + some ammo
};
