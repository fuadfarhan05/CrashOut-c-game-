#include "projectile.hpp"
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdlib>

namespace projectile {

struct ArrowInternal {
    Vector2 pos;
    Vector2 vel;
    float life;
    ArrowType type;
    int damage;
    int pierceLeft;
    bool active;
};

static std::vector<ArrowInternal> arrows;

void ClearProjectiles() {
    arrows.clear();
}

void SpawnArrow(const Vector2 &pos, const Vector2 &dir, ArrowType type) {
    ArrowInternal a;
    a.pos = pos;
    a.vel = { dir.x * 800.0f, dir.y * 800.0f }; // internal speed
    a.life = 2.0f;
    a.type = type;
    a.active = true;
    a.pierceLeft = 0;
    if(type == ARROW_NORMAL) { a.damage = 25; a.pierceLeft = 0; }
    else if(type == ARROW_FIRE) { a.damage = 18; a.pierceLeft = 0; }
    else if(type == ARROW_PIERCING) { a.damage = 16; a.pierceLeft = 2; }
    arrows.push_back(a);
}

void UpdateProjectiles(float dt, std::vector<Zombie> &zombies, Player &player, std::vector<Pickup> &pickups) {
    // move + life
    for(auto &a : arrows) {
        if(!a.active) continue;
        a.pos.x += a.vel.x * dt;
        a.pos.y += a.vel.y * dt;
        a.life -= dt;
        if(a.life <= 0.0f) { a.active = false; continue; }

        // check collision with zombies
        for(auto &z : zombies) {
            if(!z.spawn) continue;
            float dx = (z.pos.x + 15.0f) - a.pos.x; // approximate centers
            float dy = (z.pos.y + 15.0f) - a.pos.y;
            float dist = sqrtf(dx*dx + dy*dy);
            if(dist <= 20.0f) {
                // hit
                z.health -= a.damage;

                // fire arrow adds burn
                if(a.type == ARROW_FIRE) {
                    z.burnTimer = std::max(z.burnTimer, 3.0f); // burns for 3s
                    z.burnDPS = 6.0f; // DPS
                }

                // handle piercing
                if(a.type == ARROW_PIERCING) {
                    a.pierceLeft -= 1;
                    if(a.pierceLeft < 0) a.pierceLeft = 0;
                    if(a.pierceLeft == 0) a.active = false;
                } else {
                    a.active = false;
                }

                // if zombie died from the hit, spawn pickup sometimes and reward wood
                if(z.health <= 0) {
                    z.spawn = false;
                    player.wood += 1;
                    // random chance to drop ammo (no bow object)
                    int roll = GetRandomValue(0, 100);
                    if(roll < 50) {
                        // drop normal ammo 2-5
                        Pickup p; p.pos = z.pos; p.type = ARROW_NORMAL; p.amount = GetRandomValue(2,5); p.active = true; p.givesBow = false; pickups.push_back(p);
                    } else if(roll < 70) {
                        // small chance for fire
                        Pickup p; p.pos = z.pos; p.type = ARROW_FIRE; p.amount = 1; p.active = true; p.givesBow = false; pickups.push_back(p);
                    } else if(roll < 85) {
                        Pickup p; p.pos = z.pos; p.type = ARROW_PIERCING; p.amount = 1; p.active = true; p.givesBow = false; pickups.push_back(p);
                    }
                }

                // arrows should only hit one zombie per frame
                break;
            }
        }
    }

    // remove inactive
    arrows.erase(std::remove_if(arrows.begin(), arrows.end(), [](const ArrowInternal &a){ return !a.active; }), arrows.end());

    // apply burn damage to zombies
    for(auto &z : zombies) {
        if(!z.spawn) continue;
        if(z.burnTimer > 0.0f) {
            float dmg = z.burnDPS * dt;
            z.health -= (int)std::round(dmg);
            z.burnTimer -= dt;
            if(z.health <= 0) {
                z.spawn = false;
                // drop loot when killed by burn too
                player.wood += 1;
                int roll = GetRandomValue(0, 100);
                if(roll < 30) {
                    Pickup p; p.pos = z.pos; p.type = ARROW_NORMAL; p.amount = GetRandomValue(1,2); p.active = true; pickups.push_back(p);
                }
            }
        }
    }
}

void DrawProjectiles() {
    for(auto &a : arrows) {
        if(!a.active) continue;
        Color col = WHITE;
        if(a.type == ARROW_NORMAL) col = LIGHTGRAY;
        else if(a.type == ARROW_FIRE) col = ORANGE;
        else if(a.type == ARROW_PIERCING) col = SKYBLUE;
        DrawCircleV(a.pos, 4, col);
    }
}

} // namespace projectile
