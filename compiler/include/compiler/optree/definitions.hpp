#pragma once

namespace optree {

enum class ArithBinOpKind {
    Unknown,
    Add,
    Sub,
    Mult,
    Div,
    FAdd,
    FSub,
    FMult,
    FDiv,
};

enum class LogicBinOpKind {
    And,
    Or,
    Equal,
    NotEqual,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
};

} // namespace optree
