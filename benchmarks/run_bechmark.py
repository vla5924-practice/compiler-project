from argparse import ArgumentParser
import random
import os
import tokenize
import time
import subprocess
from enum import Enum
import math
import json
import pandas

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


CLI_PATH = "../compiler/build/bin/Release/compiler.exe"


def parse_args():

    parser = ArgumentParser()
    parser.add_argument('--max-word-count', type=int, help="Maximum words in test file", default=1000000)
    parser.add_argument('--words-multiplier', type=int, help="Step for files", default=10)
    parser.add_argument('-o', '--output', type=str,help="Output folder", default="results")
    # add generate argument

    return parser.parse_args()


def generate_files(output, word_count):
    words = list(operators.keys()) + list(keywords.keys())

    with open(output, "w") as py_file:
        for i in range(word_count):
            py_file.writelines(str(words[random.randint(0, len(words) - 1)]) + ' ')

def main():
    args = parse_args()
    os.makedirs(args.output, exist_ok=True)
    test_files_folder = os.path.join(args.output, "test_files")
    os.makedirs(test_files_folder, exist_ok=True)
    compiler_logs = os.path.join(args.output, "compiler_logs")
    os.makedirs(compiler_logs, exist_ok=True)

    words_counts = [int(math.pow(args.words_multiplier, i)) for i in range(0, len(str(args.max_word_count)))]
    # print(words_counts)
    tests_paths = []
    for value in words_counts:
        tests_paths.append(os.path.join(test_files_folder, f"{value}.py"))
        generate_files(tests_paths[-1], value)



    tokenize_times = {}
    output = {}
    for filename in tests_paths:
        with open(filename, 'rb') as test_file:
            start = time.perf_counter()
            x = list(tokenize.tokenize(test_file.__next__))
            end = time.perf_counter()
            tokenize_times[int(os.path.basename(filename)[:-3])] = float(end - start)

    print(tokenize_times)

    data_frame = pandas.DataFrame.from_dict(tokenize_times, orient='index')
    data_frame.to_csv(
        os.path.join(args.output, "tokenize_times.csv")
    )

    c_times = {} # parse c++ logs
    for filename in tests_paths:
        subprocess.run(
            CLI_PATH + 
            f" -l {os.path.join(compiler_logs, os.path.basename(filename)[:-3])}.txt -v -O --last-module lexer --times " + 
            filename
        )


main()
