# Основной класс `Lexer`

Лексический анализатор реализован в виде класса `Lexer`.

```cpp
class Lexer {
  private:
    static std::map<std::string_view, Keyword> keywords;
    static std::map<std::string_view, Operator> operators;
    static TokenList processString(const std::string &str, size_t line_number, ErrorBuffer &errors);
    // ...

  public:
    Lexer() = delete;
    Lexer(const Lexer &) = delete;
    Lexer(Lexer &&) = delete;
    ~Lexer() = delete;

    static TokenList process(const StringVec &source);
};
```
Поля класса: 
* `keywords` – ассоциативный контейнер, где ключом является строка, а значением является перечисляемый тип `Keyword`;
* `operators` – ассоциативный контейнер, где ключом является строка, а значением является перечисляемый тип `Operator`;

[_Назад_](README.md)
