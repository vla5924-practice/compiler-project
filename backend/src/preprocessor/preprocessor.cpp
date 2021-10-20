#include "preprocessor/preprocessor.hpp"

using namespace preprocessor;

StringVec Preprocessor::removeComments(const StringVec &source) {
    StringVec result;
    for (auto &str : source) {
        std::string copy_str = str;
        size_t found_hash = str.find('#');
        if (found_hash == std::string::npos) {
            result.push_back(str);
            continue;
        }
        while (!copy_str.empty()) {
            size_t found_smth = copy_str.find_first_of("'\"#");
            if (copy_str[found_smth] == '#') {
                std::string subresult_str = copy_str.substr(0, found_smth);
                copy_str.clear();
                if (!subresult_str.empty())
                    result.push_back(std::move(subresult_str));
            } else {
                size_t found_second = copy_str.find(copy_str[found_smth], found_smth + 1);
                copy_str = copy_str.substr(found_second + 1);
            }
        }
    }
    return result;
}

StringVec Preprocessor::process(const StringVec &source) {
    return removeComments(source);
}
