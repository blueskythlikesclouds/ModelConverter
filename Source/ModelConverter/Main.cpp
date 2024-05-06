#include "Config.h"
#include "ModelConverter.h"
#include "ModelHolder.h"

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
    Config config = CONFIG_GENERATIONS;
    bool overwriteMaterials = false;
    bool noPause = false;

    for (int i = 1; i < argc; i++)
    {
        if (equalsAny(argv[i], { "--unleashed", "--swa" }))
            config = CONFIG_UNLEASHED;

        else if (equalsAny(argv[i], { "--generations", "--gens", "--bb" }))
            config = CONFIG_GENERATIONS;

        else if (equalsAny(argv[i], { "--lost-world", "--lostworld", "--lw", "--sonic2013" }))
            config = CONFIG_LOST_WORLD;

        else if (equalsAny(argv[i], { "--forces", "--wars" }))
            config = CONFIG_FORCES;

        else if (equalsAny(argv[i], { "--frontiers", "--rangers" }))
            config = CONFIG_FRONTIERS;

        else if (equalsAny(argv[i], { "--generations-raytracing", "--gens-rt" }))
            config = CONFIG_RAYTRACING;

        else if (equalsAny(argv[i], { "--override-materials", "-y" }))
            overwriteMaterials = true;

        else if (equalsAny(argv[i], { "--no-pause", "-np" }))
            noPause = true;

        else if (src.empty())
            src = argv[i];

        else if (dst.empty())
            dst = argv[i];
    }

    if (src.empty())
    {
        printf(R"(ModelConverter - converts 3D models for various Sonic games

Usage:
  ModelConverter [options] [source] [destination]

Description:
  Converts 3D models from one format to another for use in various Sonic games.
  If the destination is not specified, it will be automatically determined from the source.

Options:
  --unleashed                   Convert model for Sonic Unleashed
  --gens                        Convert model for Sonic Generations
  --lw                          Convert model for Sonic Lost World
  --forces                      Convert model for Sonic Forces
  --frontiers                   Convert model for Sonic Frontiers
  --gens-rt                     Convert model for Sonic Generations ray tracing mod
  
  --override-materials or -y    Override existing materials in the output directory
  --no-pause or -np             Don't pause the console when an error occurs

Examples:
  ModelConverter --gens chr_Sonic_HD.fbx chr_Sonic_HD.model -y
  ModelConverter --frontiers chr_sonic.fbx chr_sonic.model
)");

        if (!noPause)
            getchar();

        return 0;
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

    ModelHolder holder;
    if (!ModelConverter::convert(src.c_str(), config, holder))
    {
        printf("ERROR: Failed to import %s\n", src.c_str());

        if (!noPause) 
            getchar();

        return -1;
    }

    if (!holder.model.save(dst.c_str(), config))
    {
        printf("ERROR: Failed to save %s\n", dst.c_str());

        if (!noPause)
            getchar();

        return -1;
    }

    std::string dir = dst;
    size_t index = dir.find_last_of("\\/");
    dir.erase(index + 1);

    for (auto& material : holder.materials)
    {
        std::string path = dir + material.name + ".material";
        if (!overwriteMaterials && std::filesystem::exists(path))
            continue;

        if (!material.save(path.c_str(), config))
        {
            printf("ERROR: Failed to save %s\n", path.c_str());

            if (!noPause) 
                getchar();

            return -1;
        }
    }

    return 0;
}
