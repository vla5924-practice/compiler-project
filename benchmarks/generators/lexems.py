from enum import Enum


class Keyword(Enum):
    BOOL = "bool",
    FALSE = "False",
    INT = "int",
    FLOAT = "float",
    STR = "str",
    IF = "if",
    ELSE = "else",
    ELIF = "elif",
    RANGE = "range",
    WHILE = "while",
    FOR = "for",
    BREAK = "break",
    IMPORT = "import",
    CONTINUE = "continue",
    DEFENITION = "def",
    RETURN = "return",
    OR = "or",
    AND = "and",
    NOT = "not",
    IN = "in",
    TRUE = "True",
    NONE = "None"

KEYWODRS = {
    "bool": Keyword.BOOL,
    "False": Keyword.FALSE,
    "int": Keyword.INT,
    "float": Keyword.FLOAT,
    "str": Keyword.STR,
    "if": Keyword.IF,
    "else": Keyword.ELSE,
    "elif": Keyword.ELIF,
    "range": Keyword.RANGE,
    "while": Keyword.WHILE,
    "for": Keyword.FOR,
    "break": Keyword.BREAK,
    "import": Keyword.IMPORT,
    "continue": Keyword.CONTINUE,
    "def": Keyword.DEFENITION,
    "return": Keyword.RETURN,
    "or": Keyword.OR,
    "and": Keyword.AND,
    "not": Keyword.NOT,
    "in": Keyword.IN,
    "True": Keyword.TRUE,
    "None": Keyword.NONE,
}

class Operator(Enum):
    Mod = "%"
    Dot = "."
    RectRightBrace = "]"
    Comma = ","
    Assign = "="
    Add = "+"
    Sub = "-"
    Mult = "*"
    Div = "/"
    Equal = "=="
    NotEqual = "!="
    Less = "<"
    Greater = ">"
    LessEqual = "<="
    GreaterEqual = ">="
    LeftBrace = "("
    RightBrace = ")"
    RectLeftBrace = "["

OPERATORS = {
    "%": Operator.Mod, 
    ".": Operator.Dot,
    # "]": Operator.RectRightBrace,
    ",": Operator.Comma, 
    "=": Operator.Assign,
    "+": Operator.Add,
    "-": Operator.Sub, 
    "*": Operator.Mult,
    "/": Operator.Div,
    "==": Operator.Equal, 
    "!=": Operator.NotEqual,
    "<": Operator.Less,
    ">": Operator.Greater, 
    "<=": Operator.LessEqual,
    ">=": Operator.GreaterEqual,
    # "(": Operator.LeftBrace, 
    # ")": Operator.RightBrace,
    # "[": Operator.RectLeftBrace
}

class TokenType(Enum):
    Keyword = "keyword"
    Identifier = "identifier"
    Operator = "operator"
    Special = "special"
    IntegerLiteral = "integer_literal"
    FloatingPointLiteral = "floating_point_literal"
    StringLiteral = "string_literal"


INDETIFICATORS = {
    "Z": TokenType.Identifier,
    "V": TokenType.Identifier,
    "Za_Pobedy": TokenType.Identifier,
    "x": TokenType.Identifier,
    "y": TokenType.Identifier,
    "i": TokenType.Identifier,
    "foo": TokenType.Identifier,
    "bar": TokenType.Identifier,
}
IDENTIFICATORS_KEYS = list(INDETIFICATORS.keys())


LITERALS = {
    "0": TokenType.IntegerLiteral,
    "1": TokenType.IntegerLiteral,
    "23122": TokenType.IntegerLiteral,
    "42.24": TokenType.FloatingPointLiteral,
    "1.0": TokenType.FloatingPointLiteral,
    "0.0": TokenType.FloatingPointLiteral,
    "0.3232": TokenType.FloatingPointLiteral,
    "232.0": TokenType.FloatingPointLiteral,
    "\"Hello\"": TokenType.StringLiteral,
    # "\'Quote1\'": TokenType.StringLiteral, ## can not recognize such strings
    "\"Quote2\"": TokenType.StringLiteral,
}
