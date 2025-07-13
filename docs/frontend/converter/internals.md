# Особенности реализации конвертера

Функции для перевода узла синаксического дерева конкретного вида можно
разделить на два вида:

- V-функции, названия которых начинаются со слова `visit`.
  Особенность V-функций состоит в том, что они в результате
  просмотра узла возвращают "вычисленное" значение и используются,
  как правило, для обхода арифметико-логических выражений.

- P-функции, названия которых начинаются со слова `process`. Такие
  функции обходят узлы, которые по своей сути не являются частями
  выражений (например, конструкции с циклами и заголовки функций).

P-функции конвертера

| **Название** | **NodeType** | **Назначение** |
|:---|:---|:---|
| `processProgramRoot` | `ProgramRoot` | Разбор от начала программы |
| `processFunctionDefinition` | `FunctionDefinition` | Разбор объявления функции |
| `processBranchRoot` | `BranchRoot` | Разбор очередного блока кода |
| `processVariableDeclaration` | `VariableDeclaration` | Разбор объявления переменной |
| `processReturnStatement` | `ReturnStatement` | Разбор возврата значения |
| `processWhileStatement` | `WhileStatement` | Разбор цикла с предусловием |
| `processIfStatement` | `IfStatement` | Разбор ветвления |
| `processForStatement` | `ForStatement` | Разбор цикла со счетчиком |

V-функции конвертера:

| **Название** | **NodeType** | **Назначение** |
|:---|:---|:---|
| `visitExpression` | `Expression` | Разбор арифметико-логических выражений |
| `visitIntegerLiteralValue` | `IntegerLiteralValue` | Разбор целочисленного литерала |
| `visitBooleanLiteralValue` | `BooleanLiteralValue` | Разбор булевого литерала |
| `visitFPLiteralValue` | `FPLiteralValue` | Разбор числового литерала с дробной частью |
| `visitStringLiteralValue` | `StringLiteralValue` | Разбор строкового литерала |
| `visitBinaryOperation` | `BinaryOperation` | Разбор бинарной операции |
| `visitVariableName` | `VariableName` | Разбор образения к переменной |
| `visitFunctionCall` | `FunctionCall` | Разбор вызова функции |
| `visitListAccessor` | `ListAccessor` | Разбор доступа к элементу списка |
| `visitUnaryOperation` | `UnaryOperation` | Разбор унарной операции |


Для удобства работы с деревом операций при реализации перечисленных
функций каждая из них принимает объект *контекста* в качестве аргумента.
Этот объект представлен структурой `ConverterContext`, и содержательная
часть ее объявления выглядит следующим образом.

```cpp
struct ConverterContext {
    struct LocalVariable {
        Value::Ptr value;
        // ...
    };
    Operation::Ptr op;
    unordered_map<string, Type::Ptr> functions;
    forward_list<unordered_map<string, LocalVariable>> variables;
    Builder builder;
    // ...
};
```

Таким образом, структура должна хранить в себе:

- операцию, внутрь которой должна выполняться дальнейшая вставка
  операций;
- список типов функций;
- список переменных с учетом вложенности блоков кода;
- объект класса, управляющего вставкой операций.

[_Назад_](README.md)
