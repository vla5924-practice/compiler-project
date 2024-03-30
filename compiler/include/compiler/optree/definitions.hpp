#pragma once

namespace optree {

enum class ArithBinOpKind {
    Unknown,
    AddI,
    SubI,
    MulI,
    DivI,
    AddF,
    SubF,
    MulF,
    DivF,
};

enum class ArithCastOpKind {
    Unknown,
    IntToFloat,
    FloatToInt,
    ExtI,
    TruncI,
    ExtF,
    TruncF,
};

enum class LogicBinOpKind {
    Unknown,
    Equal,
    NotEqual,
    AndI,
    OrI,
    LessI,
    GreaterI,
    LessEqualI,
    GreaterEqualI,
    LessF,
    GreaterF,
    LessEqualF,
    GreaterEqualF,
};

enum class LogicUnaryOpKind {
    Unknown,
    Not,
};

} // namespace optree
