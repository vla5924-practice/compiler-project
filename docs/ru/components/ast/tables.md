# Таблицы символов

В проекте таблица символов разделена на таблицу переменных и таблицу функций.

## Таблица функций

```cpp
struct Function {
    TypeId returnType;
    std::vector<TypeId> argumentsTypes;
    unsigned useCount = 0;
};

using FunctionsTable = std::map<std::string, Function>;
```

Сама таблица функций представляет ассоциативный контейнер, где ключом служит имя функции, а значением структура типа `Function`. 
Поля класса:
* `returnType` – тип значения, возвращаемого функцией;
* `argumentsTypes` – вектор, содержащий в себе типы параметров;
* `useCount` – счетчик, контролирующий количество использований функции.


## Таблица переменных

```
struct Variable {
    TypeId type;
    struct {
        bool modified = false;
    } attributes;

};

using VariablesTable = std::map<std::string, Variable>; 
```

Сама таблица переменных представляет ассоциативный контейнер, где ключом служит имя переменной, а значением структура типа `Variable`. 
Поля класса:
* `type` – тип переменной;
* `attributes` – структура, содержащая в себе атрибуты переменных.

[_Назад_](README.md)
