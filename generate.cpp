#include "generate.h"
#include <fmt/core.h>
#include "vanilla.h"
#include <sstream>
#include <fstream>
#include <regex>

namespace vanilla {
    std::string get_arg_names_and_types(FunctionProto* func, Class c, MemberFunctionProto member) {
        std::string arg_names_and_types = "";

        if (!member.is_static) {
            arg_names_and_types += fmt::format("{0}* {1}", c.name, "self");
            if (func->args.size() != 0) {
                arg_names_and_types += ", ";
            }
        }

        for (int i = 0; i < func->args.size(); i++) {
            arg_names_and_types += fmt::format("{0} {1}", func->args[i].first.name, func->args[i].second);
            if (i != func->args.size() - 1) {
                arg_names_and_types += ", ";
            }
        }

        return arg_names_and_types;
    }

    std::string get_arg_names(FunctionProto* func, Class c, MemberFunctionProto member) {
        std::string arg_names = "";

        if (!member.is_static) {
            arg_names += "self";
            if (func->args.size() != 0) {
                arg_names += ", ";
            }
        }

        for (int i = 0; i < func->args.size(); i++) {
            arg_names += func->args[i].second;
            if (i != func->args.size() - 1) {
                arg_names += ", ";
            }
        }

        return arg_names;
    }

    std::string get_arg_types(FunctionProto* func, Class c, MemberFunctionProto member) {
        std::string arg_types = "";

        if (!member.is_static) {
            arg_types += fmt::format("{0}*", c.name);
            if (func->args.size() != 0) {
                arg_types += ", ";
            }
        }

        for (int i = 0; i < func->args.size(); i++) {
            arg_types += func->args[i].first.name;
            if (i != func->args.size() - 1) {
                arg_types += ", ";
            }
        }

        return arg_types;
    }

    std::string generate_function_pure_bind(FunctionProto const& f) {
        std::string out = "";



        return out;
    }

    std::string generate_class_pure_binds(Class c) {
        std::string out = "";
        
        for (Field f : c.fields) {
            FunctionProto* func = f.get_fn();
            FunctionBindField* bind = f.get_as<FunctionBindField>();

            if (func && bind) {

            }
        }

        return out;
    }

    std::string generate_class_hook(Class c) {
        std::vector<std::string> iterated_functions = std::vector<std::string>();

        std::string out = "";

        if (c.superclasses.size() == 0) {
            out += fmt::format("class {0} ", c.name);
        }
        else {
            // can have multiple superclasses, so we need to add them all
            out += fmt::format("class {0} : public ", c.name);
            
            for (int i = 0; i < c.superclasses.size(); i++) {
                out += fmt::format("{0}", c.superclasses[i]);
                if (i != c.superclasses.size() - 1) {
                    out += ", ";
                }
            }
        }

        out += " {\npublic:\n";

        // fields
        
        for (Field f : c.fields) {
            FunctionProto* func = f.get_fn();
            FunctionBindField* bind = f.get_as<FunctionBindField>();
            if (func && bind && bind->binds.win != 0) {
                std::cout << "Found function " << func->name << std::endl;

                std::string static_ = "";

                MemberFunctionProto member = bind->prototype;

                if (member.is_static) {
                    static_ = "static ";
                }
                
                if (member.type != FunctionType::Normal) {
                    continue;
                }

                std::stringstream stream;
                stream << std::hex << bind->binds.win;
                std::string hex_address( stream.str() );

                // check if we've already iterated this function
                // because we don't generate overloads for the same function
                // we'll just allow them to hook the first one
                // but still be allowed to call the other overloads
                if (std::find(iterated_functions.begin(), iterated_functions.end(), func->name) != iterated_functions.end()) {
                    std::cout << func->name << " has already been iterated" << std::endl;

                    out += fmt::format("    {2}CINNAMON_API {0} {1}(", func->ret.name, func->name, static_);

                    std::string arg_names_and_types = get_arg_names_and_types(func, c, member);

                    out += arg_names_and_types;

                    out += ") {\n";

                    std::string arg_types = get_arg_types(func, c, member);

                    std::string arg_names = get_arg_names(func, c, member);

                    out += "        return reinterpret_cast<";
                    out += func->ret.name;
                    out += "(__thiscall*)(";
                    out += arg_types;
                    out += ")>(cinnamon::utilities::base + 0x";
                    out += hex_address;
                    out += ")(";
                    out += arg_names;
                    out += ");\n    }\n";
                    continue;
                }

                // first section, straight up binded function

                out += fmt::format("    {2}CINNAMON_API {0} {1}(", func->ret.name, func->name, static_);

                std::string arg_names_and_types = get_arg_names_and_types(func, c, member);

                out += arg_names_and_types;

                out += ") {\n";

                std::string arg_types = get_arg_types(func, c, member);

                std::string arg_names = get_arg_names(func, c, member);

                out += "        return reinterpret_cast<";
                out += func->ret.name;
                out += "(__thiscall*)(";
                out += arg_types;
                out += ")>(cinnamon::utilities::base + 0x";
                out += hex_address;
                out += ")(";
                out += arg_names;
                out += ");\n    }\n";

                // next is the hook overload

                out += "    CINNAMON_API static inline std::pair<std::string, size_t> ";
                out += func->name;
                out += "(pybind::function hook) {\n        return std::pair<std::string, size_t>(\"";
                out += c.name;
                out += "::";
                out += func->name;
                out += "H\", cinnamon::utilities::base + 0x";
                out += hex_address;
                out += ");\n    }\n";

                // then the hook itself

                if (func->ret.name == "void") {
                    out += fmt::format(
                        "    CINNAMON_API static void __fastcall {0}H({1}) {{"
                            "std::multimap<std::string, void*>::iterator itr;"
                            "for (itr = cinnamon::hooks::hooks.begin(); itr != cinnamon::hooks::hooks.end(); ++itr) {{"
                                "if (itr->first == __FUNCTION__) {{"
                                    "pybind::gil_scoped_acquire acquire;"
                                    "reinterpret_cast<{3}(*)({4})>(itr->second)({2});"
                                    "pybind::gil_scoped_release release;"
                                "}}"
                            "}}"
                            "if (cinnamon::hooks::hooks.count(__FUNCTION__) == 0) {{"
                                "std::multimap<std::string, pybind::function>::iterator pyitr;"
                                "for (pyitr = cinnamon::hooks::pythonHooks.begin(); pyitr != cinnamon::hooks::pythonHooks.end(); ++pyitr) {{"
                                    "if (pyitr->first == __FUNCTION__) {{"
                                        "try {{"
                                            "pybind::gil_scoped_acquire acquire;"
                                            "pyitr->second({2});"
                                            "pybind::gil_scoped_release release;"
                                        "}}"
                                        "catch (pybind::error_already_set& e) {{"
                                            "cinnamon::logger::log(\"An exception occurred while calling a python hook:\", cinnamon::logger::LoggingLevel::ERROR);"
                                            "cinnamon::python::printPythonException(e);"
                                        "}}"
                                    "}}"
                                "}}"
                            "}}"
                            "{0}O_({2});"
                        "}}",
                        func->name,
                        arg_names_and_types,
                        arg_names,
                        func->ret.name,
                        arg_types
                    );
                }
                else {
                    out += fmt::format(
                        "    CINNAMON_API static {0} __fastcall {1}H({2}) {{"
                        "std::multimap<std::string, void*>::iterator itr;"
                        "for (itr = cinnamon::hooks::hooks.begin(); itr != cinnamon::hooks::hooks.end(); ++itr) {{"
                            "if (itr->first == __FUNCTION__) {{"
                                "pybind::gil_scoped_acquire acquire;"
                                "reinterpret_cast<{0}(*)({4})>(itr->second)({3});"
                                "pybind::gil_scoped_release release;"
                            "}}"
                        "}}"
                        "if (cinnamon::hooks::hooks.count(__FUNCTION__) == 0) {{"
                            "std::multimap<std::string, pybind::function>::iterator pyitr;"
                            "for (pyitr = cinnamon::hooks::pythonHooks.begin(); pyitr != cinnamon::hooks::pythonHooks.end(); ++pyitr) {{"
                                "if (pyitr->first == __FUNCTION__) {{"
                                    "try {{"
                                        "pybind::gil_scoped_acquire acquire;"
                                        "{0} ret = pyitr->second({3}).cast<{0}>();"
                                        "pybind::gil_scoped_release release;"
                                        "return ret;"
                                    "}}"
                                    "catch (pybind::error_already_set& e) {{"
                                        "cinnamon::logger::log(\"An exception occurred while calling a python hook:\", cinnamon::logger::LoggingLevel::ERROR);"
                                        "cinnamon::python::printPythonException(e);"
                                    "}}"
                                "}}"
                            "}}"
                        "}}"
                        "return {1}O_({3});"
                        "}}",
                        func->ret.name,
                        func->name,
                        arg_names_and_types,
                        arg_names,
                        arg_types
                    );
                }

                // original function

                std::string ret_name = func->ret.name;

                out += fmt::format(
                    "    CINNAMON_API static inline {0}(__thiscall* {1}O_)({2}); ",
                    ret_name,
                    func->name,
                    arg_types
                );

                // c

                out += fmt::format(
                    "CINNAMON_API static inline unsigned int {0}C = 1;",
                    func->name
                );

                // original part 2

                if (func->ret.name == "void") {
                    out += fmt::format(
                        "CINNAMON_API static void __fastcall {0}O({1}) {{"
                            "std::multimap<std::string, void*>::iterator itr;"
                            "int itr2 = 0;"
                            "for (itr = cinnamon::hooks::hooks.begin(); itr != cinnamon::hooks::hooks.end(); ++itr) {{"
                                "if (cinnamon::hooks::hooks.count(\"{2}::{0}H\") <= {0}C) {{"
                                    "std::multimap<std::string, pybind::function>::iterator pyitr;"
                                    "for (pyitr = cinnamon::hooks::pythonHooks.begin(); pyitr != cinnamon::hooks::pythonHooks.end(); ++pyitr) {{"
                                        "if (cinnamon::hooks::pythonHooks.count(\"{2}::{0}H\")"
                                        "+cinnamon::hooks::hooks.count(\"{2}::{0}H\") == {0}C) {{"
                                            "{0}C = 1;"
                                            "return {0}O_({3});"
                                        "}}"
                                        "if (pyitr->first == \"{2}::{0}H\") {{"
                                            "if (itr2+cinnamon::hooks::hooks.count(\"{2}::{0}H\") == {0}C) {{"
                                                "{0}C++;"
                                                "try {{"
                                                    "pybind::gil_scoped_acquire acquire;"
                                                    "pyitr->second({3});"
                                                    "pybind::gil_scoped_release release;"
                                                "}}"
                                                "catch (pybind::error_already_set& e) {{"
                                                    "cinnamon::logger::log(\"An exception occurred while calling a python hook:\", cinnamon::logger::LoggingLevel::ERROR);"
                                                    "cinnamon::python::printPythonException(e);"
                                                "}}"
                                            "}}"
                                            "itr2++;"
                                        "}}"
                                    "}}"
                                "}}"
                                "if (itr->first == \"{2}::{0}H\") {{"
                                    "if (itr2 == {0}C) {{"
                                        "{0}C++;"
                                        "{4}(*hook)({5}) = ({4}(*)({5}))itr->second;"
                                    "}}"
                                    "itr2++;"
                                "}}"
                            "}}"
                            "if (cinnamon::hooks::pythonHooks.count(\"{2}::{0}H\") == 0) {{"
                                "std::multimap<std::string, pybind::function>::iterator pyitr;"
                                "for (pyitr = cinnamon::hooks::pythonHooks.begin(); pyitr != cinnamon::hooks::pythonHooks.end(); ++pyitr) {{"
                                    "if (pyitr->first == \"{2}::{0}H\") {{"
                                        "if (cinnamon::hooks::hooks.count(\"{2}::{0}H\")"
                                        "+cinnamon::hooks::pythonHooks.count(\"{2}::{0}H\") == {0}C) {{"
                                            "{0}C = 1;"
                                            "return {0}O_({3});"
                                        "}}"
                                        "if (itr2+cinnamon::hooks::hooks.count(\"{2}::{0}H\") == {0}C) {{"
                                            "{0}C++;"
                                            "try {{"
                                                "pybind::gil_scoped_acquire acquire;"
                                                "pyitr->second({3});"
                                                "pybind::gil_scoped_release release;"
                                            "}}"
                                            "catch (pybind::error_already_set& e) {{"
                                                "cinnamon::logger::log(\"An exception occurred while calling a python hook:\", cinnamon::logger::LoggingLevel::ERROR);"
                                                "cinnamon::python::printPythonException(e);"
                                            "}}"
                                        "}}"
                                        "itr2++;"
                                    "}}"
                                "}}"
                            "}}"
                            "if (cinnamon::hooks::pythonHooks.count(\"{2}::{0}H\")"
                            "+cinnamon::hooks::hooks.count(\"{2}::{0}H\") == {0}C) {{"
                                "{0}O_({3});"
                                "{0}C = 1;"
                                "return;"
                            "}}"
                            "return {0}O_({3});"
                        "}}"
                        ,
                        func->name,
                        arg_names_and_types,
                        c.name,
                        arg_names,
                        func->ret.name,
                        arg_types
                    );
                }
                else {
                    // same thing but with a return value
                    out += fmt::format(
                        "CINNAMON_API static {0} __fastcall {1}O({2}) {{"
                            "std::multimap<std::string, void*>::iterator itr;"
                            "int itr2 = 0;"
                            "for (itr = cinnamon::hooks::hooks.begin(); itr != cinnamon::hooks::hooks.end(); ++itr) {{"
                                "if (cinnamon::hooks::hooks.count(\"{5}::{1}H\") <= {1}C) {{"
                                    "std::multimap<std::string, pybind::function>::iterator pyitr;"
                                    "for (pyitr = cinnamon::hooks::pythonHooks.begin(); pyitr != cinnamon::hooks::pythonHooks.end(); ++pyitr) {{"
                                        "if (cinnamon::hooks::pythonHooks.count(\"{5}::{1}H\")"
                                        "+cinnamon::hooks::hooks.count(\"{5}::{1}H\") == {1}C) {{"
                                            "{1}C = 1;"
                                            "return {1}O_({3});"
                                        "}}"
                                        "if (pyitr->first == \"{5}::{1}H\") {{"
                                            "if (itr2+cinnamon::hooks::hooks.count(\"{5}::{1}H\") == {1}C) {{"
                                                "{1}C++;"
                                                "try {{"
                                                    "pybind::gil_scoped_acquire acquire;"
                                                    "pyitr->second({3});"
                                                    "pybind::gil_scoped_release release;"
                                                "}}"
                                                "catch (pybind::error_already_set& e) {{"
                                                    "cinnamon::logger::log(\"An exception occurred while calling a python hook:\", cinnamon::logger::LoggingLevel::ERROR);"
                                                    "cinnamon::python::printPythonException(e);"
                                                "}}"
                                            "}}"
                                            "itr2++;"
                                        "}}"
                                    "}}"
                                "}}"
                                "if (itr->first == \"{5}::{1}H\") {{"
                                    "if (itr2 == {1}C) {{"
                                        "{1}C++;"
                                        "{4}(*hook)({6}) = ({4}(*)({6}))itr->second;"
                                    "}}"
                                    "itr2++;"
                                "}}"
                            "}}"
                            "if (cinnamon::hooks::hooks.count(\"{5}::{1}H\") == 0) {{"
                                "std::multimap<std::string, pybind::function>::iterator pyitr;"
                                "for (pyitr = cinnamon::hooks::pythonHooks.begin(); pyitr != cinnamon::hooks::pythonHooks.end(); ++pyitr) {{"
                                    "if (pyitr->first == \"{5}::{1}H\") {{"
                                        "if (cinnamon::hooks::hooks.count(\"{5}::{1}H\")"
                                        "+cinnamon::hooks::pythonHooks.count(\"{5}::{1}H\") == {1}C) {{"
                                            "{1}C = 1;"
                                            "return {1}O_({3});"
                                        "}}"
                                        "if (itr2+cinnamon::hooks::hooks.count(\"{5}::{1}H\") == {1}C) {{"
                                            "{1}C++;"
                                            "try {{"
                                                "pybind::gil_scoped_acquire acquire;"
                                                "pyitr->second({3});"
                                                "pybind::gil_scoped_release release;"
                                            "}}"
                                            "catch (pybind::error_already_set& e) {{"
                                                "cinnamon::logger::log(\"An exception occurred while calling a python hook:\", cinnamon::logger::LoggingLevel::ERROR);"
                                                "cinnamon::python::printPythonException(e);"
                                            "}}"
                                        "}}"
                                        "itr2++;"
                                    "}}"
                                "}}"
                            "}}"
                            "if (cinnamon::hooks::pythonHooks.count(\"{5}::{1}H\")"
                            "+cinnamon::hooks::hooks.count(\"{5}::{1}H\") == {1}C) {{"
                                "{1}C = 1;"
                                "return {1}O_({3});"
                            "}}"
                            "return {1}O_({3});"
                        "}}",
                        func->ret.name,
                        func->name,
                        arg_names_and_types,
                        arg_names,
                        func->ret.name,
                        c.name,
                        arg_types
                    );

                }

                out += fmt::format(
                    "CINNAMON_API static inline size_t {0}A = cinnamon::utilities::base + 0x{1};",
                    func->name,
                    hex_address
                );

                out += fmt::format(
                    "CINNAMON_API static inline const char* {0}N = \"{0}\";\n",
                    func->name
                );

                iterated_functions.push_back(func->name);
            }
            else if (f.get_as<InlineField>()) {
                InlineField* inline_field = f.get_as<InlineField>();

                out += fmt::format("    CINNAMON_API {0}\n", inline_field->inner);
            }
        }

        return out;
    }

    std::string generate_class_hook_binds(Class c) {
        std::vector<std::string> iterated_functions = std::vector<std::string>();
        std::string out = "";

        std::string lowercase_class_name = c.name;
        std::transform(lowercase_class_name.begin(), lowercase_class_name.end(), lowercase_class_name.begin(), ::tolower);

        if (c.superclasses.size() == 0) {
            out += fmt::format("    auto {0} = pybind::class_<{1}>(m, \"{1}\");\n", lowercase_class_name, c.name);
        }
        else {
            out += fmt::format("    auto {0} = pybind::class_<{1}, {2}>(m, \"{1}\");\n", lowercase_class_name, c.name, c.superclasses[0]);
        }

        for (Field f : c.fields) {
            FunctionProto* func = f.get_fn();
            FunctionBindField* bind = f.get_as<FunctionBindField>();

            if (func && bind && bind->binds.win != 0) {
                // check if we've already iterated this function
                // because we don't generate overloads for the same function
                // since we don't support that just yet
                // so we only do the first one
                if (std::find(iterated_functions.begin(), iterated_functions.end(), func->name) != iterated_functions.end()) {
                    std::cout << func->name << " has already been iterated2" << std::endl;
                    std::string arg_types = get_arg_types(func, c, bind->prototype);
                    out += fmt::format("    {0}.def(\"{1}\", pybind::overload_cast<{2}>(&{3}::{1}));\n", lowercase_class_name, func->name, arg_types, c.name);
                    continue;
                }

                MemberFunctionProto member = bind->prototype;

                if (member.type != FunctionType::Normal) {
                    continue;
                }

                std::string arg_names_and_types = get_arg_names_and_types(func, c, member);
                std::string arg_names = get_arg_names(func, c, member);
                std::string arg_types = get_arg_types(func, c, member);

                std::stringstream stream;
                stream << std::hex << bind->binds.win;
                std::string hex_address( stream.str() );

                // first section, straight up binded function

                out += fmt::format("    {0}.def(\"{1}\", pybind::overload_cast<{2}>(&{3}::{1}));\n", lowercase_class_name, func->name, arg_types, c.name);

                // next is the hook overload

                out += fmt::format("    {0}.def(\"{1}\", pybind::overload_cast<pybind::function>(&{2}::{1}));\n", lowercase_class_name, func->name, c.name);

                // then the original

                out += fmt::format("    {0}.def(\"{1}O\", []({2}) {{ return {3}::{1}O({4}); }});\n", lowercase_class_name, func->name, arg_names_and_types, c.name, arg_names);

                // then the address

                out += fmt::format("    {0}.attr(\"{1}A\") = {2}::{1}A;\n", lowercase_class_name, func->name, c.name);

                // then the name

                out += fmt::format("    {0}.attr(\"{1}N\") = \"{1}\";\n", lowercase_class_name, func->name);

                // then the hook

                out += fmt::format("    cinnamon::hooks::hookCinnamon((PVOID){0}::{1}A, {0}::{1}H, (LPVOID*)&{0}::{1}O_);\n\n", c.name, func->name);

                iterated_functions.push_back(func->name);
            }
            else if (f.get_as<InlineField>()) {
                InlineField* inline_field = f.get_as<InlineField>();



                // find name of inline field function
                // (it's in this form)
                // inline static CCMenuItemSpriteExtra* create(cocos2d::CCNode* sprite, cocos2d::CCObject* target, cocos2d::SEL_MenuHandler callback) {
                //     return CCMenuItemSpriteExtra::create(sprite, nullptr, target, callback);
                // }
                std::regex inline_field_regex(".* (.*) (.*)\\((.*)\\) \\{");

                std::smatch match;

                std::string inline_field_name = "";
                std::string args = "";

                if (std::regex_search(inline_field->inner, match, inline_field_regex)) {
                    if (match[2] == c.name) {
                        continue; // constructor
                    }
                    else if (match[2] == "~" + c.name) {
                        continue; // destructor
                    }

                    inline_field_name = match[2];
                    args = match[3];
                }

                // change args from type name, type name, type name to just type, type, type

                std::regex inline_field_args_regex("([a-zA-Z0-9_:\\*]+) ([a-zA-Z0-9_:\\*]+)");

                std::string inline_field_args = "";

                std::smatch match2;

                while (std::regex_search(args, match2, inline_field_args_regex)) {
                    inline_field_args += match2[1];
                    inline_field_args += ", ";
                    args = match2.suffix();
                }

                // remove the last ", "

                inline_field_args = inline_field_args.substr(0, inline_field_args.size() - 2);

                out += fmt::format("    {0}.def(\"{1}\", pybind::overload_cast<{3}>(&{2}::{1}));\n", lowercase_class_name, inline_field_name, c.name, inline_field_args);
            }
        }

        return out;
    }

    std::string generate_bindings(Root root, std::string filename, bool pure) {
        std::string output = "";

        output += fmt::format("// AUTOMATICALLY GENERATED HOOK BINDING for \"{0}\" module\n", filename);

        output += fmt::format("// Generated by vanilla v{0}\n", vanilla::version);
        output += "#pragma once\n";
        output += "#include <cocos2d.h>\n";
        output += "#include \"core/utilities/game.h\"\n";
        output += "#include \"core/python.h\"\n";
        output += "#include \"core/hooks.h\"\n";
        output += "#include \"pybind11.h\"\n";
        output += "#include \"pybind11/embed.h\"\n";
        output += "#include \"bindings/manual_bindings.h\"\n";
        output += "#include <cocos-ext.h>\n";
        output += "#include \"bindings/enums.h\"\n";
        output += "#include \"bindings/winstl.h\"\n";
        output += "USING_NS_CC;\n";
        output += "namespace pybind = pybind11;\n";

        // forward declare needed classes

        for (Class c : root.classes) {
            output += fmt::format("class {0};\n", c.name);
        }
        
        // classes

        std::vector<std::string> iterated_classes = std::vector<std::string>();

        for (Class c : root.classes) {
            if (std::find(iterated_classes.begin(), iterated_classes.end(), c.name) != iterated_classes.end()) {
                continue;
            }

            for (std::string dep : c.depends) {
                if (std::find(iterated_classes.begin(), iterated_classes.end(), dep) != iterated_classes.end() || dep.rfind("cocos2d::", 0) == 0) {
                    continue;
                }

                Class dep_class = *root[dep];
                if (!pure) {
                    output += generate_class_hook(dep_class);
                    output += "\n};\n";
                }
                iterated_classes.push_back(dep);
            }

            if (!pure) {
                output += generate_class_hook(c);
                output += "\n};\n";
            }

            iterated_classes.push_back(c.name);
        }

        return output;
    }

    std::string generate_module(Root root, std::string filename, bool pure) {
        std::string output = "";

        output += fmt::format("// AUTOMATICALLY GENERATED HOOK BINDING for \"{0}\" module\n", filename);

        output += fmt::format("// Generated by vanilla v{0}\n", vanilla::version);
        output += "#pragma once\n";
        output += "#include <cocos2d.h>\n";
        output += "#include \"core/utilities/game.h\"\n";
        output += "#include \"core/python.h\"\n";
        output += "#include \"core/hooks.h\"\n";
        output += "#include \"pybind11.h\"\n";
        output += "#include \"pybind11/embed.h\"\n";
        output += "#include \"bindings/manual_bindings.h\"\n";
        output += "#include <cocos-ext.h>\n";
        output += "#include \"bindings/enums.h\"\n";
        output += "#include \"bindings/winstl.h\"\n";
        output += "#include \"bindings/geometry_dash_bindings.h\"\n";
        output += "USING_NS_CC;\n";
        output += "namespace pybind = pybind11;\n";

        if (pure) {
            output += "PYBIND11_EMBEDDED_MODULE(cocos2d, m) {\n\n";
        }
        else {
            output += "PYBIND11_EMBEDDED_MODULE(geometry_dash, m) {\n\n";
        }


        // empty the iterated classes vector
        std::vector<std::string> iterated_classes2 = std::vector<std::string>();

        output += "    manualbindings::geometry_dash_init(m);\n";

        for (Class c : root.classes) {
            if (std::find(iterated_classes2.begin(), iterated_classes2.end(), c.name) != iterated_classes2.end()) {
                continue;
            }

            for (std::string dep : c.depends) {
                if (std::find(iterated_classes2.begin(), iterated_classes2.end(), dep) != iterated_classes2.end() || dep.rfind("cocos2d::", 0) == 0) {
                    continue;
                }

                Class dep_class = *root[dep];
                if (pure) {
                    output += generate_class_pure_binds(dep_class);
                }
                else {
                    output += generate_class_hook_binds(dep_class);
                }

                iterated_classes2.push_back(dep);
            }

            if (pure) {
                output += generate_class_pure_binds(c);
            }
            else {
                output += generate_class_hook_binds(c);
            }

            iterated_classes2.push_back(c.name);
        }

        output += "\n}";

        return output;
    }

    void generate(std::string filename, std::string header_filename, std::string module_filename) {
        Root root = broma::parse_file(filename);

        bool pure = false; // hook bindings if false

        if (filename == "cocos2d.bro") {
            pure = true;
        }

        // base includes/defines

        std::string bindings = generate_bindings(root, header_filename, pure);

        // output to file

        std::ofstream myfile;

        myfile.open (header_filename);

        myfile << bindings;

        myfile.close();

        std::cout << "Outputted to " << header_filename << "!" << std::endl;


        std::string module_ = generate_module(root, module_filename, pure);

        // output to file

        std::ofstream myfile2;

        myfile2.open (module_filename);

        myfile2 << module_;

        myfile2.close();

        std::cout << "Outputted to " << module_filename << "!" << std::endl;

        
    }
}