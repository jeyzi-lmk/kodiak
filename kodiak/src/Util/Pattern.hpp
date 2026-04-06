#pragma once

namespace Pattern {
    struct Compiler {
        std::vector<BYTE> bytes;
        std::string mask;
    };

    __forceinline Compiler compile(const std::string &signature) {
        Compiler compiler;

        const char* str = signature.c_str();
        while (*str) {
            if (*str == ' ') { ++str; continue; }
            if (*str == '?') {
                compiler.bytes.push_back(0); compiler.mask.push_back('?'); ++str;
                if (*str == '?') ++str;
            } else {
                compiler.bytes.push_back(static_cast<BYTE>(std::strtoul(str, nullptr, 16)));
                compiler.mask.push_back('x');
                while (*str && *str != ' ') ++str;
            }
        }

        return compiler;
    }

    __forceinline static bool compare(const BYTE* data, const BYTE* mask, const char* maskString) {
        for (; *maskString; ++maskString, ++data, ++mask) {
            if (*maskString == 'x' && *data != *mask) {
                return false;
            }
        }
        return (*maskString) == 0;
    }
}
