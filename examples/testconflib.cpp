#include <iostream>
#include "conflib/conflib.h"


int main(int argc, char* argv[]) {
    std::map<std::string, std::string> alias;
    alias["o"] = "report.outputDir";
    alias["e"] = "report.expand";

    auto remaining = conflib::Initialize(argc, argv, alias);
    std::cout << conflib::Get<std::string>("report.theme", std::string("bad")) << std::endl;
    std::cout << conflib::Get<long>("param1", 999) << std::endl;
    std::cout << conflib::Get<std::string>("report.outputDir", "home") << std::endl;
    std::cout << conflib::Get<bool>("report.expand", false) << std::endl;

    for (std::string a : remaining) {
        std::cout << "remaining " << a << std::endl;
    }
}