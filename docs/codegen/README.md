# Генератор кода LLVM IR

Для упрощения трансляции кода в машиночитаемый двоичный файл на этапе разработки компилятора можно частично «передать» эту задачу фреймворку LLVM.

* [Актуальность использования LLVM](why_llvm.md)
* [Основной класс `IRGenerator` для синтаксического дерева](class_ast.md)
* [Основной класс `LLVMIRGenerator` для дерева операций](class_optree.md)
* [Особенности реализации функций ввода-вывода](io_implementation.md)
* [Пример работы генератора](example.md)

[_Назад_](../README.md)

```{toctree}
:hidden:

why_llvm.md
class_ast.md
class_optree.md
io_implementation.md
example.md
```
