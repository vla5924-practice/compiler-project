# Контекст для анализатора

Для реализации модуля создан класс, содержащий вспомогательные структуры данных:

```cpp
struct OptimizerContext {
    std::forward_list<VariablesTable *> variables;
    std::forward_list<std::unordered_map<std::string, VariableValue>> values;
    FunctionsTable &functions;
    Node::Ptr root;
};

using VariableValue = std::variant<long int, double>;
```

Поля класса:
* `variables` – односвязный список, содержащий в себе указатели на таблицы переменных;
* `values` – односвязный список, содержащий в себе ассоциативные контейнеры, где ключом является имя переменной, а значением объединение с безопасным типом;
* `functions` – ссылка на таблицу функций;
* `root` – умный указатель, содержащий в себе корень синтактического дерева.

[_Назад_](README.md)
