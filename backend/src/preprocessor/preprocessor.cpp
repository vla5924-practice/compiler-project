#include "preprocessor/preprocessor.hpp"

using namespace preprocessor;

StringVec Preprocessor::removeComments(const StringVec &source) {
    StringVec result;
    for (auto &str : source) {
        std::string resultStr;
        bool inComment = false;
        bool inStringLiteral = false;
        bool skip;
        for (char sym : str) {
            skip = false;
            // If we are inside the comment entity
            // then we can ignore any symbol until line ends
            // If symbol is quote
            // this means we are entering or leaving string literal
            if (sym == '\'')
                inStringLiteral = !inStringLiteral;
            // If we are not inside string literal
            // then we can check if there is '//' sequence
            if (!inStringLiteral) {
                if (sym == '#') {
                    break;
                }
            }
            resultStr.push_back(sym);
        }
        if (!resultStr.empty())
            result.push_back(std::move(resultStr));
    }
    return result;
}

StringVec Preprocessor::process(const StringVec &source) {
    return removeComments(source);
}
