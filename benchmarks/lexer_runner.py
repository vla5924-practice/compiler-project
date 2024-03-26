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
from generators import lexer_generator

CLI_PATH = "../compiler/build/bin/Release/compiler.exe"


def parse_args():

    parser = ArgumentParser()
    parser.add_argument('--max-word-count', type=int, help="Maximum words in test file", default=1000000)
    parser.add_argument('--words-multiplier', type=int, help="Step for files", default=10)
    parser.add_argument('-o', '--output', type=str,help="Output folder", default="results")
    # add generate argument

    return parser.parse_args()


def main():
    args = parse_args()
    os.makedirs(args.output, exist_ok=True)
    test_files_folder = os.path.join(args.output, "test_files")
    os.makedirs(test_files_folder, exist_ok=True)
    compiler_logs = os.path.join(args.output, "compiler_logs")
    os.makedirs(compiler_logs, exist_ok=True)

    tests_paths = lexer_generator(args.words_count, args.words_multiplier, test_files_folder)

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
