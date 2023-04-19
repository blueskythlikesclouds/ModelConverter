#pragma once

enum Config;
struct Model;

struct ModelOptimizer
{
    static void optimize(Model& model, Config config);
};
