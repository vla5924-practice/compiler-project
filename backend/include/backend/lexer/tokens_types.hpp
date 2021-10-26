#pragma once

namespace lexer {

enum class Keyword {
    Bool,
    Int,
    Float,
    Str,
    If,
    Else,
    Elif,
    Range,
    While,
    For,
    Break,
    Import,
    Continue,
    Definition,
    Return,
    Or,
    And,
    Not,
    In,
    True,
    None,
    False,
};

enum class Operator {
    Colon,
    Dot,
    Comma,
    Assign,
    Add,
    Sub,
    Mult,
    Div,
    Equal,
    NotEqual,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    LeftBrace,
    RightBrace,
    RectLeftBrace,
    RectRightBrace,
    Mod,
    Arrow,
};

enum class Special {
    Indentation,
    EndOfExpression,
};

enum class TokenType {
    Keyword,
    Identifier,
    Operator,
    Special,
    IntegerLiteral,
    FloatingPointLiteral,
    StringLiteral,
};

} // namespace lexer
