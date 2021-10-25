#include "lexer/lexer.hpp"

using namespace lexer;

// clang-format off
std::map<std::string_view, Keyword> Lexer::keywords = {
    {"bool", Keyword::Bool},         {"False", Keyword::False},
    {"int", Keyword::Int},           {"float", Keyword::Float},
    {"str", Keyword::String},        {"if", Keyword::If},
    {"else", Keyword::Else},         {"elif", Keyword::Elif},
    {"range", Keyword::Range},       {"while", Keyword::While},
    {"for", Keyword::For},           {"break", Keyword::Break},
    {"import", Keyword::Import},     {"continue", Keyword::Continue},
    {"def", Keyword::Definition},    {"return", Keyword::Return},
    {"or", Keyword::Or},             {"and", Keyword::And},
    {"not", Keyword::Not},           {"in", Keyword::In},
    {"True", Keyword::True},         {"None", Keyword::None}};

std::map<std::string_view, Operator> Lexer::operators = {
    {":", Operator::Colon},          {"%", Operator::Mod},        {".", Operator::Dot},
    {",", Operator::Comma},          {"=", Operator::Assign},     {"+", Operator::Plus},
    {"-", Operator::Minus},          {"*", Operator::Mult},       {"/", Operator::Div},
    {"==", Operator::Equal},         {"!=", Operator::NotEqual},  {"<", Operator::Less},
    {">", Operator::More},           {"<=", Operator::LessEqual}, {">=", Operator::MoreEqual},
    {"(", Operator::LeftBrace},      {")", Operator::RightBrace}, {"[", Operator::RectLeftBrace},
    {"]", Operator::RectRightBrace}, {"->", Operator::Arrow}};
// clang-format on

TokenList Lexer::process(const StringVec &source) {
    TokenList tokens;
    for (const auto &str : source) {
        TokenList part = processString(str);
        tokens.splice(tokens.end(), part);
    }
    return tokens;
}

TokenList Lexer::processString(const std::string &str) {
    TokenList tokens;

    auto space_count = str.find_first_not_of(' ');

    if (space_count == std::string::npos) {
        return tokens;
    }

    int indentation_count = space_count / 4;
    for (int i = 0; i < indentation_count; i++) {
        tokens.emplace_back(std::move(Token::make<Type::Special>(Special::Indentation)));
    }

    auto begin_token = str.begin() + space_count;
    auto end_token = str.begin() + space_count;

    for (auto i = str.begin() + space_count; i != str.end(); i++) {

        if (isalpha(*i)) {
            end_token++;
            continue;
        }

        if (end_token != begin_token) // pushing Keyword.
        {
            auto tok_id = keywords.find(
                std::string_view(&*begin_token, static_cast<size_t>(std::distance(begin_token, end_token))));
            if (tok_id != keywords.cend())
                tokens.emplace_back(std::move(Token::make<Type::Keyword>(tok_id->second)));
            else {
                while ((i != str.end()) && isalnum(*i)) // adding identifier with numbers
                {
                    end_token++;
                    i++;
                }
                tokens.emplace_back(std::move(Token::make<Type::Identifier>(std::string(begin_token, end_token))));
            }
            begin_token = i;
            end_token = i;
        }

        if (isspace(*i)) {
            begin_token++;
            end_token++;
            continue;
        }

        if (isalnum(*i)) { // pushing Integer number
            while (isalnum(*(i + 1))) {
                end_token++;
                i++;
            }

            if (*(i + 1) == '.') // pushing Float number
            {
                end_token++;
                i++;
                while (isalnum(*(i + 1))) {
                    end_token++;
                    i++;
                }
                tokens.emplace_back(
                    std::move(Token::make<Type::FloatingPointLiteral>(std::string(begin_token, end_token))));
                begin_token = i;
                end_token = i;
                continue;
            }

            tokens.emplace_back(std::move(Token::make<Type::IntegerLiteral>(std::string(begin_token, end_token))));
            begin_token = i;
            end_token = i;
            continue;
        }

        if (*i == '"') {
            i++;
            begin_token = i;
            end_token = i;
            while (*i != '"') {
                end_token++;
                i++;
            }
            tokens.emplace_back(std::move(Token::make<Type::StringLiteral>(std::string(begin_token, end_token))));
            begin_token = i;
            end_token = i;
            continue;
        }

        if (((*i == '!') || (*i == '=') || (*i == '<') || (*i == '>')) && (*(i + 1) == '=') ||
            (*i == '-') && (*(i + 1) == '>')) // pushing Operators
        {
            end_token++;
            i++;
        }
        auto tok_id = operators.find(
            std::string_view(&*begin_token, static_cast<size_t>(std::distance(begin_token, end_token))));
        if (tok_id != operators.end())
            tokens.emplace_back(std::move(Token::make<Type::Operator>(tok_id->second)));
        begin_token = i;
        end_token = i;
    }

    // adding a word or symbol at the end of a line
    if (begin_token != end_token) {
        auto tok_id = keywords.find(
            std::string_view(&*begin_token, static_cast<size_t>(std::distance(begin_token, end_token))));
        auto tok_src = operators.find(
            std::string_view(&*begin_token, static_cast<size_t>(std::distance(begin_token, end_token))));
        if (tok_id != keywords.end())
            tokens.emplace_back(std::move(Token::make<Type::Keyword>(tok_id->second)));
        else if (tok_src != operators.end())
            tokens.emplace_back(std::move(Token::make<Type::Operator>(tok_src->second)));
    }

    tokens.emplace_back(std::move(Token::make<Type::Special>(Special::EndOfExpression)));

    return tokens;
}
