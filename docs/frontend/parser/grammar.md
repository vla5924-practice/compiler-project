# Описание грамматики

Для формального описания анализаторов составим контекстно-свободную
грамматику. Грамматика состоит из следующих наборов.

* Набор терминалов – атомарные символы языка.
* Набор нетерминалов – последовательность терминалов и нетерминалов,
представляющая конструкции языка.
* Набор правил подстановки, задающих компонентный состав конструкций.
Каждое правило подстановки имеет левую и правую части, разделяемые
символом «может быть». В левой части находится нетерминал, в правой
части – строка из набора терминалов и нетерминалов.

Понятие контекстно-свободной грамматики не зависит от нотации, которая
используется для ее записи. Обычно такие нотации называют метаязыками
(метаязык – язык для описания другого языка). В качестве метаязыка
выберем расширенную форму Бэкуса-Наура.

Рассмотрим правила для некоторых из парсеров. В правилах уже не
присутствуют терминалы, так как все они преобразованы в нетерминалы на
стадии лексического анализа. Фигурные скобки означают любое количество
повторений выражения, заключенного в них.

`ProgramRoot` – содержит нетерминал для обработки объявления функций языка.

```
ProgramRoot = {FunctionDefinition}
```

Парсер `ProgramRoot` показывает, что компилируемый язык является
функциональным, т.е. состоит только из объявлений и определений функций.

`FunctionDefinition` – обработчик определения функции. Правило:

```
FunctionDefinition = Def_KeyWord Identifier “(” FunctionArguments “)” Arrow Type: Indentation BranchRoot
```

Объявление функции, соответствует определению функций в языке Python, но
требует обязательного указания типов аргументов и возвращаемого типа.

`FunctionArguments` – обработчик для перечисления аргументов функции.

```
FunctionArguments = {VariableDeclaration}{Comma}[VariableDeclaration]
```

Каждый аргумент должен иметь явно указанный тип.

`BranchRoot` – обработчик тел функций и конструкций.

```
BranchRoot = {IfStatement | ForStatement | WhileStatement | Expression | VariableDeclaration | ReturnStatement}
```

Описывает конструкции, которые возможно встретить в теле функции или
других конструкций.

`VariableDeclaration` – объявление переменной.

```
VariableDeclaration = Identifier Colon Type OperatorEqual Expression
```

Объявление переменной должно содержать явный тип.

`IfStatement` – обработчик условной конструкции if. Правило:

```
IfStatement = If_KeyWord Logical_Expression Colon Indentation BranchRoot
([Else_KeyWord Colon Indentation BranchRoot] | ({Elif_KeyWord
Expression Colon Indentation BranchRoot} [Else_KeyWord Colon
Indentation BranchRoot]))
```

Описывает конструкции для условного оператора if и его вариаций с else и
else+if (elif) частями.

`Expression` – обработчик арифметических или логических выражений.
    Правила:

```
Expression = ArithmeticOp | LogicalOp | ListExpression

ArithmeticOp = Term {(OperatorPlus | OperatorMinus) Term}

Term = Factor {(OperatorMultiplication | OperatorDivision) Factor}

Factor = IntegerLiteral | FloatLiteral | Identifier | “(”
ArithmeticOp “)”

ComparisonOp = CompTerm {CompSign CompTerm}

CompSign = OperatorGreater | OperatorGreaterEqual | OperatorLesser |
OperatorLesserEqual | OperatorEqual | OperatorNotEqual

CompTerm = ArithmeticOp | “(” ComparisonOp “)”

LogicalOp = LogicalTerm {(And_Keyword | Or_Keyword) LogicalTerm}

LogicalTerm = True_Keyword | False_Keyword | ComparisonOp | “(”
LogicalOp “)”

ListExpression = ListStaticSize | ListDynamicSize | ListDereference

ListStaticSize = “[” {(IntegerLiteral | FloatLiteral) Comma} “]”

ListDynamicSize = “[” IntegerLiteral | FloatLiteral “]”
OperatorMultiplication IntegerLiteral | Identifier

ListDereference = Identifier “[” ArithmeticOp “]”
```

Выражения могут быть арифметическими или логическими. Тип выражения
определяется типом его операндов или специальными операндами (например
true/false для логических выражений). Специальным выражением является
определение

`ForStatement` – обработчик цикла с известным числом итераций.

```
ForStatement = For_KeyWord Identifier {Comma Identifier} In_KeyWord
(Range_Function | Enumerate_Funсtion | Identifier) Colon Indentation
BranchRoot
```

`WhileStatement` – обработчик цикла с неизвестным числом итераций.

```
WhileStatement = While_KeyWord Expression Colon Indentation BranchRoot
```

`ReturnStatement` – конструкция возврата из функции.

```
ReturnStatement = Return_KeyWord Expression
```

[_Назад_](README.md)
