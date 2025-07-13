# Контекст для анализатора

Для реализации модуля создан класс, содержащий вспомогательные структуры данных:

```cpp
struct SemantizerContext {
    std::list<VariablesTable *> variables;
    FunctionsTable &functions;
    TypeId currentFunctionType;

    SemantizerContext(FunctionsTable &functions_);
    TypeId findVariable(const Node::Ptr &node);
};
```

Поля класса:
* `variables` – двусвязный список, содержащий в себе указатели на таблицы переменных;
* `functions` – ссылка на таблицу функций;
* `currentFunctionType` – тип, содержащий возвращаемое значение анализируемой в данной момент функции.

[_Назад_](README.md)
