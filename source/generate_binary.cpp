#include <fstream>
#include <string>
#include <iostream>
#include <filesystem>
#include <regex>
int main(int argc, char** argv) {
    if (argc < 2) {
        std:: cerr << "Need to pass \'K\' as command line parameter";
        return -1;
    }
    int K = atoi(argv[1]);
    if (K < 1) {
        std:: cerr << "\'K\' needs to be positive";
        return -1;
    }
    if (std::string binary_name = "turing_machine_" + std::to_string(K) + ".exe"; std::ifstream(binary_name)) {
        std::cout << "Binary for " << K << " template already exists" << std::endl;
        return 0;
    }
    std::filesystem::current_path("../source/template");
    // Create it otherwise
    std::ifstream template_file("execution_template.cpp.in");
    std::string template_code(
        (std::istreambuf_iterator<char>(template_file)),
        std::istreambuf_iterator<char>()
    );

    template_code = std::regex_replace( template_code, std::regex(R"(\$\{K\})"), std::to_string(K));

    std::ofstream out("to_build_" + std::to_string(K) + ".cpp");
    out << template_code;
    template_file.close();
    out.close();

    std::string build_command = "g++ -std=c++20 to_build_" + std::to_string(K) + ".cpp -o turing_machine_" + std::to_string(K) + ".exe";
    int result = std::system(build_command.c_str());
    if (result != 0) {
        std::cerr << "Error during build: " << result << std::endl;
        return result;
    }
    std::cout << "Build completed!";
    return 0;
}