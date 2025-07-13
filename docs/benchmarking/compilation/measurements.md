# Проведенные эксперименты и результаты

Для сравнения между собой были выбраны:

1.  Python модули Tokenizer (лексический анализатор) и AST (синтаксический анализатор).
1.  Анализаторы, сгенерированные при помощи flex и bison.
2.  Модули lexer и parser представленного компилятора.

## Сравнение времени работы лексических анализаторов

<table>
<colgroup>
<col style="width: 18%" />
<col style="width: 25%" />
<col style="width: 8%" />
<col style="width: 9%" />
<col style="width: 9%" />
<col style="width: 9%" />
<col style="width: 9%" />
<col style="width: 9%" />
</colgroup>
<tbody>
<tr>
<td colspan="2">
<p>Количество лексем</p>
</td>
<td>100000</td>
<td>1000000</td>
<td>2000000</td>
<td>3000000</td>
<td>4000000</td>
<td>5000000</td>
</tr>
<tr>
<td rowspan="3">
<p>Лексические анализаторы</p>
</td>
<td>
<p>Tokenizer (ms)</p>
</td>
<td>
<p>254</p>
</td>
<td>
<p>2791</p>
</td>
<td>
<p>5657</p>
</td>
<td>
<p>8587</p>
</td>
<td>
<p>11717</p>
</td>
<td>
<p>14103</p>
</td>
</tr>
<tr>
<td>
<p>flex (ms)</p>
</td>
<td>
<p>15</p>
</td>
<td>
<p>155</p>
</td>
<td>
<p>296</p>
</td>
<td>
<p>437</p>
</td>
<td>
<p>632</p>
</td>
<td>
<p>752</p>
</td>
</tr>
<tr>
<td>
<p>lexer (ms)</p>
</td>
<td>
<p>12</p>
</td>
<td>
<p>123</p>
</td>
<td>
<p>265</p>
</td>
<td>
<p>401</p>
</td>
<td>
<p>479</p>
</td>
<td>
<p>607</p>
</td>
</tr>
</tbody>
</table>

## Сравнение времени работы синтаксических анализаторов

<table>
<colgroup>
<col style="width: 24%" />
<col style="width: 23%" />
<col style="width: 7%" />
<col style="width: 11%" />
<col style="width: 11%" />
<col style="width: 10%" />
<col style="width: 10%" />
</colgroup>
<tbody>
<tr>
<td colspan="2">
<p>Количество функций</p>
</td>
<td style="text-align: center;">
<p>100</p>
</td>
<td style="text-align: center;">
<p>1000</p>
</td>
<td style="text-align: center;">
<p>10000</p>
</td>
<td style="text-align: center;">
<p>100000</p>
</td>
<td style="text-align: center;">
<p>200000</p>
</td>
</tr>
<tr>
<td rowspan="3">
<p>Синтаксические анализаторы</p>
</td>
<td>
<p>python AST (ms)</p>
</td>
<td style="text-align: center;">
<p>22</p>
</td>
<td style="text-align: center;">
<p>174</p>
</td>
<td style="text-align: center;">
<p>2663</p>
</td>
<td style="text-align: center;">
<p>17934</p>
</td>
<td style="text-align: center;">
<p>26901</p>
</td>
</tr>
<tr>
<td>
<p>bison (ms)</p>
</td>
<td style="text-align: center;">
<p>8</p>
</td>
<td style="text-align: center;">
<p>28</p>
</td>
<td style="text-align: center;">
<p>236</p>
</td>
<td style="text-align: center;">
<p>1726</p>
</td>
<td style="text-align: center;">
<p>2589</p>
</td>
</tr>
<tr>
<td>
<p>parser (ms)</p>
</td>
<td style="text-align: center;">
<p>2</p>
</td>
<td style="text-align: center;">
<p>25</p>
</td>
<td style="text-align: center;">
<p>175</p>
</td>
<td style="text-align: center;">
<p>1458</p>
</td>
<td style="text-align: center;">
<p>2187</p>
</td>
</tr>
</tbody>
</table>

## Выводы

Из таблиц видно, что реализации модулей python сильно медленнее в
сравнении с остальными, в то время как остальные модули близки друг к
другу. Отметим, что:

1.  Часть операций для Tokenizer и AST выполняется интерпретатором
    Python, другая часть выполняется Cи библиотекой.
2.  Модули Tokenizer и AST поддерживают большее количество языковых
    конструкций.
3.  Модуль AST также сохраняет большее количество второстепенной
    информации, которая используется следующими модулями компилятора.

[_Назад_](README.md)
