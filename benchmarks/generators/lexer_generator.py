import math
import os
import random
from typing import List

from .lexems import INDETIFICATORS, KEYWODRS, LITERALS, OPERATORS


def simple_lexems_generator(words_count: int, words_multiplier: int, output_folder: str) -> List[str]:

    lexems = {**OPERATORS, **KEYWODRS, **INDETIFICATORS, **LITERALS}

    lexems_keys = list(lexems.keys())

    words_counts = [int(math.pow(words_multiplier, i)) for i in range(0, len(str(words_count)))]
    tests_paths = []
    for value in words_counts:
        generated_file_path = os.path.join(output_folder, f"{value}.py")
        tests_paths.append(generated_file_path)
        with open(generated_file_path, "w") as generated_file:
            for i in range(value):
                random_lexem = lexems_keys[random.randint(0, len(lexems) - 1)]
                generated_file.writelines(random_lexem + ' ')


    return tests_paths
