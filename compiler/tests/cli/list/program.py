def list_static() -> None:
    x: list[float] = [1, 2.3, 4, 5.6]
    n: int = 2
    print(x[0], " ", x[n * 2 - 1], " ")
    return


def list_dynamic() -> None:
    n: int = 3
    x: list[int] = [4] * (n + 1) * 2
    print(x[6], " ", 9 * x[n] / 4)
    return


def main() -> None:
    list_static()
    list_dynamic()
    return
