# Проверка инвариантов

Проверка описанных инвариантов для каждой операции осуществляется путем
обхода дерева операций и вызова реализованных для каждой операции
функций вида `verify<OpType>(OpType op)`, где `OpType` – класс-адаптер
для конкретной операции. Эти функции, в свою очередь, выполняют
последовательность вызовов методов-характеризаторов, а также могут иметь
какую-либо дополнительную логику проверки. Характеризатор представляет
из себя класс, пример которого приведен ниже.

```cpp
struct HasOperands {
    static bool verify(const Operation::Ptr &op, SemantizerContext &ctx, size_t numOperands);
};
```

Вызов указанных методов, в свою очередь, осуществляется не напрямую, а с
помощью вспомогательного класса `TraitVerifier`.

```cpp
class TraitVerifier {
  public:
    TraitVerifier(const Operation::Ptr &op, SemantizerContext &ctx) : op(op), ctx(ctx), acc(true){};
    bool verified() const;
    template <typename Trait, typename... Args>
    TraitVerifier &verify(Args... args);
};
```

Этот класс позволяет строить цепочки проверок следующего вида, что дает
возможность не выполнять проверки после того, как одна из них в цепочке
была провалена:

```cpp
verifier.verify<HasOperands>(0)
        .verify<HasResults>(0)
        .verify<HasAttributes>(2)
        .verify<HasNthAttrOfType<std::string>>(0)
        .verify<HasNthAttrOfType<FunctionType>>(1);
```

[_Назад_](README.md)
