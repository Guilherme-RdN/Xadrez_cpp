#include "AIFactory.h"

MinimaxAI AIFactory::create(Difficulty difficulty) {
    switch (difficulty) {
        case Difficulty::Easy:
            return MinimaxAI(2, EvaluatorFactory::create(EvaluatorFactory::Strategy::Material), true);
        case Difficulty::Medium:
            return MinimaxAI(3, EvaluatorFactory::create(EvaluatorFactory::Strategy::Positional));
        case Difficulty::Hard:
            return MinimaxAI(4, EvaluatorFactory::create(EvaluatorFactory::Strategy::Positional));
    }
    return MinimaxAI(3, EvaluatorFactory::create(EvaluatorFactory::Strategy::Positional));
}

const char* AIFactory::name(Difficulty difficulty) {
    switch (difficulty) {
        case Difficulty::Easy:   return "Facil";
        case Difficulty::Medium: return "Medio";
        case Difficulty::Hard:   return "Dificil";
    }
    return "Medio";
}
