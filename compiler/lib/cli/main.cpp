#include "compiler.hpp"
#include "options.hpp"

using namespace cli;

int main(int argc, char *argv[]) {
    Options opt;

    try {
        opt = std::move(parseArguments(argc, argv));
    } catch (const OptionsError &err) {
        std::cerr << err.what() << "\n";
        return 1;
    }

    Compiler compiler(opt);
    int ret = compiler.run();
    if (ret)
        return ret;

    if (opt.time) {
        for (const auto &[stage, elapsed] : compiler.measurements())
            std::cerr << stage << " Elapsed time: " << elapsed << " ms\n";
    }

    return 0;
}
