import argparse
import os
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile


def parse_args():
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("--compiler", default="compiler", help="compiler executable")
    parser.add_argument("--program", required=True, help="test program")
    parser.add_argument("--input", default="", help="test input")
    parser.add_argument("--output", required=True, help="test output")
    parser.add_argument("--run", action="store_true", help="run output file after the compilation")
    parser.add_argument("compiler_args", nargs="*", help="additional compiler arguments")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    with tempfile.TemporaryDirectory() as temp_dir:
        compiler_output = os.path.join(temp_dir, "output")
        cmd = [args.compiler, args.program, "--output", compiler_output]
        if args.run:
            cmd.append("--compile")
        if args.compiler_args:
            cmd.extend(args.compiler_args)
        print("Run compiler command:", shlex.join(cmd))
        subprocess.check_call(cmd, timeout=10)
        actual_output = ""
        if args.run:
            print("Run compiled executable:", compiler_output)
            input_text = Path(args.input).read_text() if args.input else None
            cp = subprocess.run([compiler_output], input=input_text, capture_output=True, text=True, timeout=10)
            actual_output = cp.stdout.strip()
        else:
            actual_output = Path(compiler_output).read_text()
        actual_output = actual_output.strip()
        expected_output = Path(args.output).read_text().strip()
        print("---", "Actual output:", actual_output, "---", "Expected output:", expected_output, "---", sep="\n")
        if actual_output == expected_output:
            print("Verdict: PASS")
        else:
            print("Verdict: FAIL")
            return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
