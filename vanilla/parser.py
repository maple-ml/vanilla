from sly.yacc import SlyLogger
from lexer import GenLexer
from objects import exprs
from sly import Parser
from objects import Runtime

# disable pesky warnings

def warning(*args, **kwargs):
    pass

SlyLogger.warning = warning

class GenParser(Parser):
    tokens = GenLexer.tokens

    precedence = (
        ("right", "="),
        ("right", ":")
    )

    def __init__(self):
        self.env = {}

    @_("")
    def statement(self, p):
        pass

    @_("expr")
    def statement(self, p):
        return p.expr

    # array def
    @_('"[" [ expr ] { "," expr } "]"')
    def expr(self, p):
        arr = []
        if p.expr0:
            arr = [p.expr0[1]]
            
            for obj in p._5_repeat:
                arr.append(obj[1][1])
        return (exprs.ARRAY, arr)
    
    # func args def
    @_('"(" [ expr expr ] { "," expr expr } ")"')
    def func_args(self, p):
        arr = []
        if p.expr0:
            arr = [(p.expr0[1], p.expr1[1])]
            for obj in p._7_repeat:
                arr.append((obj[1][1], obj[2][1]))
        return (exprs.FUNC_ARGS, arr)
    
    #func def
    @_('expr expr func_args', 'expr expr expr func_args')
    def expr(self, p):
        if getattr(p, "expr2", None): # has modifier
            return (exprs.FUNC, p.expr1, p.expr2, p.func_args, p.expr0)
        return (exprs.FUNC, p.expr0, p.expr1, p.func_args)

    # class def
    @_('expr expr [ ":" expr ] { "," [ expr ] } "{"',
       'expr expr "{"')
    def expr(self, p):
        base = p.expr1[1]
        parents = None
        if getattr(p, "expr2", None):
            parents = [p.expr2[1]]
        
            for parent in p._2_repeat:
                parents.append(parent[1][0][1])

        if p.expr0 != "class":
            SyntaxError("Error while defining class")
        return (exprs.CLASS, base, parents)

    @_('expr expr "=" expr')
    def expr(self, p):
        if p.expr0[1] == "set":
            return (exprs.EQUALS, p.expr1, p.expr2)
        elif p.expr0[1] == "attr":
            return (exprs.ATTR, p.expr1, p.expr2)
        elif p.expr0[1] == "static":
            return (exprs.EQUALS, p.expr0, p.expr1, ["static"])
        elif p.expr0[1] == "overload":
            return (exprs.EQUALS, p.expr0, p.expr1, ["overload"])
        elif p.expr0[1] == "staticoverload":
            return (exprs.EQUALS, p.expr0, p.expr1, ["static", "overload"])

    @_('expr "=" expr')
    def expr(self, p):
        return (exprs.EQUALS, p.expr0, p.expr1)

    # endclass
    @_('"}"')
    def expr(self, p):
        return (exprs.ENDCLASS)

    @_("NAME")
    def expr(self, p):
        return (exprs.VAR, p.NAME)

    @_("NUMBER")
    def expr(self, p):
        return (exprs.NUM, p.NUMBER)
    
    @_("HEX")
    def expr(self, p):
        return (exprs.HEX, p.HEX)

    @_("STRING")
    def expr(self, p):
        return (exprs.STR, p.STRING)

    def error(self, p):
        if p:
            print(f"Syntax error at ({Runtime.lineno}, {p.index})")
            # Just discard the token and tell the parser it's okay.
            self.errok()
        else:
            print("Syntax error at EOF")