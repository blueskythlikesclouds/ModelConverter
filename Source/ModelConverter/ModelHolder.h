#pragma once

#include "Model.h"
#include "Material.h"

struct ModelHolder
{
    Model model;
    std::vector<Material> materials;
};
