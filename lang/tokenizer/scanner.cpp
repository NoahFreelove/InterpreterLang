#include "scanner.h"

#include <cstring>

token * lang::scanner::get_digit(char first_num) {
std::vector<char> chars = std::vector<char>();
    chars.push_back(first_num);
    while (isdigit(peek()) && !past_end()) {
        chars.push_back(next());
    }
    char c = peek();

    std::string str(chars.begin(), chars.end());
    const char* cstr = strcpy(new char[str.length() + 1], str.c_str());
    if(c != '.') {
        if(c != 'l' && c != 'f' && c != 'd' &&  peek_str(7) != "ulong64") {
            return new token(INT, cstr, line, std::stoi(str));
        }
        // check if long
        if(c == 'l' && peek_str(7) != "ulong64") {
            next();
            return new token(LONG, cstr, line, std::stol(str));
        }
        // Check if ulong64
        if(peek_str(7) == "ulong64" && c != 'l' && c != 'f' && c != 'd') {
            for (int i = 0; i < 7; ++i) {
                next();
            }
            return new token(ULONG64, cstr, line, std::stoull(str));
        }
        // check if float but no decimal
        if (peek() == 'f' && c != 'l' && peek_str(7) != "ulong64") {
            next();
            return new token(FLOAT, cstr, line, std::stof(str));
        }
        // check if double but no decimal
        if (peek() == 'd' && c != 'l' && peek_str(7) != "ulong64") {
            next();
            return new token(DOUBLE, cstr, line, std::stod(str));
        }
    }
    else{
        next();
        std::vector<char> decimals = std::vector<char>();
        while (isdigit(peek()) && !past_end()) {
            decimals.push_back(next());
        }
        std::string str(chars.begin(), chars.end());
        std::string str2(decimals.begin(), decimals.end());
        const char* cstr = strcpy(new char[str.length() + 1], str.c_str());

        // check if float
        if(peek() != 'd') {

            return new token(FLOAT, cstr, line, std::stof(str + "." + str2));
        }
        // check if double
        if(peek() == 'd') {
            next();
            return new token(DOUBLE, cstr, line, std::stod(str + "." + str2));

        }
        err = true;
        interpreter::error("Invalid decimal number: " + std::string(chars.begin(), chars.end()) + peek());

    }
    err = true;
    interpreter::error("Invalid number: " + std::string(chars.begin(), chars.end()) + peek());

    return nullptr;
}

token * lang::scanner::get_identifier(char c) {
    // Keep adding to the identifier until we hit a non-alphanumeric character
            std::vector<char> chars = std::vector<char>();
            chars.push_back(c);
            while (isalnum(peek()) && !past_end() && !isspace(peek())) {
                chars.push_back(next());
            }
            std::string str(chars.begin(), chars.end());
            // make copy of str on heap
            const char* cstr = strcpy(new char[str.length() + 1], str.c_str());

            //std::cout << "CHECKING IDENTIFIER: " << str << std::endl;

    // Check if keyword
    if(str == "if") {
        return new token(IF, cstr, line);
    }
    if(str == "else") {
        return new token(ELSE, cstr, line);
    }if(str == "else-if") {
        return new token(ELSE_IF, cstr, line);
    }
    if(str == "for") {
        return new token(FOR, cstr, line);
    }
    if(str == "while") {
        return new token(WHILE, cstr, line);
    }
    if(str == "struct") {
        return new token(STRUCT, cstr, line);
    }
    if(str == "class") {
        return new token(CLASS, cstr, line);
    }
    if(str == "private") {
        return new token(PRIVATE, cstr, line);
    }
    if(str == "public") {
        return new token(PUBLIC, cstr, line);
    }
    if(str == "true") {
        return new token(TRUE, cstr, line);
    }
    if(str == "false") {
        return new token(FALSE, cstr, line);
    }
    if(str == "return") {
        return new token(RETURN, cstr, line);
    }
    if(str == "function") {
        return new token(FUNCTION, cstr, line);
    }
    if(str == "null") {
        return new token(NULL_LANG, cstr, line);
    }
    if(str == "var") {
        return new token(VAR, cstr, line);
    }
    if(str == "as") {
        return new token(AS, cstr, line);
    }
    if(str == "byval") {
        return new token(BYVAL, cstr, line);
    }
    if(str == "discard") {
        return new token(DISCARD, cstr, line);
    }
    if(str == "persistent") {
        return new token(PERSISTENT, cstr, line);
    }
    if(str == "and") {
        return new token(AND, cstr, line);
    }
    if(str == "or") {
        return new token(OR, cstr, line);
    }
    if(str == "not") {
        return new token(BANG, cstr, line);
    }
    if(str == "xor") {
        return new token(XOR, cstr, line);
    }
    if(str == "print") {
        return new token(PRINT, cstr, line);
    }
    return new token(IDENTIFIER, cstr, line);
}

void lang::scanner::skip(int amount) {
    current+= amount;
    if(current >= end) {
        current = end;
    }
}

token* lang::scanner::trigger_error(char c) {
    err = true;
    interpreter::error("Unexpected character: " + std::string(1, c));
    return nullptr;
}

token * lang::scanner::get_string() {

}


token * lang::scanner::parse_token() {
    char c = next();
    if(at_end() && c == '\\') {
        return nullptr;
    }
            switch (c) {
                case ' ':
                    return nullptr;
                case '\n':
                    line++;
                    return nullptr;
                case '\r':
                    return nullptr;
                case '\t':
                    return nullptr;
                case '(':
                    return new token(LEFT_PAREN, "(", line, nullptr);
                case ')':
                    return new token(RIGHT_PAREN, "(", line, nullptr);
                case '{':
                    return new token(LEFT_BRACE, "{", line, nullptr);
                case '}':
                    return new token(RIGHT_BRACE, "}", line, nullptr);
                case '[':
                    return new token(LEFT_BRACKET, "[", line, nullptr);
                case ']':
                    return new token(RIGHT_BRACKET, "]", line, nullptr);
                case ';':
                    return new token(SEMICOLON, ";", line, nullptr);
                case ',':
                    return new token(COMMA, ",", line, nullptr);
                case '.':
                    return new token(DOT, ".", line, nullptr);
                case '!': {
                    if(peek() == '=') {
                        skip();
                        return new token(BANG_EQUAL, "!=", line, nullptr);
                    }
                    return new token(BANG, "!", line, nullptr);
                }
                case '=': {
                    if(peek(1) == '=') {
                        skip(2);
                        return new token(TEQUAL, "==", line, nullptr);
                    }
                    if(peek() == '=') {
                        skip();
                        return new token(DEQUAL, "=", line, nullptr);
                    }
                    return new token(EQUAL, "=", line, nullptr);
                }
                case '>': {
                    if(peek() == '=') {
                        skip();
                        return new token(GTE, ">=", line, nullptr);
                    }
                    return new token(GT, ">", line, nullptr);
                }
                case '<': {
                    if(peek() == '=') {
                        skip();
                        return new token(LTE, "<=", line, nullptr);
                    }
                    return new token(LT, "<", line, nullptr);
                }
                case '+':
                    return new token(PLUS, "+", line, nullptr);
                case '-':
                    return new token(MINUS, "-", line, nullptr);
                case '*':
                    return new token(STAR, "*", line, nullptr);
                case '/': {
                    // We ignore comments
                    if(peek() == '/') {
                        while (!past_end())
                            next();
                        return nullptr;
                    }
                    if(peek() == 'i') {
                        skip();
                        return new token(SLASHI, "/i", line, nullptr);
                    }
                    return new token(SLASH, "/", line, nullptr);
                }
                case '&': {
                    if(peek() == '&') {
                        skip();
                        return new token(AND, "&&", line, nullptr);
                    }
                    return new token(ID_GRAB, "&", line, nullptr);
                }
                case '|': {
                    if(peek() == '|') {
                        skip();
                        return new token(OR, "||", line, nullptr);
                    }
                    return trigger_error(c);

                }
                case '^': {
                    if(peek() == '^') {
                        skip();
                        return new token(XOR, "^^", line, nullptr);
                    }
                    return new token(EXPONENT, "^", line, nullptr);
                }

                default: {
                    if(isdigit(c)) {
                        return get_digit(c);
                    }
                    if(c == '"') {
                        return get_string();
                    }
                    if(isalpha(c)) {
                        return get_identifier(c);
                    }
                    return trigger_error(c);
                }
            }
}

lang::scanner::scanner() = default;

std::vector<token *> lang::scanner::scan_line(std::string *data) {
    scanner::data = data;
    if(scanner::data == nullptr)
        return {};
    line++;
    end = data->length();
    if(in_multi_line_expression && data->empty()) {
        in_multi_line_expression = false;
        return multi_line_expression;
    }
    if(!in_multi_line_expression) {
        multi_line_expression = {};
    }
    if(last_char() == '\\') {
        in_multi_line_expression = true;
    }
    std::vector<token*> tokens = std::vector<token*>();
    err = false;
    for (current = 0; current < end;) {
        if(err)
            break;
        auto* t = parse_token();
        if(t != nullptr) {
            if(in_multi_line_expression) {
                multi_line_expression.push_back(t);
            }
            else {
                tokens.push_back(t);
            }
        }
    }

    if(err) {
        return {};
    }


    return tokens;
}

bool lang::scanner::in_multi_line() {
    return in_multi_line_expression;
}
