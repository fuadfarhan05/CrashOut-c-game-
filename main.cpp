#include "raylib.h"
//#include "item.hpp"
#include <vector>
#include <cmath>

struct Player {
    Vector2 pos;
    float speed;
    int wood = 0;
    int health = 100;
};

struct Tree {
    Vector2 pos;
    bool chopped = false;
};

struct Zombie {
    Vector2 pos;
    float speed = 1.5f;
    bool spawn = false;
    int health = 30;
};

struct Campfire {
    Vector2 pos;
    bool active = true;
    float healRadius = 80.0f; // heal range 
};

int main() {
    const int screenWidth = 800;
    const int screenHeight = 800;

    const float attackRange = 70.0f;
    const int attackDamage = 20;
    //const int swordDamage = 20;
    float attackCooldown = 0.2f;
    float timeSinceLastAttack = 0.0f;

    const int zombieWidth = 30;
    const int zombieHeight = 30;
    const int healthBarWidth = 30;
    const int healthBarHeight = 5;

    bool isGameOver = false;

    InitWindow(screenWidth, screenHeight, "CrashOut");
    SetTargetFPS(60);

    Player player = { {400, 300}, 4.0f };

    std::vector<Campfire> campfires;
    const int requiredmat = 10;
    


    std::vector<Zombie> zombies;
    for (int i = 0; i < 60; i++) {
        Zombie z;
        z.pos = { (float)GetRandomValue(200, 1500), (float)GetRandomValue(500, 1200) };
        z.spawn = false;
        zombies.push_back(z);
    }

    std::vector<Tree> trees;
    for(int i = 0; i < 100; i++) {
        Tree t;
        t.pos = { (float)GetRandomValue(100, 1500), (float)GetRandomValue(100, 1500) };
        trees.push_back(t);
    }

    Camera2D camera = {0};
    camera.target = player.pos;
    camera.offset = { screenWidth/2.0f, screenHeight/2.0f };
    camera.zoom = 0.7f;

    while(!WindowShouldClose()) {

        float deltaTime = GetFrameTime();
        timeSinceLastAttack += deltaTime;

        // Check Game Over
        if(player.health <= 0) {
            player.health = 0;
            isGameOver = true;
        }

        // Restart game
        if(isGameOver && IsKeyPressed(KEY_R)) {
            player.health = 100;
            player.wood = 0;
            player.pos = {400, 300};
            timeSinceLastAttack = 0.0f;
            isGameOver = false;
            for(auto &z : zombies) {
                z.spawn = false;
                z.health = 30;
                z.pos = { (float)GetRandomValue(200, 1500), (float)GetRandomValue(500, 1200) };
            }
            for(auto &t : trees) t.chopped = false;
            for(auto &c : campfires) c.active = false;
        }

        // Only update game if not game over
        if(!isGameOver) {

            // Player movement
            if (IsKeyDown(KEY_W)) player.pos.y -= player.speed;
            if (IsKeyDown(KEY_S)) player.pos.y += player.speed;
            if (IsKeyDown(KEY_A)) player.pos.x -= player.speed;
            if (IsKeyDown(KEY_D)) player.pos.x += player.speed;

            // Player attack
            if(IsKeyDown(KEY_SPACE) && timeSinceLastAttack >= attackCooldown) {
                timeSinceLastAttack = 0.0f;
                for(auto &zombie: zombies) {
                    if(zombie.spawn){
                        float dx = zombie.pos.x - player.pos.x;
                        float dy = zombie.pos.y - player.pos.y;
                        float distance = sqrt(dx*dx + dy*dy);
                        if(distance <= attackRange) {
                            zombie.health -= attackDamage;
                            if(zombie.health <= 0) {
                                zombie.spawn = false;
                                player.wood += 1;
                            }
                        }
                    }
                }
            }

            if(IsKeyPressed(KEY_F) && player.wood >= requiredmat) {
                Campfire c;
                c.pos = player.pos;
                campfires.push_back(c);
                player.wood -= requiredmat;
            }

            for(auto &c : campfires) {
                if(c.active) {
                    float dx = player.pos.x - c.pos.x;
                    float dy = player.pos.y - c.pos.y;
                    float dist = sqrt(dx*dx + dy*dy);
                    if(dist <= c.healRadius) {
                        player.health += 80 * deltaTime; 
                        if(player.health > 100) player.health = 100;
                    }
                }

            }

    

            // Chop trees
            Rectangle playerRect = { player.pos.x, player.pos.y, 40,40 };
            for(auto &tree: trees) {
                Rectangle treeRect = { tree.pos.x, tree.pos.y, 40, 60 };
                if(!tree.chopped && CheckCollisionRecs(playerRect, treeRect) && IsKeyPressed(KEY_E)) {
                    tree.chopped = true;
                    player.wood++;
                    for (auto &zombie : zombies) {
                        if (!zombie.spawn) { 
                            zombie.pos = { tree.pos.x + GetRandomValue(-100, 100), tree.pos.y + GetRandomValue(-100, 100) };
                            zombie.spawn = true;
                            break; 
                        }
                    }
                }
            }

            // Zombies movement & collision avoidance
            float minDistance = 35.0f;
            for(auto &zombie : zombies) {
                if(zombie.spawn) {
                    Vector2 direction = { player.pos.x - zombie.pos.x, player.pos.y - zombie.pos.y };
                    float length = sqrt(direction.x*direction.x + direction.y*direction.y);
                    if(length != 0) {
                        direction.x /= length;
                        direction.y /= length;
                    }
                    zombie.pos.x += direction.x * zombie.speed;
                    zombie.pos.y += direction.y * zombie.speed;

                    // Avoid overlapping other zombies
                    for(auto &other : zombies) {
                        if(&zombie == &other || !other.spawn) continue;
                        Vector2 diff = { zombie.pos.x - other.pos.x, zombie.pos.y - other.pos.y };
                        float dist = sqrt(diff.x*diff.x + diff.y*diff.y);
                        if(dist < minDistance && dist != 0) {
                            diff.x /= dist;
                            diff.y /= dist;
                            float pushAmount = (minDistance - dist) * 0.5f;
                            zombie.pos.x += diff.x * pushAmount;
                            zombie.pos.y += diff.y * pushAmount;
                        }
                    }

                    // Damage player on collision
                    Rectangle zombieRect = { zombie.pos.x, zombie.pos.y, zombieWidth, zombieHeight };
                    if(CheckCollisionRecs(playerRect, zombieRect)) {
                        player.health -= 1;
                        if(player.health < 0) player.health = 0;
                    }
                }
            }

            camera.target = player.pos;
        }

        // --- Drawing ---
        BeginDrawing();
        ClearBackground(DARKGREEN);

        if(isGameOver) {
            DrawText("GAME OVER", screenWidth/2 - 100, screenHeight/2 - 50, 50, RED);
            DrawText("Press R to Restart", screenWidth/2 - 100, screenHeight/2 + 20, 20, WHITE);
        } else {
            BeginMode2D(camera);

            DrawRectangle(0,0,1600,1600,(Color){34,139,34,255});

            for(auto &tree: trees) {
                if(!tree.chopped) DrawRectangle(tree.pos.x, tree.pos.y, 40,60,GREEN);
                else DrawRectangle(tree.pos.x, tree.pos.y, 20,10,BROWN);
            }


            for(auto &zombie : zombies) {
                if(zombie.spawn) {
                    DrawRectangle(zombie.pos.x, zombie.pos.y, zombieWidth, zombieHeight, BLACK);
                    float healthPercent = (float)zombie.health / 30.0f;
                    DrawRectangle(zombie.pos.x, zombie.pos.y - 10, healthBarWidth * healthPercent, healthBarHeight, RED);
                    DrawRectangleLines(zombie.pos.x, zombie.pos.y - 10, healthBarWidth, healthBarHeight, BLACK);
                }
            }

            //add break to remove multiple placements
            for(auto &c : campfires) {
                if(c.active) {
                    DrawCircleV(c.pos, 15, ORANGE);        // fire
                    DrawCircleLines(c.pos.x, c.pos.y, c.healRadius, YELLOW); // heal area
                    break;
                }
            }
            

            DrawRectangle(player.pos.x, player.pos.y, 30, 30, RED);
            DrawCircleLines(player.pos.x + 15, player.pos.y + 15, attackRange, BLUE);

            EndMode2D();

            // --- HUD ---
            int hudX = 20;
            int healthBarY = 20;
            int barWidth = 200;
            int healthBarHeightHUD = 20;
            int cooldownBarHeight = 10;
            int padding = 5;

            // Player health
            float healthPercentHUD = player.health / 100.0f;
            DrawRectangle(hudX, healthBarY, barWidth, healthBarHeightHUD, GRAY);
            DrawRectangle(hudX, healthBarY, barWidth * healthPercentHUD, healthBarHeightHUD, GREEN);
            DrawRectangleLines(hudX, healthBarY, barWidth, healthBarHeightHUD, BLACK);
            DrawText("Health", hudX, healthBarY - 15, 10, BLACK);

            // Attack cooldown
            float coolPercent = timeSinceLastAttack / attackCooldown;
            if(coolPercent > 1.0f) coolPercent = 1.0f;
            int cooldownY = healthBarY + healthBarHeightHUD + padding;
            DrawRectangle(hudX, cooldownY, barWidth, cooldownBarHeight, GRAY);
            DrawRectangle(hudX, cooldownY, barWidth * coolPercent, cooldownBarHeight, BLUE);
            DrawRectangleLines(hudX, cooldownY, barWidth, cooldownBarHeight, BLACK);
            DrawText("Attack", hudX, cooldownY - 15, 10, BLACK);

            // Wood
            int woodY = cooldownY + cooldownBarHeight + padding;
            DrawText(TextFormat("Wood: %d", player.wood), hudX, woodY, 20, RAYWHITE);

            // Controls
            int controlsY = woodY + 25;
            DrawText("WASD = move, E = chop, SPACE = attack", hudX, controlsY, 15, RAYWHITE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
