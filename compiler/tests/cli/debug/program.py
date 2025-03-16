def foo(x: int) -> float:
    z: int = 5 - x
    return 3.5 / z

def main() -> None:
    x: int = 1
    y: float
    y = input()
    print("hi")
    if y > 1.5:
        l1: list[int] = [1, 2, 3]
        l2: list[float] = [-4] * x
        z: float = foo(x) * (2.5 + l2[x])
    else:
        y = 6
        while x < 10:
            x = x + 1
    for i in range(10):
        if True:
        #     break
        # elif False:
        #     continue
        # else:
            pass
    print(y)
    return
