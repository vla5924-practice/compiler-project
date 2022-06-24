# Основной класс `Semantizer`

```cpp
class Semantizer {
  public:
    Semantizer() = delete;
    Semantizer(const Semantizer &) = delete;
    Semantizer(Semantizer &&) = delete;
    ~Semantizer() = delete;

    static void process(SyntaxTree &tree);
};
```

[_Назад_](README.md)
