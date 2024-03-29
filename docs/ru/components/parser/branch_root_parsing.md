# Парсер для очередного блока кода

Парсер, выполняющий разбор очередного блока кода, интересен тем, что представляет собой некоторую композицию вызовов многих других парсеров в зависимости от того, начало какой языковой конструкции было обнаружено в списке токенов. Обобщенно алгоритм упомянутого парсера выглядит следующим образом:

1. **Проверка текущего уровня вложенности блока:**
   1. Текущий уровень вложенности принимается равным числу отступов в начале строки.
   2. Если текущий уровень вложенности строго больше последнего вычисленного значения уровня, то считается, что допущена синтаксическая ошибка. Парсер возбуждает исключение, но пытается продолжить работу.
   3. Если текущий уровень строго меньше последнего вычисленного, то считается, что код в текущей строке принадлежит одному из родительских блоков. Парсер перемещается вверх по дереву, пока не встретит узел типа «блок кода».
   4. Если текущий уровень равен последнему вычисленному значению, то считается, что код в текущей строке находится в том же блоке, что и код в предыдущей строке. Парсер продолжает работу.
2. **Обнаружение языковых конструкций:**
   1. Парсер получает очередной токен из списка.
   2. Производится попытка определения языковой конструкции в зависимости от типа и содержимого текущего токена:
      * если текущий токен – ключевое слово `if`, то в дерево добавляется узел типа «конструкция if» и вызывается парсер `parseIfStatement`;
      * если текущий токен – ключевое слово `while`, то в дерево добавляется узел типа «конструкция while» и вызывается парсер `parseWhileStatement`;
      * если текущий токен, а также два следующих за ним вместе образуют начало объявления переменной (то есть, первый токен является идентификатором, второй токен является специальной последовательностью «двоеточие», третий токен является идентификатором и одновременно именем типа), то в дерево добавляется узел «объявление переменной» и вызывается парсер `parseVariableDeclaration`;
      * если текущий токен – ключевое слово `elif` или ключевое слово `else`, то, в случае если текущий узел дерева является узлом типа «конструкция if», в дерево добавляется узел «конструкция elif» или «конструкция else» и вызывается соответствующий парсер (`parseElifStatement` или `parseElseStatement`), иначе считается, что допущена синтаксическая ошибка;
      * если текущий токен – ключевое слово `return`, то в дерево добавляется узел типа «конструкция return» и вызывается парсер `parseReturnStatement`;
      * если ни одна из вышеперечисленных проверок не оказалась истинной, считается, что далее в коде находится арифметико-логическое выражение; в дерево добавляется узел типа «выражение» и вызывается парсер `parseExpression`.

[_Назад_](README.md)
