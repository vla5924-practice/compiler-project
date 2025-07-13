# Основной класс `LLVMIRGenerator` для дерева операций

В программной реализации генератор промежуточного представления LLVM IR
представлен в виде класса `LLVMIRGenerator`. Содержательная часть
объявления класса выглядит следующим образом:

``` c++
class LLVMIRGenerator {
    void visit(const optree::Operation::Ptr &op);
    void visitBody(const optree::Operation::Ptr &op);

    void visit(const optree::ModuleOp &op);
    void visit(const optree::FunctionOp &op);
    void visit(const optree::ArithBinaryOp &op);
    // ...

  public:
    explicit LLVMIRGenerator(const std::string &moduleName);
    void process(const optree::Program &program);
    void dumpToFile(const std::string &filename) const;
};
```

Основным методом, выполняющим непосредственно генерацию кода, является
`process`. Этот метод принимает дерево операций, полученное в процессе
компиляции. Построенный код может быть в дальнейшем сохранен в файл с
помощью метода `dumpToFile`.

Принцип построения кода состоит в обходе синтаксического дерева и
добавления инструкций с помощью соответствующего модуля из состава
фреймворка LLVM. Обход дерева производится рекурсивно от корня и
начинается внутри метода `process`. Методы, отвечающие за обработку
узлов дерева различных типов, называются `visit`.

[_Назад_](README.md)
