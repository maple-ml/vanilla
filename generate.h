#pragma once
#include <broma.hpp>

using namespace broma;


namespace vanilla {
    void generate(std::string filename, std::string header_filename, std::string module_filename);
    std::string generate_file_base(Root root, bool pure);
    std::string generate_class(Class c, bool pure);
    std::string generate_function(FunctionProto const& f, bool pure);
}