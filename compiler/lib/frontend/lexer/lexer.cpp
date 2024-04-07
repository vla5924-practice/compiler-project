#include "lexer/lexer.hpp"

#include <string_view>
#include <unordered_map>

#include "lexer/lexer_error.hpp"
#include "lexer/token.hpp"
#include "lexer/token_types.hpp"

using namespace lexer;
using namespace utils;

namespace {

std::unordered_map<std::string_view, Keyword> keywords = {
    {"bool", Keyword::Bool},      {"False", Keyword::False},
    {"int", Keyword::Int},        {"float", Keyword::Float},
    {"str", Keyword::Str},        {"if", Keyword::If},
    {"else", Keyword::Else},      {"elif", Keyword::Elif},
    {"range", Keyword::Range},    {"while", Keyword::While},
    {"for", Keyword::For},        {"break", Keyword::Break},
    {"import", Keyword::Import},  {"continue", Keyword::Continue},
    {"def", Keyword::Definition}, {"return", Keyword::Return},
    {"or", Keyword::Or},          {"and", Keyword::And},
    {"not", Keyword::Not},        {"in", Keyword::In},
    {"True", Keyword::True},      {"None", Keyword::None},
};

std::unordered_map<std::string_view, Operator> operators = {
    {"%", Operator::Mod},       {".", Operator::Dot},        {"]", Operator::RectRightBrace},
    {",", Operator::Comma},     {"=", Operator::Assign},     {"+", Operator::Add},
    {"-", Operator::Sub},       {"*", Operator::Mult},       {"/", Operator::Div},
    {"==", Operator::Equal},    {"!=", Operator::NotEqual},  {"<", Operator::Less},
    {">", Operator::Greater},   {"<=", Operator::LessEqual}, {">=", Operator::GreaterEqual},
    {"(", Operator::LeftBrace}, {")", Operator::RightBrace}, {"[", Operator::RectLeftBrace},
};

inline std::string_view makeStringView(std::string::const_iterator begin_token, std::string::const_iterator end_token) {
    return std::string_view(&*begin_token, static_cast<size_t>(std::distance(begin_token, end_token)));
}

} // namespace

TokenList Lexer::process(const SourceFile &source) {
    TokenList tokens;
    ErrorBuffer errors;
    for (const auto &line : source) {
        TokenList part = processString(line, errors);
        tokens.splice(tokens.end(), part);
    }
    if (!errors.empty()) {
        throw errors;
    }
    return tokens;
}

TokenList Lexer::processString(const SourceLine &source, ErrorBuffer &errors) {
    constexpr const char *ALLOWED_SYMBOLS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"
                                            "\".,+-*/><=%()[]!: ";
    constexpr const char *ID_ALLOWED_SYMBOLS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";

    TokenList tokens;
    const SourceRef &ref = source.ref;
    auto space_count = source.text.find_first_not_of(' ');

    if (space_count == std::string::npos) {
        return tokens;
    }

    if (space_count % 4 != 0) {
        errors.push<LexerError>(ref.inSameLine(1u), "Extra spaces at the begining of line are not allowed");
    }

    size_t indentation_count = space_count / 4;
    for (size_t i = 0; i < indentation_count; i++) {
        tokens.emplace_back(Special::Indentation, ref.inSameLine(indentation_count * 4u));
    }

    auto begin_token = source.text.begin() + space_count;
    auto end_token = begin_token;

    for (auto i = begin_token; i != source.text.end(); i++) {
        const char *j = ALLOWED_SYMBOLS;
        for (; *j != '\0'; j++) {
            if (*i == *j)
                break;
        }
        if (*j == '\0') {
            errors.push<LexerError>(ref.inSameLine(std::distance(source.text.begin(), i)),
                                    std::string("Unexpected symbol ") + *i);
            break;
        }

        if (isalpha(*i) || *i == '_') {
            end_token++;
            continue;
        }

        // pushing Keyword.
        if (end_token != begin_token) {
            auto tok_id = keywords.find(makeStringView(begin_token, end_token));
            if (tok_id != keywords.cend())
                tokens.emplace_back(tok_id->second, source.makeRef(i));
            else {
                while (i != source.text.end()) { // adding identifier with numbers
                    const char *j = ID_ALLOWED_SYMBOLS;
                    for (; *j != '\0'; j++) {
                        if (*i == *j) {
                            end_token++;
                            i++;
                            break;
                        }
                    }
                    if (*j == '\0') {
                        break;
                    }
                }
                auto pos = makeStringView(begin_token, end_token).find_first_not_of(ID_ALLOWED_SYMBOLS);
                if (pos != std::string::npos) {
                    errors.push<LexerError>(ref.inSameLine(std::distance(source.text.begin(), begin_token)),
                                            "Identifier cannot contain special characters");
                }
                tokens.emplace_back(TokenType::Identifier, std::string(begin_token, end_token), source.makeRef(i));
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
            while (i != source.text.end() && isdigit(*i)) {
                end_token++;
                i++;
            }

            if (i != source.text.end() && isalpha(*i)) {
                errors.push<LexerError>(ref.inSameLine(std::distance(i, source.text.begin())),
                                        "Unexpected characters in numeric literal");
            }

            if (i != source.text.end() && *i == '.') { // pushing Float number
                end_token++;
                i++;
                while (i != source.text.end() && isdigit(*i)) {
                    end_token++;
                    i++;
                }
                if (i != source.text.end() && isalpha(*i))
                    errors.push<LexerError>(ref.inSameLine(std::distance(i, source.text.begin())),
                                            "Unexpected characters in numeric literal");
                tokens.emplace_back(TokenType::FloatingPointLiteral, std::string(begin_token, end_token),
                                    source.makeRef(i));
                begin_token = i;
                end_token = i;
                i--;
                continue;
            }

            tokens.emplace_back(TokenType::IntegerLiteral, std::string(begin_token, end_token), source.makeRef(i));
            begin_token = i;
            end_token = i;
            i--;
            continue;
        }

        if (*i == '"') {
            i++;
            begin_token = i;
            end_token = i;
            while (i != source.text.end() && *i != '"') {
                end_token++;
                i++;
            }
            tokens.emplace_back(TokenType::StringLiteral, std::string(begin_token, end_token), source.makeRef(i));
            begin_token = i;
            end_token = i;

            if (i == source.text.end()) {
                errors.push<LexerError>(ref.inSameLine(source.text.size()), "No matching closing quote found");
                break;
            }

            continue;
        }

        begin_token = i;
        end_token = i;

        // pushing Operators
        if (i + 1 != source.text.cend())
            if ((*i == '!' || *i == '=' || *i == '<' || *i == '>') && *(i + 1) == '=') {
                i++;
                end_token++;
            }

        end_token++;

        if (i + 1 != source.text.cend() && (*i == '-') && *(i + 1) == '>') {
            tokens.emplace_back(Special::Arrow, source.makeRef(i));
            i++;
            begin_token = end_token;
            continue;
        }

        if (*i == ':') {
            tokens.emplace_back(Special::Colon, source.makeRef(i));
            begin_token = end_token;
            continue;
        }

        auto tok_id = operators.find(makeStringView(begin_token, end_token));
        if (tok_id != operators.end())
            tokens.emplace_back(tok_id->second, source.makeRef(i));

        begin_token = end_token;
    }

    // adding a word or symbol at the end of a line
    if (begin_token != end_token) {
        auto tok_id = keywords.find(makeStringView(begin_token, end_token));
        auto tok_src = operators.find(makeStringView(begin_token, end_token));
        if (tok_id != keywords.end())
            tokens.emplace_back(tok_id->second, source.makeRef(begin_token));
        else if (tok_src != operators.end())
            tokens.emplace_back(tok_id->second, source.makeRef(begin_token));
        else {
            auto pos = makeStringView(begin_token, end_token).find_first_not_of(ID_ALLOWED_SYMBOLS);
            if (pos != std::string::npos) {
                errors.push<LexerError>(ref.inSameLine(std::distance(source.text.begin(), begin_token)),
                                        "Identifier cannot contain special characters");
            }
            tokens.emplace_back(TokenType::Identifier, std::string(begin_token, end_token),
                                source.makeRef(begin_token));
        }
    }

    tokens.emplace_back(Special::EndOfExpression, ref.inSameLine(source.text.length()));

    return tokens;
}
