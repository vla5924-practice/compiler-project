# Пример работы генератора

Исходный код программы:

```python
def fact(n: int) -> int:
    if n <= 1:
        return 1
    return n * fact(n - 1)

def main() -> None:
    x: int
    x = fact(3)
    print(x)
```

Код LLVM IR, построенный генератором:

```llvm
@.placeholder.int = private unnamed_addr constant [3 x i8] c"%d\00", align 1
declare i32 @printf(i8*, ...)
declare i32 @scanf(i8*, ...)

define i64 @fact(i64 %0) {
fact_entry:
  %n = alloca i64, align 8
  store i64 %0, i64* %n, align 4
  br label %block
block:                                            ; preds = %fact_entry
  br label %ifcond
ifcond:                                           ; preds = %block
  %1 = load i64, i64* %n, align 4
  %2 = icmp sle i64 %1, 1
  br i1 %2, label %ifbody, label %endif
ifbody:                                           ; preds = %ifcond
  ret i64 1
  br label %endif
endif:                                            ; preds = %ifbody, %ifcond
  %3 = load i64, i64* %n, align 4
  %4 = sub i64 %3, 1
  %5 = call i64 @fact(i64 %4)
  %6 = load i64, i64* %n, align 4
  %7 = mul i64 %6, %5
  ret i64 %7
}

define void @main() {
main_entry:
  br label %block
block:                                            ; preds = %main_entry
  %x = alloca i64, align 8
  %0 = call i64 @fact(i64 3)
  store i64 %0, i64* %x, align 4
  %1 = load i64, i64* %x, align 4
  %2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.placeholder.int, i32 0, i32 0), i64 %1)
  %3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.placeholder.newline, i32 0, i32 0))
  ret void
}
```

[_Назад_](README.md)
