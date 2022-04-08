#include "preprocessor/preprocessor.hpp"

using namespace preprocessor;

static void removeComments(utils::Source &source) {
    auto &lineIter = source.lines.begin();
    while (lineIter != source.lines.end()) {
        bool inStringApostrophe = false;
        bool inStringQuotes = false;
        size_t i = 0;
        std::string &str = lineIter->text;
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
        if (i != 0) {
            str = str.substr(0, i);
            lineIter++;
        } else {
            auto eraseIter = lineIter;
            lineIter++;
            source.lines.erase(eraseIter);
        }
    }
}

void Preprocessor::process(utils::Source &source) {
    removeComments(source);
}

// old implementation
StringVec Preprocessor::process(const StringVec &source) {
    StringVec result;
    for (const auto &str : source) {
        bool inStringApostrophe = false;
        bool inStringQuotes = false;
        size_t i = 0;
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
