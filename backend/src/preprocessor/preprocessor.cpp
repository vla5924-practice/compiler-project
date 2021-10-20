#include "preprocessor/preprocessor.hpp"

using namespace preprocessor;

StringVec Preprocessor::removeComments(const StringVec &source) {
    StringVec result;
    for (const auto &str : source) {
        std::string resultStr;
        bool inStringApostrophe = false;
        bool inStringQuotes = false;
        int i = 0;
        for (; i < str.length(); i++) {
            if (str[i] == '\'')
                inStringApostrophe = !inStringApostrophe;
            if (str[i] == '"')
                inStringQuotes = !inStringQuotes;
            if (!inStringApostrophe && !inStringQuotes) {
                if (str[i] == '#') {
                    break;
                }
            }
        }
        if (i != 0)
            result.emplace_back(std::move(str.substr(0, i)));
    }
    return result;
}

StringVec Preprocessor::process(const StringVec &source) {
    return removeComments(source);
}
