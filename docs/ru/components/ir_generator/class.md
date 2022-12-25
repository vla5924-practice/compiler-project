# Основной класс `IRGenerator`

В программной реализации генератор промежуточного представления LLVM IR представлен в виде класса `IRGenerator`. Содержательная часть объявления класса выглядит следующим образом:

```cpp
class IRGenerator {
  public:
    IRGenerator(const std::string &moduleName);
    void process(const SyntaxTree &tree);
    void writeToFile(const std::string &filename);

  private:
    llvm::Value *visitNode(Node::Ptr node);
    llvm::Value *visitBinaryOperation(Node *node);
    llvm::Value *visitFloatingPointLiteralValue(Node *node);
    llvm::Value *visitFunctionCall(Node *node);
    // ...

    void processNode(Node::Ptr node);
    void processExpression(Node *node);
    void processFunctionDefinition(Node *node);
    void processIfStatement(Node *node);
    // ...
};
```

Основным методом, выполняющим непосредственно генерацию кода, является `process`. Этот метод принимает завершенное или оптимизированное синтаксическое дерево. Построенный код может быть в дальнейшем сохранен в файл с помощью метода `writeToFile`.

Принцип построения кода состоит в обходе синтаксического дерева и добавления инструкций с помощью соответствующего модуля из состава фреймворка LLVM. Обход дерева производится рекурсивно от корня и начинается внутри метода `process`. Как видно в представленном фрагменте кода, часть методов, отвечающих за обработку узлов дерева различных типов, имеют в названии префикс _process_ или _visit_. Особенность методов _visit_ состоит в том, что они в результате просмотра узла возвращают вычислимое значение и используются для обхода арифметико-логических выражений. Методы же, имеющие названия, начинающиеся со слова _process_, обходят узлы, которые по своей сути не являются частями выражений (например, конструкции с циклами и заголовки функций).

[_Назад_](README.md)
