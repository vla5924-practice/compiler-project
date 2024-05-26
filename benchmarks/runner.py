import ast
import csv
import glob
import os
import re
import subprocess
import time
import tokenize
from argparse import ArgumentParser
from typing import Dict

CLI_PATH = "/workspaces/compiler-project/build/bin/compiler"
FLEX_PATH = "/workspaces/compiler-project/benchmarks/flex/lexer"
YACC_PATH = "/workspaces/compiler-project/benchmarks/yacc/parser"


def run_compiler(module: str, files: str, log_folder: str, compiler_path: str = CLI_PATH) -> Dict:
        times = {
            "measured" : f"{module.lower()} (ms)",
        }
        for filename in files:
            print(f"{module} proccesing {os.path.basename(filename)}")
            log_path = os.path.join(log_folder, os.path.basename(filename)) + ".log"
            print(f"{compiler_path} -O {filename} --time --stop-after {module}")
            with open(log_path, 'w') as log_file:
                result = subprocess.run(
                    f"{compiler_path} -O {filename} --time --stop-after {module}", 
                    capture_output = True, text = True, shell=True)
                log_file.write(result.stderr)
            parsed = parse_compiler_log(log_path)
            times[os.path.basename(filename)] = round(parsed[f"{module.lower()} (ms)"], 6)
        return times


def run_flex(module: str, files: str, log_folder: str, compiler_path: str = CLI_PATH) -> Dict:
        times = {
            "measured" : f"{module.lower()} (ms)",
        }
        for filename in files:
            print(f"{module} proccesing {os.path.basename(filename)}")
            log_path = os.path.join(log_folder, os.path.basename(filename)) + ".log"
            print(f"{compiler_path} {os.path.basename(filename)[:-3]} {filename} {log_path}")
            # with open(log_path, 'w') as log_file:
            result = subprocess.run(
                    f"{compiler_path} {os.path.basename(filename)[:-3]} {filename} {log_path}", 
                    capture_output = True, text = True, shell=True)
                # log_file.write(result.stderr)
            parsed = parse_compiler_log(log_path)
            print(parsed)
            times[os.path.basename(filename)] = round(parsed[f"{module.lower()} (ms)"], 6)
        return times

def run_yacc(module: str, files: str, log_folder: str, compiler_path: str = CLI_PATH) -> Dict:
        times = {
            "measured" : f"{module.lower()} (ms)",
        }
        for filename in files:
            print(f"{module} proccesing {os.path.basename(filename)}")
            log_path = os.path.join(log_folder, os.path.basename(filename)) + ".log"
            print(f"{compiler_path} {os.path.basename(filename)[:-3]} {filename} {log_path}")
            # with open(log_path, 'w') as log_file:
            result = subprocess.run(
                    f"{compiler_path} {filename}", 
                    capture_output = True, text = True, shell=True)
            print(result)
                # log_file.write(result.stderr)
            parsed = parse_compiler_log_yacc(result.stdout)
            print(parsed)
            times[os.path.basename(filename)] = round(parsed[f"{module.lower()} (ms)"], 6)
        return times

def parse_compiler_log_yacc(log_path: str):

    finded_list = re.findall("([a-z]*) (Elapsed time:) (\d.\d*)", log_path)
    print(finded_list)
    return {
        f"{finded[0]} (ms)": float(finded[2])
        for finded in finded_list
    }

def parse_compiler_log(log_path: str):
    with open(log_path, "r") as log_file:
        log = log_file.read()
    finded_list = re.findall("([a-z]*) (Elapsed time:) (\d.\d*)", log)
    print(finded_list)
    return {
        f"{finded[0]} (ms)": float(finded[2])
        for finded in finded_list
    }

def parse_args():

    parser = ArgumentParser()
    parser.add_argument('--python-lexer', action="store_true")
    parser.add_argument('--lexer-flex', action="store_true")
    parser.add_argument('--lexer', action="store_true")
    parser.add_argument('--lexer-test-files', type=str)

    parser.add_argument('--python-parser', action="store_true")
    parser.add_argument('--parser', action="store_true")
    parser.add_argument('--parser-yacc', action="store_true")
    parser.add_argument('--parser-test-files', type=str)

    parser.add_argument('-o', '--output', type=str,help="Output folder", default="results")

    return parser.parse_args()


def main():
    args = parse_args()
    os.makedirs(args.output, exist_ok=True)

    compiler_logs = os.path.join(args.output, "compiler_logs")
    os.makedirs(compiler_logs, exist_ok=True)

    result_times = []

    # lexer section
    if (args.python_lexer or args.lexer):
        if (not os.path.exists(args.lexer_test_files)):
                raise RuntimeError(f"Can not find lexer_test_files {args.lexer_test_files}")
        lexer_files = sorted(glob.glob(os.path.join(args.lexer_test_files, "*.py")))

    if (args.python_parser or args.parser or args.parser_yacc):
        if (not os.path.exists(args.parser_test_files)):
                raise RuntimeError(f"Can not find parser_test_files {args.parser_test_files}")
        parser_files = sorted(glob.glob(os.path.join(args.parser_test_files, "*.py")))


    if (args.python_lexer):
        times = {
            "measured" : "python lexer (ms)",
        }
        for filename in lexer_files:
            print(f"Python lexer proccesing {os.path.basename(filename)}")
            with open(filename, 'rb') as test_file:
                start = time.perf_counter()
                x = list(tokenize.tokenize(test_file.__next__))
                end = time.perf_counter()
                times[os.path.basename(filename)] = round((end - start) * 1000, 6)
        result_times.append(times)

    if (args.lexer):
        result_times.append(run_compiler("lexer", lexer_files, compiler_logs))

    if (args.lexer_flex):
        result_times.append(run_flex("lexerflex", lexer_files, compiler_logs, FLEX_PATH))

    # parser section
    if (args.python_parser):
        times = {
            "measured" : "python parser (ms)",
        }
        for filename in parser_files:
            print(f"Python parser proccesing {os.path.basename(filename)}")
            with open(filename, 'rb') as test_file:
                start = time.perf_counter()
                x = ast.parse(source=test_file.read(), filename="sss.py")
                end = time.perf_counter()
                times[os.path.basename(filename)] = round((end - start) * 1000, 6)
        result_times.append(times)

    if (args.parser):
        result_times.append(run_compiler("parser", parser_files, compiler_logs))
    
    if (args.parser_yacc):
        result_times.append(run_yacc("lex", parser_files, compiler_logs, YACC_PATH))

    with open(os.path.join(args.output, "result_times.csv"), 'w') as csvfile:
        writer = csv.DictWriter(
            csvfile, 
            fieldnames=[
                "measured", 
                *map(os.path.basename, parser_files),
            ])
        writer.writeheader() 
        writer.writerows(result_times) 


if __name__ == "__main__":
    main()
