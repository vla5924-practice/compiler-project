#include "lexer.hpp"

std::map<std::string_view, Token::Keyword> Lexer::keywords = {
    {"begin", Token::Keyword::Begin},     {"end", Token::Keyword::End},     {"var", Token::Keyword::Var},
    {"integer", Token::Keyword::Integer}, {"if", Token::Keyword::If},       {"then", Token::Keyword::Then},
    {"else", Token::Keyword::Else},       {"while", Token::Keyword::While}, {"do", Token::Keyword::Do},
    {"for", Token::Keyword::For},         {"to", Token::Keyword::To},       {"repeat", Token::Keyword::Repeat},
    {"until", Token::Keyword::Until},
};

std::map<std::string_view, Token::Operator> Lexer::operators = {
    {":", Token::Operator::Colon},     {";", Token::Operator::Semicolon},  {".", Token::Operator::Stop},
    {",", Token::Operator::Comma},     {":=", Token::Operator::Assign},    {"+", Token::Operator::Plus},
    {"-", Token::Operator::Minus},     {"*", Token::Operator::Mult},       {"/", Token::Operator::Div},
    {"=", Token::Operator::Equal},     {"<>", Token::Operator::NotEqual},  {"<", Token::Operator::Less},
    {">", Token::Operator::More},      {"<=", Token::Operator::LessEqual}, {">=", Token::Operator::MoreEqual},
    {"(", Token::Operator::LeftBrace}, {")", Token::Operator::RightBrace},
};

TokenList Lexer::process(const StringVec &source) {
    std::list<Token> tokens;
    for (const auto &str : source) {
        TokenList part = processString(str);
        tokens.insert(tokens.end(), part.begin(), part.end());
    }
    return tokens;
}

TokenList Lexer::processString(const std::string &str) {
    std::list<Token> tokens;
    std::string id_str;
    int i = 0;
    for (auto i = str.begin(); i != str.end(); ++i) {
        if (isspace(*i) && id_str.size() == 0) // skipping spaces at the beginning of a line
        {
            continue;
        }

        if (isalpha(*i)) // adding letters or numbers to the id_str
        {
            id_str += *i;
        } else {
            if (id_str.size() != 0) // pushing Keyword.
            {
                auto tok_id = Lexer::keywords.find(id_str);
                if (tok_id != Lexer::keywords.end())
                    tokens.push_back(Token::make<Token::Type::Keyword>(tok_id->second));
                else {
                    while (((i) != str.end()) && isalnum(*i)) // adding identifier with numbers
                    {
                        id_str += *i;
                        i++;
                    }
                    tokens.push_back(Token::make<Token::Type::Identifier>(id_str));
                }
                id_str.clear();
            }

            if (isspace(*i)) {
                continue;
            }

            id_str += *i;
            if (*i == 39) // ASCII ' = 39?
            {
                id_str.clear();
                i++;
                while (*i != 39) // TODO need '' check
                {
                    id_str += *i;
                    i++;
                }
                tokens.push_back(Token::make<Token::Type::StringLiteral>(id_str));
                id_str.clear();
                continue;
            }

            if (isalnum(id_str[0])) // pushing Integer number
            {
                while (isalnum(*(i + 1))) {
                    id_str += *(i + 1);
                    i++;
                }

                if (*(i + 1) == '.') // pushing Float number
                {
                    id_str += *(i + 1);
                    i++;
                    while (isalnum(*(i + 1))) {
                        id_str += *(i + 1);
                        i++;
                    }
                    tokens.push_back(Token::make<Token::Type::FloatingPointLiteral>(id_str));
                    id_str.clear();
                    continue;
                }

                tokens.push_back(Token::make<Token::Type::IntegerLiteral>(id_str));
                id_str.clear();
                continue;
            }

            if (((*i == ':') || (*i == '<') || (*i == '>')) && (*(i + 1) == '=')) // pushing Operators
            {
                id_str += *(i + 1);
                i++;
            }
            auto tok_id = Lexer::operators.find(id_str);
            if (tok_id != Lexer::operators.end())
                tokens.push_back(Token::make<Token::Type::Operator>(tok_id->second));
            id_str.clear();
        }
    }
    // adding a word or symbol at the end of a line
    // these words and symbols are 'begin', ';', '.', ')'
    if (id_str.size() != 0) {
        auto tok_id = Lexer::keywords.find(id_str);
        auto tok_src = Lexer::operators.find(id_str);
        if (tok_id != Lexer::keywords.end())
            tokens.push_back(Token::make<Token::Type::Keyword>(tok_id->second));
        else if (tok_src != Lexer::operators.end())
            tokens.push_back(Token::make<Token::Type::Operator>(tok_src->second));
    }
    return tokens;
}
