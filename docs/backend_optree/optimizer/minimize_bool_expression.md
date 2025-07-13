# Минимизация булевых выражений

Минимизация булевых выражений – оптимизация в компиляторах, умещающая
количество операций в булевых выражениях.

Оптимизация выполняется на бинарных и унарных операциях. Оптимизацию
можно разделить на несколько отдельных трансформаций.

Опишем поддерживаемые трансформации, основанные на свойствах и аксиомах
булевой логики.

| Закон, аксиома, свойство | Описание |
|----|----|
| Аксиома поглощения | a or (a and b) = a; a and (a or b) = a |
| Аксиома дополнительности | a or !a = 1; a and !a = 0 |
| Свойство идемпотентности | a or a = a; a and a = a |
| Законах де Моргана | !a and !b = !(a or b); !a or !b = !(a and b) |
| Законах Блейка-Порецкого | a or (a and b) = a; a and (a or b) = a |
| Свойствах констант | a or false = a; a and true = a; a or true = true; a and false = false; !false = true; !true = false |
| Свойства интервалов и отрезков | {(x \> a) or (x \> b) \| a \> b, a,b – литералы} = x \>= b |
| Закон двойного отрицания | !!a = a |
| Свойства эквивалентности | a ~ a = 1; a ~ !a = 0 |

Так как оптимизация выполняется на дереве операций, опишем особенности,
связанные с трансформациями:

1.  Под константами true и false могут подразумеваться и литералы. Общие
    правила для преобразований:
    1.  Целочисленные литералы: 0 – false, все остальные числа true.
    2.  Дробные литералы, аналогично целочисленным.
    3.  Строковые литералы – пустые строки false, остальные true.
2.  Все приведенные трансформации всегда приводят к редуцированию
    количества операций или операндов.
3.  Проверки происходят при точном совпадении операций, а не при их
    семантической схожести. Например,

```python
X: int = -4
Y: int = -4
Z: bool = X and Y
```

Переменные X и Y имеют одинаковые значения, но являются разными узлами
в дереве операций. Для таких ситуаций необходимо подставлять значения
этих переменных, что делает другая оптимизация.

Приведем несколько примеров выполнения оптимизации.

<table>
<colgroup>
<col style="width: 50%" />
<col style="width: 50%" />
</colgroup>
<thead>
<tr>
<th colspan="2" style="text-align: center;">Выражения, содержащие
константы</th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">До оптимизации</td>
<td style="text-align: center;">После оптимизации</td>
</tr>
<tr>
<td style="text-align: center;"><p>def foo(x: int, y: float) -&gt;
None:</p>
<p>z: bool = 1 and (x and (x or y))</p></td>
<td style="text-align: center;"><p>def foo(x: int, y: float) -&gt;
None:</p>
<p>z: bool = True</p></td>
</tr>
<tr>
<td colspan="2" style="text-align: center;"><b>Выражения, содержащие только
переменные (аксиома поглощения)</b></td>
</tr>
<tr>
<td style="text-align: center;">До оптимизации</td>
<td style="text-align: center;">После оптимизации</td>
</tr>
<tr>
<td style="text-align: center;"><p>def foo(x: int, y: float) -&gt;
None:</p>
<p>z: bool = x or y or (x and y)</p></td>
<td style="text-align: center;"><p>def foo(x: int, y: float) -&gt;
None:</p>
<p>z: bool = x or y</p></td>
</tr>
<tr>
<td colspan="2" style="text-align: center;"><b>Свертка интервалов (аксиома
поглощения)</b></td>
</tr>
<tr>
<td style="text-align: center;">До оптимизации</td>
<td style="text-align: center;">После оптимизации</td>
</tr>
<tr>
<td style="text-align: center;"><p>def foo(x: int) -&gt; None:</p>
<p>z: bool = (x &gt; 4) or (x &gt; 5)</p></td>
<td style="text-align: center;"><p>def foo(x: int) -&gt; None:</p>
<p>z: bool = x &gt; 4</p></td>
</tr>
</tbody>
</table>

[_Назад_](README.md)
