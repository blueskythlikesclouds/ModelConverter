#pragma once

struct Config;
struct Model;

struct ModelOptimizer
{
    static void optimize(Model& model, const Config& config);
};
