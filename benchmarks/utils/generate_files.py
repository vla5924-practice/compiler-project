import random
from argparse import ArgumentParser
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



keywords = {
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

operators = {
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

def parse_args():

    parser = ArgumentParser()

    parser.add_argument('-o', '--output', type=str, required=True)
    parser.add_argument('--word-count', type=int, required=True)

    return parser.parse_args()

def main():
    args = parse_args()

    words = list(operators.keys()) + list(keywords.keys())

    with open(args.output, "w") as py_file:
        for i in range(args.word_count):
            py_file.writelines(str(words[random.randint(0, len(words) - 1)]) + ' ')


main()
