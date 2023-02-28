import os

def join_list(iter):
    return ", ".join(iter)

def generate_hook_bindings(classes, options):
    out = f"""// AUTOMATICALLY GENERATED HOOK BINDING for "{options['module_name']}" module
#pragma once
#include <cocos2d.h>
#include "core/utilities/game.h"
#include "core/hooks.h"
#include "pybind11.h"
#include "pybind11/embed.h"

USING_NS_CC;
namespace pybind = pybind11;

#define uint unsigned int
#define constchar const char
#define CONSTCCPoint const CCPoint
#define CONSTCCSize const CCSize
#define CONSTCCRect const CCRect
#define CONSTccBezierConfig const ccBezierConfig
#define CONSTccColor3B const ccColor3B
#define CONSTCCAffineTransform const CCAffineTransform
#define CONSTCCBool const CCBool
#define CONSTCCInteger const CCInteger
#define CONSTCCFloat const CCFloat
#define CONSTCCDouble const CCDouble
#define CONSTCCString const CCString
#define CONSTCCArray const CCArray
#define CONSTCCDictionary const CCDictionary
#define CONSTCCSet const CCSet
#define CONSTstdstring const std::string
#define CONSTCCColor4F const ccColor4F
#define CONSTCCColor4B const ccColor4B

"""

    for class_ in classes:
        if class_.parents: # has base classes
            out += f"class {class_.name} : public {', '.join(class_.parents)} {{\n"
        else:
            out += f"class {class_.name} {{\n"

        out += "public:\n"

        # just don't even try to scroll sideways
        for func in class_.functions:
            # call
            out += f"    CINNAMON_API {func.type} {func.name}({join_list(func.args_name_and_types)}) {{ "
            out += f"return reinterpret_cast<{func.type}({func.calling_conv}*)({join_list(func.args_types)})>(cinnamon::utilities::base + {func.address})({join_list(func.args_names)}); }}\n"

            # hook overload
            out += f'    CINNAMON_API static inline std::pair<std::string, size_t> {func.name}(pybind::function hook) {{ return std::pair<std::string, size_t>("{func.class_}::{func.name}H", cinnamon::utilities::base + {func.address}); }}\n'

            # hook
            if func.type == "void":
                out += f"    CINNAMON_API static {func.type} __fastcall {func.name}H({join_list(func.args_name_and_types)}) {{ std::multimap<std::string, void*>::iterator itr; for (itr = cinnamon::hooks::hooks.begin(); itr != cinnamon::hooks::hooks.end(); ++itr) {{ if (itr->first == __FUNCTION__) {{ reinterpret_cast<{func.type}(*)({join_list(func.args_types)})>(itr->second)({join_list(func.args_names)}); }} }} if (cinnamon::hooks::hooks.count(__FUNCTION__) == 0) {{ std::multimap<std::string, pybind::function>::iterator pyitr; for (pyitr = cinnamon::hooks::pythonHooks.begin(); pyitr != cinnamon::hooks::pythonHooks.end(); ++pyitr) {{ if (pyitr->first == __FUNCTION__) {{ pybind::gil_scoped_acquire acquire; pyitr->second({join_list(func.args_names)}).cast<{func.type}>(); pybind::gil_scoped_release release; return; }} }} }} return {func.name}O_({join_list(func.args_names)}); }}\n"
            else:
                out += f"    CINNAMON_API static {func.type} __fastcall {func.name}H({join_list(func.args_name_and_types)}) {{ std::multimap<std::string, void*>::iterator itr; for (itr = cinnamon::hooks::hooks.begin(); itr != cinnamon::hooks::hooks.end(); ++itr) {{ if (itr->first == __FUNCTION__) {{ reinterpret_cast<{func.type}(*)({join_list(func.args_types)})>(itr->second)({join_list(func.args_names)}); }} }} if (cinnamon::hooks::hooks.count(__FUNCTION__) == 0) {{ std::multimap<std::string, pybind::function>::iterator pyitr; for (pyitr = cinnamon::hooks::pythonHooks.begin(); pyitr != cinnamon::hooks::pythonHooks.end(); ++pyitr) {{ if (pyitr->first == __FUNCTION__) {{ pybind::gil_scoped_acquire acquire; {func.type} ret = pyitr->second({join_list(func.args_names)}).cast<{func.type}>(); pybind::gil_scoped_release release; return ret; }} }} }} return {func.name}O_({join_list(func.args_names)}); }}\n"

            # original
            out += f"    CINNAMON_API static inline {func.type}(__thiscall* {func.name}O_)({join_list(func.args_types)});\n"

            # original part 2 (god save me)
            if func.type == "void":
                out += f'    CINNAMON_API static inline int {func.name}C = 1; CINNAMON_API static void __fastcall {func.name}O({join_list(func.args_name_and_types)}) {{ std::multimap<std::string, void*>::iterator itr; int itr2 = 0; for (itr = cinnamon::hooks::hooks.begin(); itr != cinnamon::hooks::hooks.end(); ++itr) {{ if (cinnamon::hooks::hooks.count("{func.class_}::{func.name}H") == {func.name}C) {{ std::multimap<std::string, pybind::function>::iterator pyitr; for (pyitr = cinnamon::hooks::pythonHooks.begin(); pyitr != cinnamon::hooks::pythonHooks.end(); ++pyitr) {{ if (cinnamon::hooks::pythonHooks.count("{func.class_}::{func.name}H")+cinnamon::hooks::hooks.count("{func.class_}::{func.name}H") == {func.name}C) {{ {func.name}C = 1; return {func.name}O_({join_list(func.args_names)}); }} if (pyitr->first == "{func.class_}::{func.name}H") {{ if (itr2 == {func.name}C) {{ {func.name}C++; pybind::gil_scoped_acquire acquire; pyitr->second({join_list(func.args_names)}); pybind::gil_scoped_release release; }} itr2++; }} }} }} if (itr->first == "{func.class_}::{func.name}H") {{ if (itr2 == {func.name}C) {{ {func.name}C++; {func.type}(*hook)({join_list(func.args_types)}) = ({func.type}(*)({join_list(func.args_types)}))itr->second; }} itr2++; }} }} if (cinnamon::hooks::pythonHooks.count("{func.class_}::{func.name}H")+cinnamon::hooks::hooks.count("{func.class_}::{func.name}H") == {func.name}C) {{ {func.name}O_({join_list(func.args_names)}); {func.name}C = 1; return; }} return {func.name}O_({join_list(func.args_names)}); }} \n'
            else:
                out += f'    CINNAMON_API static inline int {func.name}C = 1; CINNAMON_API static {func.type} __fastcall {func.name}O({join_list(func.args_name_and_types)}) {{ std::multimap<std::string, void*>::iterator itr; int itr2 = 0; for (itr = cinnamon::hooks::hooks.begin(); itr != cinnamon::hooks::hooks.end(); ++itr) {{ if (cinnamon::hooks::hooks.count("{func.class_}::{func.name}H") == {func.name}C) {{ std::multimap<std::string, pybind::function>::iterator pyitr; for (pyitr = cinnamon::hooks::pythonHooks.begin(); pyitr != cinnamon::hooks::pythonHooks.end(); ++pyitr) {{ if (cinnamon::hooks::pythonHooks.count("{func.class_}::{func.name}H")+cinnamon::hooks::hooks.count("{func.class_}::{func.name}H") == {func.name}C) {{ {func.name}C = 1; return {func.name}O_({join_list(func.args_names)}); }} if (pyitr->first == "{func.class_}::{func.name}H") {{ if (itr2 == {func.name}C) {{ {func.name}C++; pybind::gil_scoped_acquire acquire; pyitr->second({join_list(func.args_names)}); pybind::gil_scoped_release release; }} itr2++; }} }} }} if (itr->first == "{func.class_}::{func.name}H") {{ if (itr2 == {func.name}C) {{ {func.name}C++; {func.type}(*hook)({join_list(func.args_types)}) = ({func.type}(*)({join_list(func.args_types)}))itr->second; }} itr2++; }} }} if (cinnamon::hooks::pythonHooks.count("{func.class_}::{func.name}H")+cinnamon::hooks::hooks.count("{func.class_}::{func.name}H") == {func.name}C) {{ {func.type} ret = {func.name}O_({join_list(func.args_names)}); {func.name}C = 1; return ret; }} return {func.name}O_({join_list(func.args_names)}); }} \n'

            # address
            out += f"    CINNAMON_API static inline size_t {func.name}A = cinnamon::utilities::base + {func.address};\n"

            # name
            out += f'    CINNAMON_API static inline const char* {func.name}N = "{func.name}H";\n'

            # TODO: add cinnamon hooking with minhook/util func

            out += ""

        out += "};\n\n"

        # pybind

    out += f"PYBIND11_EMBEDDED_MODULE({options['module_name']}, m) {{"

    # imports
    for mod in options.get("imports", []):
        out += f'\n    pybind::module_::import("{mod}");'

    if options.get("imports", None):
        out += "\n"

    for class_ in classes:
        if class_.parents:
            out += f'\n    auto {class_.name.lower()} = pybind::class_<{class_.name}, {join_list(class_.parents)}>(m, "{class_.name}");\n'
        else:
            out += f'\n    auto {class_.name.lower()} = pybind::class_<{class_.name}>(m, "{class_.name}");\n'
        
        for func in class_.functions:

            # call
            #if func.args:
            out += f'    {class_.name.lower()}.def("{func.name}", pybind::overload_cast<{join_list(func.args_types)}>(&{class_.name}::{func.name}));\n'
            #else:
            #    out += f'    {class_.name.lower()}.def("{func.name}", &{class_.name}::{func.name});\n'
            # overload tricks
            out += f'    {class_.name.lower()}.def("{func.name}", pybind::overload_cast<pybind::function>(&{class_.name}::{func.name}));\n'

            # original lambda
            out += f'    {class_.name.lower()}.def("{func.name}O", []({join_list(func.args_name_and_types)}) {{ return {class_.name}::{func.name}O({join_list(func.args_names)}); }});\n'

            # address
            out += f'    {class_.name.lower()}.attr("{func.name}A") = {class_.name}::{func.name}A;\n'

            # name
            out += f'    {class_.name.lower()}.attr("{func.name}N") = {class_.name}::{func.name}N;\n'

            # hook
            out += f"    cinnamon::hooks::hookCinnamon((PVOID){class_.name}::{func.name}A, {class_.name}::{func.name}H, (LPVOID*)&{class_.name}::{func.name}O_);\n"

    out += "}"

    if not os.path.exists("../../src/bindings/"):
        os.mkdir("../../src/bindings/")
    open(f"../../src/bindings/{options['module_name']}_bindings.h", "w").write(out)

def generate_pure_bindings(functions, classes, attrs, options):
    out = f"""// AUTOMATICALLY GENERATED PURE BINDING for "{options['module_name']}" module
#pragma once
#include <cocos2d.h>
#include "core/utilities/game.h"
#include "core/hooks.h"
#include "pybind11.h"
#include "pybind11/embed.h"

USING_NS_CC;
namespace pybind = pybind11;

#define uint unsigned int
#define constchar const char
#define longlong long long
#define ulonglong ulong long
#define CONSTCCPoint const CCPoint
#define CONSTCCSize const CCSize
#define CONSTCCRect const CCRect
#define CONSTccBezierConfig const ccBezierConfig
#define CONSTccColor3B const ccColor3B
#define CONSTCCAffineTransform const CCAffineTransform
#define CONSTCCBool const CCBool
#define CONSTCCInteger const CCInteger
#define CONSTCCFloat const CCFloat
#define CONSTCCDouble const CCDouble
#define CONSTCCString const CCString
#define CONSTCCArray const CCArray
#define CONSTCCDictionary const CCDictionary
#define CONSTCCSet const CCSet
#define CONSTstdstring const std::string
#define CONSTCCColor4F const ccColor4F
#define CONSTCCColor4B const ccColor4B

"""
    out += f"PYBIND11_EMBEDDED_MODULE({options['module_name']}, m) {{\n"

    # imports
    for mod in options.get("imports", []):
        out += f'\n    pybind::module_::import("{mod}");'

    if options.get("imports", None):
        out += "\n"

    for func in functions:
        if func.overload and not func.static:
            out += f'    {class_.name.lower()}.def("{func.name}", pybind::overload_cast<{join_list(func.args_types)}>(&{func.name}), pybind::return_value_policy::automatic_reference);\n'
        elif func.static and not func.overload:
            out += f'    {class_.name.lower()}.def_static("{func.name}", &{func.name});\n'
        elif func.static and func.overload:
            out += f'    {class_.name.lower()}.def_static("{func.name}", pybind::overload_cast<{join_list(func.args_types)}>(&{func.name}), pybind::return_value_policy::automatic_reference);\n'
        else:
            out += f'    {class_.name.lower()}.def("{func.name}", &{func.name});\n'

    for name, value in attrs.items():
        out += f'    m.attr("{name}") = {value};\n'

    for class_ in classes:
        if class_.parents:
            out += f'\n    auto {class_.name.lower()} = pybind::class_<{class_.name}, {join_list(class_.parents)}>(m, "{class_.name}");\n'
        else:
            out += f'\n    auto {class_.name.lower()} = pybind::class_<{class_.name}>(m, "{class_.name}");\n'

        for func in class_.functions:
            if func.overload and not func.static:
                out += f'    {class_.name.lower()}.def("{func.name}", pybind::overload_cast<{join_list(func.args_types)}>(&{class_.name}::{func.name}), pybind::return_value_policy::automatic_reference);\n'
            elif func.static and not func.overload:
                out += f'    {class_.name.lower()}.def_static("{func.name}", &{class_.name}::{func.name}, pybind::return_value_policy::automatic_reference);\n'
            elif func.static and func.overload:
                out += f'    {class_.name.lower()}.def_static("{func.name}", pybind::overload_cast<{join_list(func.args_types)}>(&{class_.name}::{func.name}), pybind::return_value_policy::automatic_reference);\n'
            else:
                out += f'    {class_.name.lower()}.def("{func.name}", &{class_.name}::{func.name}, pybind::return_value_policy::automatic_reference);\n'

        for member in class_.members:
            match member[0]:
                case "rw":
                    out += f'    {class_.name.lower()}.def_readwrite("{member[1]}", &{class_.name}::{member[1]});\n'
                case "ro":
                    out += f'    {class_.name.lower()}.def_readonly("{member[1]}", &{class_.name}::{member[1]});\n'

    out += "}"

    if not os.path.exists("../../src/bindings"):
        os.mkdir("../../src/bindings/")
    open(f"../../src/bindings/{options['module_name']}_bindings.h", "w").write(out)