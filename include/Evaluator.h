#pragma once
#include "Board.h"
#include <memory>

class Evaluator {
public:
    virtual ~Evaluator() = default;
    virtual int evaluate(const Board& board, Color side) const = 0;
    virtual const char* name() const = 0;
};

class MaterialEvaluator : public Evaluator {
public:
    int evaluate(const Board& board, Color side) const override;
    const char* name() const override { return "Material"; }
};

class PositionalEvaluator : public Evaluator {
public:
    int evaluate(const Board& board, Color side) const override;
    const char* name() const override { return "Positional"; }
};

class AggressiveEvaluator : public Evaluator {
public:
    int evaluate(const Board& board, Color side) const override;
    const char* name() const override { return "Aggressive"; }
};

class EvaluatorFactory {
public:
    enum class Strategy { Material, Positional, Aggressive };
    static std::unique_ptr<Evaluator> create(Strategy strategy);
};
