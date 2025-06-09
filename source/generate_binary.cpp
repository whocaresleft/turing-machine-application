/*
 * Copyright (c) 2025 Biribo' Francesco
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <fstream>
#include <string>
#include <iostream>
#include <filesystem>
#include <regex>

#ifdef _WIN32
std::string extension = ".exe";
#endif

#ifdef linux
std::string extension = "";
#endif
/* This file will look for, given an integer K as command line, an executable named turing_machine_K in the current folder
 * If it does, no problemo, otherwise, it will look in the template folder and generate a .cpp file that declares
 * K as a constexpr for that program, so a turing machine and computation CAN be generated with not problem, it will then
 * try to build it and generate said binary, that can be later executed with "turing_machine_K <machine_json> <alphabet_json> <tape_json>"
 *
 * That binary will be the simplest execution of a turing machine computation, just setting all up, starting the execution and
 * then wait for it to end. If anyone needs to change that behaviour, they can feel free to modify "execution_template.cpp.in" as they please
 */
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
    if (std::string binary_name = "turing_machine_" + std::to_string(K) + extension; std::ifstream(binary_name)) {
        std::cout << "Binary for " << K << " template already exists" << std::endl;
        return 0;
    }
    std::filesystem::current_path("template");
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

    std::string build_command = "g++ -std=c++20 to_build_" + std::to_string(K) + ".cpp -o ../turing_machine_" + std::to_string(K) + extension;
    int result = std::system(build_command.c_str());
    if (result != 0) {
        std::cerr << "Error during build: " << result << std::endl;
        return result;
    }
    std::cout << "Build completed!";
    return 0;
}