#pragma once

enum Config;
struct Model;

struct ModelProcessor
{
    static void process(Model& model, Config config);
};
