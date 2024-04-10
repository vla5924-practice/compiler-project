import os
from argparse import ArgumentParser

from generators.code_generator import parser_generator
from generators.lexer_generator import simple_lexems_generator


def parse_args():
    parser = ArgumentParser("generate_files.py")

    parser.add_argument("--output", default="test_files")
    parser.add_argument("--lexer", action="store_true")
    parser.add_argument('--max-word-count', type=int, help="Maximum words in test file", default=1000000)
    parser.add_argument('--words-multiplier', type=int, help="Step for files", default=10)
    parser.add_argument("--parser", action="store_true")
    parser.add_argument('--max-function-count', type=int, help="", default=100000)

    return parser.parse_args()


def main():
    args = parse_args()
    os.makedirs(args.output, exist_ok=True)
    if (args.lexer):
        lexer_test_path = os.path.join(args.output, "lexer_test_files")
        os.makedirs(lexer_test_path, exist_ok=True)
        simple_lexems_generator(args.max_word_count, args.words_multiplier, lexer_test_path)

    if (args.parser):
        parser_test_path = os.path.join(args.output, "parser_test_files")
        os.makedirs(parser_test_path, exist_ok=True)
        parser_generator(args.max_function_count, parser_test_path)


if __name__ == "__main__":
    main()
