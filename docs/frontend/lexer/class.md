# Основной класс `Lexer`

Лексический анализатор реализован в виде класса `Lexer`.

```cpp
class Lexer {
    static TokenList processString(const std::string &str, size_t line_number, ErrorBuffer &errors);

  public:
    Lexer() = delete;
    Lexer(const Lexer &) = delete;
    Lexer(Lexer &&) = delete;
    ~Lexer() = delete;

    static TokenList process(const StringVec &source);
};
```

[_Назад_](README.md)
