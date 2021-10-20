#include "preprocessor/preprocessor.hpp"

using namespace preprocessor;

StringVec Preprocessor::removeComments(const StringVec &source) {
    StringVec result;
    for (auto &str : source) {
        std::string resultStr;
        bool inStringLiteral = false;
        bool inString = false;
        int i = 0;
        for (; i < str.length(); i++) {
            // If we are inside the comment entity
            // then we can ignore any symbol until line ends
            // If symbol is quote
            // this means we are entering or leaving string literal
            if (str[i] == '\'')
                inStringLiteral = !inStringLiteral;
            if (str[i] == '"')
                inString = !inString;
            // If we are not inside string literal
            // then we can check if there is '//' sequence
            if (!inStringLiteral && !inString) {
                if (str[i] == '#') {
                    break;
                }
            }
        }
        if (i != 0)
            result.push_back(std::move(str.substr(0, i)));
    }
    return result;
}

StringVec Preprocessor::process(const StringVec &source) {
    return removeComments(source);
}
