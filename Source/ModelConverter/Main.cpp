#include "ModelConverter.h"
#include "Material.h"
#include "Config.h"

int main(int argc, const char* argv[])
{
    std::string src;
    std::string dst;

    for (int i = 1; i < argc; i++)
    {
        if (src.empty())
        {
            src = argv[i];
        }
        else if (dst.empty())
        {
            dst = argv[i];
        }
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
    converter.convert({ false, false });

    converter.model.save(dst.c_str());

    std::string dir = dst;
    size_t index = dir.find_last_of("\\/");
    dir.erase(index + 1);

    for (auto& material : converter.materials)
    {
        std::string path = dir + material.name + ".material";
        if (!std::filesystem::exists(path))
            material.save(path.c_str());
    }
}
