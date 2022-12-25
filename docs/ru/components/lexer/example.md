# Пример работы анализатора

Рассмотрим работу лексического анализатора на примере.

Пусть на вход поступает строка

```py
    x: int = 1.0 + y
```

Разбиение этого выражения на токены представлено на схеме:

```mermaid
stateDiagram
    direction LR
    Indentation: ....
    state ":" as Colon
    Equals: =
    Number: 1.0
    Plus: +

    Indentation --> x
    x --> Colon
    Colon --> int
    int --> Equals
    Equals --> Number
    Number --> Plus
    Plus --> y

    note right of Indentation
        спец. послед-ть (отступ)
    end note
    note right of x
        идентификатор
    end note
    note right of Colon
        спец. послед-ть (двоеточие)
    end note
    note right of int
        идентификатор
    end note
    note right of Equals
        оператор
    end note
    note right of Number
        литерал
    end note
    note right of Plus
        оператор
    end note
    note right of y
        идентификатор
    end note
```

[_Назад_](README.md)
