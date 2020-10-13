#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include "conflib/conflib.h"



static void InitWithEnvCmdlineOpt() {
    // set up environment variable CODERRECT_CMDLINE_OPTS
    std::string data = R"(
    {
        "Opts": [
            "-report.theme=red",
            "-o",
            "/tmp",
            "abc.c",
            "def.c"
        ]
    }
)";
    setenv("CODERRECT_CMDLINE_OPTS", data.c_str(), 1);

    std::map<std::string, std::string> alias;
    alias["o"] = "report.outputDir";
    alias["e"] = "report.expand";

    auto remaining = conflib::Initialize(alias, true);
    std::cout << "report.theme=" << conflib::Get<std::string>("report.theme", std::string("bad")) << std::endl;
    std::cout << "param1=" << conflib::Get<long>("param1", 999) << std::endl;
    std::cout << "report.outputDir=" << conflib::Get<std::string>("report.outputDir", "home") << std::endl;
    std::cout << "report.expand=" << conflib::Get<bool>("report.expand", false) << std::endl;

    for (std::string a : remaining) {
        std::cout << "remaining " << a << std::endl;
    }
}

/*
int main(int argc, char* argv[]) {
//    InitWithEnvCmdlineOpt();
//    return 0;

    std::map<std::string, std::string> alias;
    alias["o"] = "report.outputDir";
    alias["e"] = "report.expand";

    auto remaining = conflib::Initialize(alias, false, argc, argv);
    std::vector<std::string> loggers;
    conflib::Get("loggers", loggers);
    std::cout << "loggers: ";
    for (auto itr = loggers.begin(); itr != loggers.end(); ++itr) {
        std::cout << *itr << ", ";
    }
    std::cout << std::endl;

    std::cout << conflib::Get<std::string>("report.theme", std::string("bad")) << std::endl;
    std::cout << conflib::Get<long>("param1", 999) << std::endl;
    std::cout << conflib::Get<std::string>("report.outputDir", "home") << std::endl;
    std::cout << conflib::Get<bool>("report.expand", false) << std::endl;

    for (std::string a : remaining) {
        std::cout << "remaining " << a << std::endl;
    }
}
*/


int main(int argc, char* argv[]) {
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    std::cout << cwd << '\n';

    std::map<std::string, std::string> alias{};
    conflib::Initialize(alias, false, argc, argv);
    std::cout << conflib::Get<std::string>("threadAPIProfiles.apr.a", "notfound") << '\n';
}