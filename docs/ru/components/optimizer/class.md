# Основной класс `Optimizer`

```cpp
class Optimizer {
  public:
    Optimizer() = delete;
    Optimizer(const Optimizer &) = delete;
    Optimizer(Optimizer &&) = delete;
    ~Optimizer() = delete;

    static void process(SyntaxTree &tree, const OptimizerOptions &options = OptimizerOptions::all());
};
```

[_Назад_](README.md)
