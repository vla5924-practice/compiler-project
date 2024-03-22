#include "preprocessor/preprocessor.hpp"

using namespace preprocessor;
using utils::SourceFile;

namespace {
SourceFile removeComments(const SourceFile &source) {
    SourceFile result;
    for (const auto &str : source) {
        bool inStringApostrophe = false;
        bool inStringQuotes = false;
        size_t i = 0;
        for (; i < str.text.length(); i++) {
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
            result.emplace_back(std::move(str.text.substr(0, i)), str.ref);
    }
    return result;
}

} // namespace

SourceFile Preprocessor::process(const SourceFile &source) {
    return removeComments(source);
}
