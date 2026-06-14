#pragma once
#include "MinimaxAI.h"

enum class Difficulty { Easy, Medium, Hard };

class AIFactory {
public:
    static MinimaxAI create(Difficulty difficulty);
    static const char* name(Difficulty difficulty);
};
