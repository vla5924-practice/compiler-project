# Инфраструктура для тестирования лексического и синтаксического анализатора

Были реализованы генераторы тестовых файлов для лексических и
синтаксических анализаторов:

1.  **Генератор для лексических анализаторов**. Результатом его работы
    являются файлы с заданным количеством лексем. Лексемы выбираются
    случайным образом из множества, которое поддерживает язык.
2.  **Генератор для синтаксических анализаторов**. Результатом его
    работы являются файлы с заданным количеством функций. Функции имеют:
    1.  сигнатуру с случайными именами, именами переменных и типами;
    2.  тела функций, которые представляют собой объединение нескольких
        тел функции, заранее заданных (список возможных тел расширяем).

В данной работе рассматриваются лексический и синтаксический генераторы
– **flex** и **bison**, соответственно.

**Flex** – это инструмент для лексического анализа, который может
использоваться для выделения из исходного текста определенных строк
заранее заданным способом.

**Bison** – это инструмент для синтаксического разбора; он читает текст
и может использоваться для конвертирования последовательности слов в
структурированный формат для дальнейшей обработки.

На вход генераторы принимают правила выделения лексем, а на выходе даёт
код анализатора, в виде функции на языке Си. Общая структура для
описания генераторов:

1.  блок определений – содержит стартовые значения и определения;

2.  блок правил – выражения и соответствующие им действия;

3.  блок кода на С – пользовательский код.

Определения и правила описываются при помощи одного из вариантов
расширенной формы Бекуса-Наура.

Пример описания лексического генератора (flex) для целых чисел и чисел с
плавающей точкой:

<table>
<colgroup>
<col style="width: 31%" />
<col style="width: 20%" />
<col style="width: 48%" />
</colgroup>
<thead>
<tr>
<th colspan="3" style="text-align: center;"><strong>Блок
определений</strong></th>
</tr>
</thead>
<tbody>
<tr>
<td colspan="2" style="text-align: left;">Нетерминальные
символы(идентификаторы)</td>
<td style="text-align: left;">Выражение</td>
</tr>
<tr>
<td colspan="2" style="text-align: left;"><p>DIGIT</p>
<p>IntegerLiteral</p>
<p>FloatLiteral</p></td>
<td style="text-align: left;"><p>[0-9]</p>
<p>{DIGIT}+</p>
<p>{DIGIT}+"."{DIGIT}*</p></td>
</tr>
<tr>
<td colspan="3" style="text-align: center;"><strong>Блок
правил</strong></td>
</tr>
<tr>
<td style="text-align: left;">Правило</td>
<td colspan="2" style="text-align: left;">Обработчик</td>
</tr>
<tr>
<td style="text-align: left;"><p>{IntegerLiteral}</p>
<p>{FloatLiteral}</p></td>
<td colspan="2" style="text-align: left;"><p>int int_literal =
atoi(yytext); printf("%d\n“, int_literal);</p>
<p>float fl_literal = atof(yytext); printf("%f\n“, fl_literal);</p></td>
</tr>
<tr>
<td colspan="3" style="text-align: center;"><strong>Блок
пользовательского кода</strong></td>
</tr>
<tr>
<td colspan="3" style="text-align: left;"><p>void main(int argc, char
**argv)</p>
<p>{</p>
<p>++argv, --argc;</p>
<p>if ( argc &gt; 0 )</p>
<p>yyin = fopen( argv[0], "r" );</p>
<p>else</p>
<p>yyin = stdin;</p>
<p>yylex();</p>
<p>}</p></td>
</tr>
</tbody>
</table>

Пример описания синтаксического генератора (bison) для целых чисел и
чисел с плавающей точкой:

<table>
<colgroup>
<col style="width: 46%" />
<col style="width: 53%" />
</colgroup>
<thead>
<tr>
<th colspan="2" style="text-align: center;"><strong>Блок
определений</strong></th>
</tr>
</thead>
<tbody>
<tr>
<td colspan="2" style="text-align: left;">Лексемы</td>
</tr>
<tr>
<td colspan="2" style="text-align: left;"><p>%token INTEGER_LITERAL
FLOAT_LITERAL // ожидаемые токены</p>
<p>%start input // Входное правило для анализатора</p></td>
</tr>
<tr>
<td colspan="2" style="text-align: center;"><strong>Блок
правил</strong></td>
</tr>
<tr>
<td style="text-align: left;">Правило</td>
<td style="text-align: left;">Обработчик</td>
</tr>
<tr>
<td style="text-align: left;"><p>%%</p>
<p>input: /* empty */</p>
<p>|</p>
<p>integer_literal input</p>
<p>|</p>
<p>float_literal input;</p>
<p>integer_literal: INTEGER_LITERAL</p>
<p>float_literal: FLOAT_LITERAL</p></td>
<td style="text-align: left;"><p>// Входное правило для анализатора</p>
<p>{printf("Current int: %d\n", $1);}</p>
<p>{printf("Current float: %f\n", fsum);}</p></td>
</tr>
</tbody>
</table>

Для связи двух генераторов между собой необходимо подключить
сгенерированный с помощью bison файл в описание для flex и в
обработчиках вернуть соответствующие токены.

[_Назад_](README.md)
