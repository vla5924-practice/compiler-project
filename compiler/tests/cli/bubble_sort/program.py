def bubble_sort(values: list[int], n: int) -> None:
    for i in range(n):
        for j in range(0, n - i - 1):
            if values[j] > values[j + 1]:
                temp: int = values[j]
                values[j] = values[j + 1]
                values[j + 1] = temp
    return

def sort_list_static() -> None:
    values: list[int] = [1, 5, 2, 8]
    bubble_sort(values, 4)
    for v in values:
        print(v, " ")
    print("\n")
    return

def sort_list_dynamic() -> None:
    n: int = input()
    values: list[int] = [0] * n
    for i in range(n):
        v: int = input()
        values[i] = v
    bubble_sort(values, n)
    for i in range(n):
        print(values[i], " ")
    print("\n")
    return

def main() -> None:
    sort_list_static()
    sort_list_dynamic()
    return
