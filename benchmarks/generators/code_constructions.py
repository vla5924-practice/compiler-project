ARITHMETIC_FUNCTION = """
    a: int = 1 + 2
    b: float = 3.0
    c: int = a + b
    z: float = a + b
    x: float = c + z
    y: float = (3.0 + 2) + a + b * c - 2 + 4 * a + c / z
"""
IF_FUNCTION = """
    a: int = 1
    b: int = 2
    if a == b:
        c: float = a + b
    else:
        c: int = a + b
"""

WHILE_FUNCTION = """
    i: int = 2
    sum: int = 0
    float_sum: float = 3.0
    while i < 34:
        sum += i
        float_sum += i
"""

GCD_FUNCTION = """
    a: int = 4
    b: int = 10000
    temp: int = 0
    if a > b:  # define the if condition 
        temp = b 
    else: 
        temp = a 
    i: int = 1
    gcd: int = 0
    while i < temp + 1:
        if a == i: 
            gcd = i
"""

UNNAMED_BODYS = [
    ARITHMETIC_FUNCTION,
    IF_FUNCTION,
    WHILE_FUNCTION,
    GCD_FUNCTION
]
