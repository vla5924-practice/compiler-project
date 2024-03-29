# Статическое приведение типов

Статическое приведение типов – преобразование значения одного типа в значение другого типа во время компиляции программы. Статическое приведение типов применяется к выражениям, содержащим вычисленные константные выражения. 

## Пример на уровне кода

До оптимизации:

```py
def main() -> None:
    x: int = 1
         y: float = 2 + x
```

После оптимизации:

```py
def main() -> None:
    x: int = 1
         y: float = 3.0
```


Во время стадии семантического анализа выполняется проверка типов и вставка конструкций для приведения типов. В совокупности с двумя предыдущими оптимизациями (распространением констант и их сверткой) получаем конечное значение выражения необходимого типа.

[_Назад_](README.md)
