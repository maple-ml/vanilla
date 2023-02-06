from objects import exprs
from sly import Lexer

class GenLexer(Lexer):
    def __init__(self):
        self.nesting_level = 0

    ignore = " \t;"
    literals = {"=", "{", "}", "*", "(", ")", ",", ":", "[", "]"}

    tokens = {NAME, NUMBER, STRING, HEX}

    NAME = r"[a-zA-Z_\&][a-zA-Z0-9_\*\:\&]*"
    STRING = r"\".*?\""

    @_(r"""("[^"\\]*(\\.[^"\\]*)*"|'[^'\\]*(\\.[^'\\]*)*')""")
    def STRING(self, t):
        t.value = str(self.remove_quotes(t.value))
        return t

    @_(r"0[xX][0-9a-fA-F]+")
    def HEX(self, t):
        t.value = int(t.value, 16)
        return t

    @_(r"\d+")
    def NUMBER(self, t):
        t.value = int(t.value)
        return t

    @_(r"//.*$")
    def COMMENT(self, t):
        pass

    @_(r'\n+')
    def NEWLINE(self, t):
        self.lineno += len(t.value)

    def remove_quotes(self, text: str):
        if text.startswith('"') or text.startswith("'"):
            return text[1:-1]
        return text