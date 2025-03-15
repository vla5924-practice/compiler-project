#include "lexer/lexer.hpp"

#include <cctype>
#include <iterator>
#include <string_view>
#include <unordered_map>

#include "lexer/lexer_error.hpp"
#include "lexer/token.hpp"
#include "lexer/token_types.hpp"

using namespace lexer;
using namespace utils;

namespace {

std::unordered_map<std::string_view, Keyword> keywords = {
    {"bool", Keyword::Bool},      {"False", Keyword::False},   {"int", Keyword::Int},
    {"float", Keyword::Float},    {"str", Keyword::Str},       {"if", Keyword::If},
    {"else", Keyword::Else},      {"elif", Keyword::Elif},     {"for", Keyword::For},
    {"break", Keyword::Break},    {"import", Keyword::Import}, {"continue", Keyword::Continue},
    {"def", Keyword::Definition}, {"return", Keyword::Return}, {"or", Keyword::Or},
    {"and", Keyword::And},        {"not", Keyword::Not},       {"in", Keyword::In},
    {"True", Keyword::True},      {"None", Keyword::None},     {"list", Keyword::List},
    {"while", Keyword::While},    {"pass", Keyword::Pass},
};

std::unordered_map<std::string_view, Operator> operators = {
    {"%", Operator::Mod},       {".", Operator::Dot},        {"]", Operator::RectRightBrace},
    {",", Operator::Comma},     {"=", Operator::Assign},     {"+", Operator::Add},
    {"-", Operator::Sub},       {"*", Operator::Mult},       {"/", Operator::Div},
    {"==", Operator::Equal},    {"!=", Operator::NotEqual},  {"<", Operator::Less},
    {">", Operator::Greater},   {"<=", Operator::LessEqual}, {">=", Operator::GreaterEqual},
    {"(", Operator::LeftBrace}, {")", Operator::RightBrace}, {"[", Operator::RectLeftBrace},
};

std::string_view makeStringView(std::string::const_iterator tokenBegin, std::string::const_iterator tokenEnd) {
    return {&*tokenBegin, static_cast<size_t>(std::distance(tokenBegin, tokenEnd))};
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
    constexpr const char *allowedChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_\".,+-*/><=%()"
                                         "[]!: ";
    constexpr const char *idAllowedChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";

    TokenList tokens;
    const SourceRef &ref = source.ref;
    auto numSpaces = source.text.find_first_not_of(' ');

    if (numSpaces == std::string::npos) {
        return tokens;
    }

    if (numSpaces % 4 != 0) {
        errors.push<LexerError>(ref.inSameLine(1U), "Extra spaces at the begining of line are not allowed");
    }

    size_t numIndents = numSpaces / 4;
    for (size_t i = 0; i < numIndents; i++) {
        tokens.emplace_back(Special::Indentation, ref.inSameLine(numIndents * 4U));
    }

    auto tokenBegin = source.text.begin() + numSpaces;
    auto tokenEnd = tokenBegin;

    for (auto i = tokenBegin; i != source.text.end(); i++) {
        const char *j = allowedChars;
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
            tokenEnd++;
            continue;
        }

        // pushing Keyword.
        if (tokenEnd != tokenBegin) {
            auto tokenId = keywords.find(makeStringView(tokenBegin, tokenEnd));
            if (tokenId != keywords.cend())
                tokens.emplace_back(tokenId->second, source.makeRef(i));
            else {
                while (i != source.text.end()) { // adding identifier with numbers
                    const char *j = idAllowedChars;
                    for (; *j != '\0'; j++) {
                        if (*i == *j) {
                            tokenEnd++;
                            i++;
                            break;
                        }
                    }
                    if (*j == '\0') {
                        break;
                    }
                }
                auto pos = makeStringView(tokenBegin, tokenEnd).find_first_not_of(idAllowedChars);
                if (pos != std::string::npos) {
                    errors.push<LexerError>(ref.inSameLine(std::distance(source.text.begin(), tokenBegin)),
                                            "Identifier cannot contain special characters");
                }
                tokens.emplace_back(TokenType::Identifier, std::string(tokenBegin, tokenEnd), source.makeRef(i));
            }
            tokenBegin = i;
            tokenEnd = i;
        }

        if (isspace(*i)) {
            tokenBegin = i + 1;
            tokenEnd = i + 1;
            continue;
        }

        if (isalnum(*i)) { // pushing Integer number
            tokenBegin = i;
            tokenEnd = i;
            while (i != source.text.end() && isdigit(*i)) {
                tokenEnd++;
                i++;
            }

            if (i != source.text.end() && isalpha(*i) && *i != 'e') {
                errors.push<LexerError>(ref.inSameLine(std::distance(i, source.text.begin())),
                                        "Unexpected characters in numeric literal");
            }
            bool canBeFloat = false;
            if (i != source.text.end() && *i == '.') {
                tokenEnd++;
                i++;
                canBeFloat = true;
            }

            if (i != source.text.end() && *i == 'e') {
                tokenEnd++;
                i++;
                if (i != source.text.end() && (*i == '-' || *i == '+')) {
                    tokenEnd++;
                    i++;
                    canBeFloat = true;
                } else {
                    errors.push<LexerError>(ref.inSameLine(std::distance(i, source.text.begin())),
                                            "Unexpected characters in numeric literal");
                }
            }

            if (canBeFloat) { // pushing Float number
                while (i != source.text.end() && isdigit(*i)) {
                    tokenEnd++;
                    i++;
                }

                if (i != source.text.end() && isalpha(*i) && *i != 'e')
                    errors.push<LexerError>(ref.inSameLine(std::distance(i, source.text.begin())),
                                            "Unexpected characters in numeric literal");
                if (i != source.text.end() && *i == 'e') {
                    tokenEnd++;
                    i++;
                    if (i != source.text.end() && (*i == '-' || *i == '+')) {
                        tokenEnd++;
                        i++;
                    } else {
                        errors.push<LexerError>(ref.inSameLine(std::distance(i, source.text.begin())),
                                                "Unexpected characters in numeric literal");
                    }
                    while (i != source.text.end() && isdigit(*i)) {
                        tokenEnd++;
                        i++;
                    }
                }

                if (i != source.text.end() && isalpha(*i))
                    errors.push<LexerError>(ref.inSameLine(std::distance(i, source.text.begin())),
                                            "Unexpected characters in numeric literal");
                tokens.emplace_back(TokenType::FloatingPointLiteral, std::string(tokenBegin, tokenEnd),
                                    source.makeRef(i));
                tokenBegin = i;
                tokenEnd = i;
                i--;
                continue;
            }

            tokens.emplace_back(TokenType::IntegerLiteral, std::string(tokenBegin, tokenEnd), source.makeRef(i));
            tokenBegin = i;
            tokenEnd = i;
            i--;
            continue;
        }

        if (*i == '"') {
            i++;
            tokenBegin = i;
            tokenEnd = i;
            while (i != source.text.end() && *i != '"') {
                tokenEnd++;
                i++;
            }
            tokens.emplace_back(TokenType::StringLiteral, std::string(tokenBegin, tokenEnd), source.makeRef(i));
            tokenBegin = i;
            tokenEnd = i;

            if (i == source.text.end()) {
                errors.push<LexerError>(ref.inSameLine(source.text.size()), "No matching closing quote found");
                break;
            }

            continue;
        }

        tokenBegin = i;
        tokenEnd = i;

        // pushing Operators
        if (i + 1 != source.text.cend())
            if ((*i == '!' || *i == '=' || *i == '<' || *i == '>') && *(i + 1) == '=') {
                i++;
                tokenEnd++;
            }

        tokenEnd++;

        if (i + 1 != source.text.cend() && (*i == '-') && *(i + 1) == '>') {
            tokens.emplace_back(Special::Arrow, source.makeRef(i));
            i++;
            tokenBegin = tokenEnd;
            continue;
        }

        if (*i == ':') {
            tokens.emplace_back(Special::Colon, source.makeRef(i));
            tokenBegin = tokenEnd;
            continue;
        }

        auto tokenId = operators.find(makeStringView(tokenBegin, tokenEnd));
        if (tokenId != operators.end())
            tokens.emplace_back(tokenId->second, source.makeRef(i));

        tokenBegin = tokenEnd;
    }

    // adding a word or symbol at the end of a line
    if (tokenBegin != tokenEnd) {
        auto tokenId = keywords.find(makeStringView(tokenBegin, tokenEnd));
        auto tokenSrc = operators.find(makeStringView(tokenBegin, tokenEnd));
        if (tokenId != keywords.end())
            tokens.emplace_back(tokenId->second, source.makeRef(tokenBegin));
        else if (tokenSrc != operators.end())
            tokens.emplace_back(tokenId->second, source.makeRef(tokenBegin));
        else {
            auto pos = makeStringView(tokenBegin, tokenEnd).find_first_not_of(idAllowedChars);
            if (pos != std::string::npos) {
                errors.push<LexerError>(ref.inSameLine(std::distance(source.text.begin(), tokenBegin)),
                                        "Identifier cannot contain special characters");
            }
            tokens.emplace_back(TokenType::Identifier, std::string(tokenBegin, tokenEnd), source.makeRef(tokenBegin));
        }
    }

    tokens.emplace_back(Special::EndOfExpression, ref.inSameLine(source.text.length()));

    return tokens;
}
