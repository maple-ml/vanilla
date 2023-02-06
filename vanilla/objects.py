from enum import Enum
from typing import Dict, List, Tuple

class exprs(Enum):
    EQUALS = 1
    VAR = 2
    NUM = 3
    STR = 4
    CLASS = 7
    POINTER = 8
    FUNC_ARGS = 9
    FUNC = 10
    HEX = 11
    ENDCLASS = 12
    ARRAY = 13
    ATTR = 14

class Function:
    def __init__(self, class_: str, type: str, name: str, args: List[Tuple[str, str]], address: int, calling_conv: str = "__thiscall") -> None:
        self.class_: str = class_
        self.type: str = type
        self.name: str = name
        self.args: List[Tuple[str, str]] = args
        self.address: int = address
        self.calling_conv: str = calling_conv
        self.overload = False
        self.static = False

    @property
    def args_name_and_types(self) -> List[str]:
        ret = []
        for arg, var in self.args:
            ret.append(f"{arg} {var}")
        return ret

    @property
    def args_types(self) -> List[str]:
        ret = []
        for arg, _ in self.args:
            ret.append(arg)
        return ret

    @property
    def args_names(self) -> List[str]:
        ret = []
        for _, var in self.args:
            ret.append(var)
        return ret

    def __repr__(self) -> str:
        return f"Function({self.class_}, {self.type}, {self.name}, {self.args}, {self.address})"

    def __str__(self) -> str:
        return f"{self.type} {self.class_}::{self.name}({', '.join(self.args_name_and_types)})"

class Class:
    def __init__(self, name: str, parents: List[str], functions: List[Function]=None, attrs: Dict[str, str]=None) -> None:
        self.name: str = name
        self.parents: List[str] = parents
        if functions:
            self.functions: List[Function] = functions
        else:
            self.functions: List[Function] = []
        
        if attrs:
            self.attrs: Dict[str, str] = attrs
        else:
            self.attrs: Dict[str, str] = {}


    def __repr__(self) -> str:
        return f"Class({self.name}, {self.parents}, {self.functions})"

    def __str__(self) -> str:
        return f"class {self.name} : {', '.join(self.parents)}"

class Runtime:
    classes: List[Class] = []
    functions: List[Function] = []
    attrs: Dict[str, str] = {}
    current_class: Class = None
    options: Dict[str, str] = {"module_name": None, "imports": [], "binding_type": "pure"}

    lineno: int = 1