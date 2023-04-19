#include "ModelConverter.h"
#include "Material.h"
#include "Config.h"

static bool equalsAny(const char* arg, std::initializer_list<const char*> values)
{
    for (auto& value : values)
    {
        if (_stricmp(arg, value) == 0)
            return true;
    }
    return false;
}

int main(int argc, const char* argv[])
{
    std::string src;
    std::string dst;
    Config config = CONFIG_FLAG_NONE;
    bool overwriteMaterials = false;

    for (int i = 1; i < argc; i++)
    {
        if (equalsAny(argv[i], { "--unleashed", "--swa" }))
            config = CONFIG_UNLEASHED;

        else if (equalsAny(argv[i], { "--generations", "--gens", "--bb" }))
            config = CONFIG_GENERATIONS;

        else if (equalsAny(argv[i], { "--lost_world", "--lostworld", "--lw", "--sonic2013" }))
            config = CONFIG_LOST_WORLD;

        else if (equalsAny(argv[i], { "--forces", "--wars" }))
            config = CONFIG_FORCES;

        else if (equalsAny(argv[i], { "--frontiers", "--rangers" }))
            config = CONFIG_FRONTIERS;

        else if (equalsAny(argv[i], { "--override-materials", "-y" }))
            overwriteMaterials = true;

        else if (src.empty())
            src = argv[i];

        else if (dst.empty())
            dst = argv[i];
    }

    if (dst.empty())
    {
        dst = src;

        size_t slash = dst.find_last_of("\\/");
        size_t dot = dst.find_last_of('.');

        if (dot != std::string::npos && (slash == std::string::npos || slash < dot))
            dst.erase(dot);

        dst += ".model";
    }

    ModelConverter converter(src.c_str());
    converter.convert(config);
    converter.model.save(dst.c_str(), config);

    std::string dir = dst;
    size_t index = dir.find_last_of("\\/");
    dir.erase(index + 1);

    for (auto& material : converter.materials)
    {
        std::string path = dir + material.name + ".material";
        if (overwriteMaterials || !std::filesystem::exists(path))
            material.save(path.c_str(), config);
    }
}
