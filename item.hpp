#pragma once

#include "raylib.h"
#include <vector>
#include <cmath>

class Item {
    public:
        std::string name;
        std::string descriptions;
        int damage;

        Item(std::string name, std::string description, int damage);
        virtual ~Item() = default;


}