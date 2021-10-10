#pragma once

#include <iostream>
#include <string>
#include <vector>

using StringVec = std::vector<std::string>;

std::ostream &operator<<(std::ostream &out, const StringVec &vec);