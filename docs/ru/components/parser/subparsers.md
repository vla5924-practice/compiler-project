# Список функций-парсеров

В таблице ниже приведен список реализованных парсеров в алфавитном порядке, а также их описание и соответствие с элементами перечисления `NodeType`.

Название парсера | Значение NodeType | Описание (назначение)
--- | --- | ---
`parseBranchRoot` | `BranchRoot` | Разбор очередного блока кода
`parseElifStatement` | `ElifStatement` | Разбор дополнительных веток в ветвлении
`parseElseStatement` | `ElseStatement` | Разбор альтернативной ветки в ветвлении
`parseExpression` | `Expression` | Разбор арифметико-логических выражений
`parseFunctionArguments` | `FunctionArguments` | Разбор списка аргументов функции
`parseFunctionDefiniton` | `FunctionDefiniton` | Разбор объявления функции
`parseIfStatement` | `IfStatement` | Разбор основной ветки в ветвлении
`parseProgramRoot` | `ProgramRoot` | Разбор от начала программы
`parseReturnStatement` | `ReturnStatement` | Разбор инструкции возврата значения
`parseVariableDeclaration` | `VariableDeclaration` | Разбор объявления переменной
`parseWhileStatement` | `WhileStatement` | Разбор циклической конструкции

[_Назад_](README.md)
