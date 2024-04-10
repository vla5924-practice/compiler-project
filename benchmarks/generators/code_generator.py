import math
import os
import random

from .code_constructions import UNNAMED_BODYS
from .lexems import IDENTIFICATORS_KEYS

types = ["float", "int", "str"]

def generate_function(function_count: int) -> str:
    result = ""
    for i in range(function_count):
        function_name = random.choice(IDENTIFICATORS_KEYS) + f"_{i}"
        return_type = random.choice(types)
        arguments = ", ".join([
            f"{indetificator}: {random.choice(types)}"
            for indetificator in random.sample(IDENTIFICATORS_KEYS, 3)
        ])
        result += f"def {function_name} ({arguments}) -> {return_type}:\n"
        for _ in range(random.randrange(1, 5)):
            result += random.choice(UNNAMED_BODYS)
        result += "\n"

    return result

def parser_generator(function_count: int, output: str):
    function_count = [int(math.pow(10, i)) for i in range(0, len(str(function_count)))]
    for i in function_count:
        file_name = os.path.join(output, f"{i}.py")
        with open(file_name, 'w') as parser_file:
            parser_file.write(generate_function(i))
