#include "vanilla.h"
#include <broma.hpp>
#include "generate.h"
#include <fstream>

using namespace broma;

int main() {
    std::cout << "vanilla v" << vanilla::version << std::endl;

    // parse file

    std::cout << "Parsed file!" << std::endl;

	vanilla::generate("src/bindings/geometrydash.bro", "src/bindings/geometry_dash_bindings.h", "src/bindings/geometry_dash_module.h");

    std::cout << "Generated code!" << std::endl;

	return 0;
}
