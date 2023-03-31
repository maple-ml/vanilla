from generate import generate_hook_bindings, generate_pure_bindings
from typing import Any, Dict, List
from objects import Function, Class, exprs
from parser import GenParser
from objects import Runtime
from lexer import GenLexer
import pathlib
import sys
import os

dir_path = os.path.dirname(os.path.realpath(__file__))
os.chdir(dir_path)

class GenExecute:
    def __init__(self, tree):
        self.evaluate(tree)

    def evaluate(self, node):
        if not isinstance(node, tuple):
            return node

        if node is None:
            return None

        match node:
            case [exprs.CLASS, base, parents]:
                Runtime.current_class = Class(base, parents)
                Runtime.classes.append(Runtime.current_class)
                return Runtime.current_class

            case [exprs.FUNC, type, name, args]:  # no =
                type = self.evaluate(type)
                name = self.evaluate(name)
                args = self.evaluate(args)

                if Runtime.current_class:  # part of a class
                    address = f"{Runtime.current_class.name}::{name}"
                    # add function to class
                    func = Function(Runtime.current_class.name, type, name, args, address)
                    Runtime.current_class.functions.append(func)
                else:  # pure
                    # address is a string
                    func = Function(None, type, name, args, name)
                    Runtime.functions.append(func)

            case [exprs.FUNC, type, name, args, option]:  # no =, with modifier
                type = self.evaluate(type)
                name = self.evaluate(name)
                args = self.evaluate(args)
                option = self.evaluate(option)

                if Runtime.current_class:  # part of a class
                    address = f"{Runtime.current_class.name}::{name}"
                    # add function to class
                    func = Function(Runtime.current_class.name, type, name, args, address)
                    if "overload" in option:
                        func.overload = True
                    if "static" in option:
                        func.static = True
                    Runtime.current_class.functions.append(func)
                else:  # pure
                    # address is a string
                    func = Function(None, type, name, args, name)
                    if "overload" in option:
                        func.overload = True
                    if "static" in option:
                        func.static = True
                    Runtime.functions.append(func)

            case [exprs.EQUALS, [exprs.FUNC, type, name, args], address]:
                type = self.evaluate(type)
                name = self.evaluate(name)
                args = self.evaluate(args)
                address = self.evaluate(address)

                if Runtime.current_class:  # part of a class
                    # add function to class
                    func = Function(Runtime.current_class.name, type, name, args, hex(address))
                    Runtime.current_class.functions.append(func)
                else:  # pure
                    # address is a string
                    func = Function(None, type, name, args, address)
                    Runtime.functions.append(func)

            case [exprs.EQUALS, [exprs.FUNC, type, name, args, option], address]:
                type = self.evaluate(type)
                name = self.evaluate(name)
                args = self.evaluate(args)
                address = self.evaluate(address)
                option = self.evaluate(option)

                if Runtime.current_class:  # part of a class
                    # add function to class
                    func = Function(Runtime.current_class.name, type, name, args, hex(address))
                    if "overload" in option:
                        func.overload = True
                    if "static" in option:
                        func.static = True
                    Runtime.current_class.functions.append(func)
                else:  # pure
                    # address is a string
                    func = Function(None, type, name, args, address)
                    if "overload" in option:
                        func.overload = True
                    if "static" in option:
                        func.static = True
                    Runtime.functions.append(func)

            case [exprs.EQUALS, name, value]:
                Runtime.options[self.evaluate(name)] = self.evaluate(value)
                return self.evaluate(value)

            case [exprs.HEX, num]:
                return num

            case [exprs.NUM, num]:
                return num

            case [exprs.POINTER, name]:
                return f"{name}*"

            case [exprs.VAR, name]:
                return name

            case [exprs.FUNC_ARGS, args]:
                ret = []
                for arg, var in args:
                    ret.append((self.evaluate(arg), self.evaluate(var)))
                return ret

            case [exprs.ARRAY, arr]:
                return arr

            case [exprs.ENDCLASS]:
                Runtime.current_class = None

            case [exprs.ATTR, name, value]:
                Runtime.attrs[self.evaluate(name)] = self.evaluate(value)

            case [exprs.MEMBER, name, visibility]:
                if Runtime.current_class:
                    Runtime.current_class.members.append((self.evaluate(visibility), self.evaluate(name)))



def generate_bindings_from_file(filename):
    lexer = GenLexer()
    parser = GenParser()

    try:
        filepath = filename
        file = open(filepath, "r")
    except FileNotFoundError:
        filepath = dir_path + "\\" + filename
        file = open(filepath, "r")

    sys.path.append(filepath.replace(pathlib.PurePath(filename).name, ""))

    for line in file.readlines():
        tree = parser.parse(lexer.tokenize(line))
        GenExecute(tree)
        Runtime.lineno += 1

    functions = Runtime.functions
    classes = Runtime.classes
    attrs = Runtime.attrs
    options = Runtime.options

    if options["binding_type"] == "hook":
        generate_hook_bindings(classes, options)
    elif options["binding_type"] == "pure":
        generate_pure_bindings(functions, classes, attrs, options)
    else:
        ValueError("Invalid binding type")

    Runtime.functions = []
    Runtime.classes = []
    Runtime.attrs = {}
    Runtime.options = {"module_name": None, "imports": [], "binding_type": "pure"}
    Runtime.current_class = None
    Runtime.lineno = 1


if __name__ == "__main__":
    generate_bindings_from_file("../bindings/geometrydash.van")
    generate_bindings_from_file("../bindings/cocos2d.van")