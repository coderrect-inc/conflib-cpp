#include <iostream>
#include "conflib/conflib.h"


int main(int argc, char* argv[]) {
    conflib::Initialize(argc, argv);
    std::cout << conflib::Get<std::string>("report.theme", std::string("bad")) << std::endl;
    std::cout << conflib::Get<long>("param1", 999) << std::endl;
}