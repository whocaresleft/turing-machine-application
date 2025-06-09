#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <regex>
#include <cstdlib>

int main(int argc, char** argv) {
    //if (argc < 2) {
    //    std:: cerr << "Need to pass \'K\' as command line parameter";
    //    return -1;
    //}
    //int K = atoi(argv[1]);
    //if (K < 1) {
    //    std:: cerr << "\'K\' needs to be positive";
    //    return -1;
    //}
int K = 3;
    if (std::string binary_name = "turing_machine_" + std::to_string(K) + ".exe"; std::ifstream(binary_name)) {
        std::cout << "Binary for " << K << " template already exists" << std::endl;
        return 0;
    }

    // Create it otherwise
    std::ifstream template_file("template/template.cpp.in");
    std::string template_code(
        (std::istreambuf_iterator<char>(template_file)),
        std::istreambuf_iterator<char>()
    );

    template_code = std::regex_replace( template_code, std::regex(R"(\$\{K\})"), std::to_string(K));

    std::ofstream out("to_build_" + std::to_string(K) + ".cpp");
    out << template_code;
    template_file.close();
    out.close();

    std::string cmd = "cmake -DK=" + std::to_string(K) + " -B turing_machine_" + std::to_string(K) + " && cmake --build turing_machine_" + std::to_string(K);
    std::system(cmd.c_str());
}