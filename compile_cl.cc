#include <ctype.h>

#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

namespace {

std::string FileNameToVar(const std::string& file_name) {
    std::string var = "k";
    bool start = true;
    for (const char ch : file_name) {
        if (isalnum(ch)) {
            if (start) {
                var += toupper(ch);
            } else {
                var += ch;
            }
            start = false;
        } else {
            start = true;
        }
    }
    return var;
}

std::string ClFileNameToBaseName(const std::string& cl_file) {
    std::string base_name = cl_file;
    for (char& ch : base_name) {
        if (ch == '.') ch = '_';
    }
    return base_name;
}

}  // namespace

#define RED "\e[0;31m"
#define GREEN "\e[0;92m"
#define NC "\e[0m"

int main(int argc, char *argv[]) {
    cl::Context context(CL_DEVICE_TYPE_GPU);
    for (int i = 1; i < argc; i++) {
        // Compile cl file.
        const std::string cl_file = argv[i];
        std::ifstream stream(cl_file);
        if (!stream.is_open()) {
            std::cerr << RED "Failed to open '" << cl_file << "'." NC << std::endl;
            continue;
        }
        const std::string source_code{
            std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
        cl::Program program(context, source_code);
        try {
            program.build();
        } catch (cl::Error err) {
            std::cerr << RED "Failed to compile '" << cl_file << "': " << err.what()
                << " (" << err.err() << ")." NC << std::endl;
            continue;
        }
        const std::string var_name = FileNameToVar(cl_file);
        const std::string base_name = ClFileNameToBaseName(cl_file);
        // Generate .h
        const std::string cl_h_file = base_name + ".h";
        std::cout << GREEN "Generating '" << cl_h_file << "' ..." NC << std::endl;
        std::ofstream h_stream(cl_h_file);
        if (!h_stream.is_open()) {
            std::cerr << RED "Failed to write to '" << cl_h_file << "'." NC << std::endl;
            continue;
        }
        h_stream << "#ifdef __cplusplus\n";
        h_stream << "extern \"C\" {\n";
        h_stream << "#endif\n";
        h_stream << "extern const char " << var_name << "[];\n";
        h_stream << "#ifdef __cplusplus\n";
        h_stream << "}\n";
        h_stream << "#endif";
        h_stream.close();
        // Generate .c
        const std::string cl_c_file = base_name + ".c";
        std::cout << GREEN "Generating '" << cl_c_file << "' ..." NC << std::endl;
        std::ofstream c_stream(cl_c_file);
        if (!c_stream.is_open()) {
            std::cerr << RED "Failed to write to '" << cl_c_file << "'." NC << std::endl;
            continue;
        }
        c_stream << "const char " << var_name << "[] =";
        bool new_line = true;
        for (const char ch : source_code) {
            if (new_line) {
                c_stream << "\n    \"";
                new_line = false;
            }
            switch (ch) {
                case '\r':
                    break;
                case '\n':
                    c_stream << "\\n\"";
                    new_line = true;
                    break;
                case '"':
                case '\\':
                    c_stream << '\\';
                default:
                    c_stream << ch;
            }
        }
        if (new_line) {
            c_stream << ";\n";
        } else {
            c_stream << "\";\n";
        }
        c_stream.close();
    }
}
