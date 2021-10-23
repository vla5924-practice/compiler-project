#include "lexer/lexer.hpp"

using namespace lexer;

// clang-format off
std::map<std::string_view, Token::Keyword> Lexer::keywords = {
    {"bool", Token::Keyword::Bool},         {"False", Token::Keyword::False},
    {"int", Token::Keyword::Int},           {"float", Token::Keyword::Float},
    {"str", Token::Keyword::String},        {"if", Token::Keyword::If},
    {"else", Token::Keyword::Else},         {"elif", Token::Keyword::Elif},
    {"range", Token::Keyword::Range},       {"while", Token::Keyword::While},
    {"for", Token::Keyword::For},           {"break", Token::Keyword::Break},
    {"import", Token::Keyword::Import},     {"continue", Token::Keyword::Continue},
    {"def", Token::Keyword::Definition},    {"return", Token::Keyword::Return},
    {"or", Token::Keyword::Or},             {"and", Token::Keyword::And},
    {"not", Token::Keyword::Not},           {"in", Token::Keyword::In},
    {"True", Token::Keyword::True},         {"None", Token::Keyword::None}};

std::map<std::string_view, Token::Operator> Lexer::operators = {
    {":", Token::Operator::Colon},          {"%", Token::Operator::Mod},        {".", Token::Operator::Dot},
    {",", Token::Operator::Comma},          {"=", Token::Operator::Assign},     {"+", Token::Operator::Plus},
    {"-", Token::Operator::Minus},          {"*", Token::Operator::Mult},       {"/", Token::Operator::Div},
    {"==", Token::Operator::Equal},         {"!=", Token::Operator::NotEqual},  {"<", Token::Operator::Less},
    {">", Token::Operator::More},           {"<=", Token::Operator::LessEqual}, {">=", Token::Operator::MoreEqual},
    {"(", Token::Operator::LeftBrace},      {")", Token::Operator::RightBrace}, {"[", Token::Operator::RectLeftBrace},
    {"]", Token::Operator::RectRightBrace}, {"->", Token::Operator::Arrow}};
// clang-format on

TokenList Lexer::process(const StringVec &source) {
    std::list<Token> tokens;
    for (const auto &str : source) {
        TokenList part = processString(str);
        tokens.splice(tokens.end(), part);
    }
    return tokens;
}

TokenList Lexer::processString(const std::string &str) {
    std::list<Token> tokens;
    std::string id_str;
    auto i = str.begin();

    auto space_counter = str.find_first_not_of(' ');

    if (space_counter == std::string::npos) {
        return tokens;
    }

    int indentation_count = space_counter / 4;
    for (int i = 0; i < indentation_count; i++) {
        tokens.emplace_back(std::move(Token::make<Token::Type::Special>(Token::Special::Indentation)));
    }

    i += space_counter;

    auto begin_token = i;
    auto end_token = i;

    for (; i != str.end(); ++i) {

        if (isalpha(*i)) {
            end_token++;
        } else {
            if (end_token != begin_token) // pushing Keyword.
            {
                auto tok_id = Lexer::keywords.find(
                    std::string_view(&*begin_token, static_cast<size_t>(std::distance(begin_token, end_token))));
                if (tok_id != Lexer::keywords.cend())
                    tokens.emplace_back(std::move(Token::make<Token::Type::Keyword>(tok_id->second)));
                else {
                    while ((i != str.end()) && isalnum(*i)) // adding identifier with numbers
                    {
                        end_token++;
                        i++;
                    }
                    tokens.emplace_back(
                        std::move(Token::make<Token::Type::Identifier>(std::string(begin_token, end_token))));
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
                        std::move(Token::make<Token::Type::FloatingPointLiteral>(std::string(begin_token, end_token))));
                    begin_token = i;
                    end_token = i;
                    continue;
                }

                tokens.emplace_back(
                    std::move(Token::make<Token::Type::IntegerLiteral>(std::string(begin_token, end_token))));
                begin_token = i;
                end_token = i;
                continue;
            }

            if (*i == '\"') {
                i++;
                begin_token = i;
                end_token = i;
                while (*i != '\"')
                {
                    end_token++;
                    i++;
                }
                tokens.emplace_back(
                    std::move(Token::make<Token::Type::StringLiteral>(std::string(begin_token, end_token))));
                begin_token = i;
                end_token = i;
                continue;
            }

            if (((*i == '!') || (*i == '=') || (*i == '<') || (*i == '>')) && (*(i + 1) == '=') || (*i == '-') && (*(i + 1) == '>')) // pushing Operators
            {
                end_token++;
                i++;
            }
            auto tok_id = Lexer::operators.find(
                    std::string_view(&*begin_token, static_cast<size_t>(std::distance(begin_token, end_token))));
            if (tok_id != Lexer::operators.end())
                tokens.emplace_back(std::move(Token::make<Token::Type::Operator>(tok_id->second)));
            begin_token = i;
            end_token = i;
        }
    }
    
    // adding a word or symbol at the end of a line
    // these words and symbols are 'begin', ';', '.', ')'
    if (begin_token != end_token) {
        auto tok_id = Lexer::keywords.find(
                    std::string_view(&*begin_token, static_cast<size_t>(std::distance(begin_token, end_token))));
        auto tok_src = Lexer::operators.find(
                    std::string_view(&*begin_token, static_cast<size_t>(std::distance(begin_token, end_token))));
        if (tok_id != Lexer::keywords.end())
            tokens.emplace_back(std::move(Token::make<Token::Type::Keyword>(tok_id->second)));
        else if (tok_src != Lexer::operators.end())
            tokens.emplace_back(std::move(Token::make<Token::Type::Operator>(tok_src->second)));
    }

    tokens.emplace_back(std::move(Token::make<Token::Type::Special>(Token::Special::EndOfExpression)));

    return tokens;
}
