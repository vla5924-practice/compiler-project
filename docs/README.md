# Документация - `compiler-project`

## О проекте

* [Основные положения](overview/README.md)
* [Компилируемый язык](language/README.md)

## Внутренние представления программы

* [Синтаксическое дерево](./representations/ast/README.md)
* [Дерево опреаций](./representations/optree/README.md)

## Компоненты проекта

* [Фронтенд (`frontend`)](./frontend/README.md)
* [Бэкенд на базе синтаксического дерева (`backend/ast`)](./backend_ast/README.md)
* [Бэкенд на базе дерева операций (`backend/optree`)](./backend_optree/README.md)
* [Генератор кода LLVM IR](./codegen/README.md)
* [Консольное приложение](./cli/README.md)

## Сравнения производительности

* [Производительность компиляции](./benchmarking/compilation/README.md)
* [Производительность исполнения](./benchmarking/execution/README.md)

## См. также

* [Список используемой литературы](./references.md)
* [Репозиторий GitHub](./repository.md)

> «Таким образом, был разработан и реализован компилятор».
