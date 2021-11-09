#include "lexer/lexer.hpp"

#include "lexer/lexer_error.hpp"

using namespace lexer;

// clang-format off
std::map<std::string_view, Keyword> Lexer::keywords = {
    {"bool", Keyword::Bool},         {"False", Keyword::False},
    {"int", Keyword::Int},           {"float", Keyword::Float},
    {"str", Keyword::Str},           {"if", Keyword::If},
    {"else", Keyword::Else},         {"elif", Keyword::Elif},
    {"range", Keyword::Range},       {"while", Keyword::While},
    {"for", Keyword::For},           {"break", Keyword::Break},
    {"import", Keyword::Import},     {"continue", Keyword::Continue},
    {"def", Keyword::Definition},    {"return", Keyword::Return},
    {"or", Keyword::Or},             {"and", Keyword::And},
    {"not", Keyword::Not},           {"in", Keyword::In},
    {"True", Keyword::True},         {"None", Keyword::None}};

std::map<std::string_view, Operator> Lexer::operators = {
    {"%", Operator::Mod},            {".", Operator::Dot},        {"]", Operator::RectRightBrace},
    {",", Operator::Comma},          {"=", Operator::Assign},     {"+", Operator::Add},
    {"-", Operator::Sub},            {"*", Operator::Mult},       {"/", Operator::Div},
    {"==", Operator::Equal},         {"!=", Operator::NotEqual},  {"<", Operator::Less},
    {">", Operator::Greater},        {"<=", Operator::LessEqual}, {">=", Operator::GreaterEqual},
    {"(", Operator::LeftBrace},      {")", Operator::RightBrace}, {"[", Operator::RectLeftBrace}};
// clang-format on

TokenList Lexer::process(const StringVec &source) {
    TokenList tokens;
    size_t line_number = 1;
    ErrorBuffer errors;
    for (const auto &str : source) {
        TokenList part = processString(str, line_number++, errors);
        tokens.splice(tokens.end(), part);
    }
    if (!errors.empty()) {
        throw errors;
    }
    return tokens;
}

TokenList Lexer::processString(const std::string &str, size_t line_number, ErrorBuffer &errors) {
    TokenList tokens;

    auto space_count = str.find_first_not_of(' ');

    if (space_count == std::string::npos) {
        return tokens;
    }

    if (space_count % 4 != 0) {
        errors.push<LexerError>(line_number, 1, "Extra spaces at the begining of line are not allowed");
    }

    int indentation_count = space_count / 4;
    for (int i = 0; i < indentation_count; i++) {
        tokens.emplace_back(Special::Indentation);
    }

    auto begin_token = str.begin() + space_count;
    auto end_token = str.begin() + space_count;

    for (auto i = str.begin() + space_count; i != str.end(); i++) {

        if (isalpha(*i)) {
            end_token++;
            continue;
        }

        if (*i == '@') {
            errors.push<LexerError>(line_number, std::distance(i, str.begin()),
                                    "Identifier cannot contain special characters");
        }

        // pushing Keyword.
        if (end_token != begin_token) {
            auto tok_id = keywords.find(
                std::string_view(&*begin_token, static_cast<size_t>(std::distance(begin_token, end_token))));
            if (tok_id != keywords.cend())
                tokens.emplace_back(tok_id->second);
            else {
                while ((i != str.end()) && isalnum(*i)) // adding identifier with numbers
                {
                    end_token++;
                    i++;
                }
                tokens.emplace_back(TokenType::Identifier, std::string(begin_token, end_token));
            }
            begin_token = i;
            end_token = i;
        }

        if (isspace(*i)) {
            begin_token = i + 1;
            end_token = i + 1;
            continue;
        }

        if (isalnum(*i)) { // pushing Integer number
            begin_token = i;
            end_token = i;
            while (i != str.end() && isdigit(*i)) {
                end_token++;
                i++;
            }

            if (i != str.end() && isalpha(*i)) {
                errors.push<LexerError>(line_number, std::distance(i, str.begin()),
                                        "Identifier cannot start with numbers");
            }

            if (i != str.end() && *i == '.') { // pushing Float number
                end_token++;
                i++;
                while (i != str.end() && isalnum(*i)) {
                    end_token++;
                    i++;
                }
                tokens.emplace_back(TokenType::FloatingPointLiteral, std::string(begin_token, end_token));
                begin_token = i;
                end_token = i;
                i--;
                continue;
            }

            tokens.emplace_back(TokenType::IntegerLiteral, std::string(begin_token, end_token));
            begin_token = i;
            end_token = i;
            i--;
            continue;
        }

        if (*i == '"') {
            i++;
            begin_token = i;
            end_token = i;
            while (i != str.end() && *i != '"') {
                end_token++;
                i++;
            }
            tokens.emplace_back(TokenType::StringLiteral, std::string(begin_token, end_token));
            begin_token = i;
            end_token = i;

            if (i == str.end()) {
                errors.push<LexerError>(line_number, str.size(), "No matching closing quote found");
                break;
            }

            continue;
        }

        begin_token = i;
        end_token = i;

        // pushing Operators
        if (i + 1 != str.cend())
            if ((*i == '!' || *i == '=' || *i == '<' || *i == '>') && *(i + 1) == '=') {
                i++;
                end_token++;
            }

        end_token++;

        if (i + 1 != str.cend() && (*i == '-') && *(i + 1) == '>') {
            tokens.emplace_back(Special::Arrow);
            i++;
            begin_token = i;
            end_token = i;
            continue;
        }

        if (*i == ':') {
            tokens.emplace_back(Special::Colon);
            begin_token = i;
            end_token = i;
            continue;
        }

        auto tok_id =
            operators.find(std::string_view(&*begin_token, static_cast<size_t>(std::distance(begin_token, end_token))));
        if (tok_id != operators.end())
            tokens.emplace_back(tok_id->second);

        begin_token = i;
        end_token = i;
    }

    // adding a word or symbol at the end of a line
    if (begin_token != end_token) {
        auto tok_id =
            keywords.find(std::string_view(&*begin_token, static_cast<size_t>(std::distance(begin_token, end_token))));
        auto tok_src =
            operators.find(std::string_view(&*begin_token, static_cast<size_t>(std::distance(begin_token, end_token))));
        if (tok_id != keywords.end())
            tokens.emplace_back(tok_id->second);
        else if (tok_src != operators.end())
            tokens.emplace_back(tok_id->second);
        else
            tokens.emplace_back(TokenType::Identifier, std::string(begin_token, end_token));
    }

    tokens.emplace_back(Special::EndOfExpression);

    return tokens;
}
