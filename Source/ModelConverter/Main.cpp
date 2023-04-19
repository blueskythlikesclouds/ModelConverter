#include "ModelConverter.h"
#include "Config.h"

int main(int argc, const char* argv[])
{
    std::string path(argv[1]);
    std::string dest = path.substr(0, path.find_last_of('.')) + ".model";

    ModelConverter(path.c_str()).convert({true, true}).save(dest.c_str());
}
