#include "scanner.h"

#include <cstring>
#include <memory>

std::shared_ptr<token> lang::scanner::get_digit(char first_num) {
    std::vector<char> chars = std::vector<char>();
    chars.push_back(first_num);
    while (isdigit(peek()) && !past_end()) {
        chars.push_back(next());
    }
    char c = peek();

    std::string str(chars.begin(), chars.end());
    const char* cstr = strcpy(new char[str.length() + 1], str.c_str());
    if(c != '.') {
        if(c != 'l' && c != 'f' && c != 'd' &&  peek_str(3) != "ull") {
            // if its too big, return error
            if(str.length() > 10) {
                err = true;
                interpreter::error("Number too big: " + std::string(chars.begin(), chars.end()) + peek() + " for int");
                return nullptr;
            }
            return std::make_shared<token>(INT, cstr, line, std::stoi(str));
        }
        // check if long
        if(c == 'l' && peek_str(3) != "ull") {
            next();
            if(str.length() > 19) {
                err = true;
                interpreter::error("Number too big: " + std::string(chars.begin(), chars.end()) + peek() + " for long");
                return nullptr;
            }
            return std::make_shared<token>(LONG, cstr, line, std::stol(str));
        }
        // Check if ulong64
        if(peek_str(3) == "ull" && c != 'l' && c != 'f' && c != 'd') {
            for (int i = 0; i < 3; ++i) {
                next();
            }
            if(str.length() > 20) {
                err = true;
                interpreter::error("Number too big: " + std::string(chars.begin(), chars.end()) + peek() + " for ulong64");
                return nullptr;
            }
            return std::make_shared<token>(ULONG64, cstr, line, std::stoull(str));
        }
        // check if float but no decimal
        if (peek() == 'f' && c != 'l' && peek_str(3) != "ull") {
            next();
            // if its too big, return error
            if(str.length() > 10) {
                err = true;
                interpreter::error("Number too big: " + std::string(chars.begin(), chars.end()) + peek() + " for float");
                return nullptr;
            }
            return std::make_shared<token>(FLOAT, cstr, line, std::stof(str));
        }
        // check if double but no decimal
        if (peek() == 'd' && c != 'l' && peek_str(3) != "ull") {
            next();
            // if its too big, return error
            if(str.length() > 19) {
                err = true;
                interpreter::error("Number too big: " + std::string(chars.begin(), chars.end()) + peek() + " for double");
                return nullptr;
            }
            return std::make_shared<token>(DOUBLE, cstr, line, std::stod(str));
        }
    }
    else{
        next();
        std::vector<char> decimals = std::vector<char>();
        while (isdigit(peek()) && !past_end()) {
            decimals.push_back(next());
        }
        std::string str2(decimals.begin(), decimals.end());
        const char* cstr = strcpy(new char[str.length() + 1], str.c_str());
        // check if float
        if(peek() == 'f') {
            next();
            // if its too big, return error
            if(str.length() + str2.length() > 10) {
                err = true;
                interpreter::error("Number too big: " + std::string(chars.begin(), chars.end()) + peek() + " for float");
                return nullptr;
            }
            return std::make_shared<token>(FLOAT, cstr, line, std::stof(str + "." + str2));
        }

        // assume if its not float that its double
        if(peek() == 'd')
            next();

        // if its too big, return error
        if(str.length() + str2.length() > 19) {
            err = true;
            interpreter::error("Number too big: " + std::string(chars.begin(), chars.end()) + peek() + " for double");
            return nullptr;
        }
        return std::make_shared<token>(DOUBLE, cstr, line, std::stod(str + "." + str2));

    }
    err = true;
    interpreter::error("Invalid number: " + std::string(chars.begin(), chars.end()) + peek());

    return nullptr;
}

std::shared_ptr<token> lang::scanner::get_identifier(char c) {
    // Keep adding to the identifier until we hit a non-alphanumeric character
            std::vector<char> chars = std::vector<char>();
            chars.push_back(c);
            while ((isalnum(peek()) || peek() == '_') && !past_end() && !isspace(peek())) {
                chars.push_back(next());
            }
            std::string str(chars.begin(), chars.end());
            // make copy of str on heap
            const char* cstr = strcpy(new char[str.length() + 1], str.c_str());

            //std::cout << "CHECKING IDENTIFIER: " << str << std::endl;

    // Check if keyword
    if(str == "if") {
        return std::make_shared<token>(IF, cstr, line);
    }
    if(str == "endif") {
        return std::make_shared<token>(END_IF, cstr, line);
    }
    if(str == "else") {
        return std::make_shared<token>(ELSE, cstr, line);
    }
    if(str == "elseif") {
        return std::make_shared<token>(ELSE_IF, cstr, line);
    }

    if(str == "for") {
        return std::make_shared<token>(FOR, cstr, line);
    }
    if(str == "while") {
        return std::make_shared<token>(WHILE, cstr, line);
    }
    if(str == "struct") {
        return std::make_shared<token>(STRUCT, cstr, line);
    }
    if(str == "class") {
        return std::make_shared<token>(CLASS, cstr, line);
    }
    if(str == "private") {
        return std::make_shared<token>(PRIVATE, cstr, line);
    }
    if(str == "public") {
        return std::make_shared<token>(PUBLIC, cstr, line);
    }
    if(str == "true") {
        return std::make_shared<token>(TRUE, cstr, line);
    }
    if(str == "false") {
        return std::make_shared<token>(FALSE, cstr, line);
    }
    if(str == "return") {
        return std::make_shared<token>(RETURN, cstr, line);
    }
    if(str == "proc" || str == "def" || str == "func" || str == "function" || str == "procedure" || str == "method") {
        return std::make_shared<token>(PROC_KEYW, cstr, line);
    }
    if(str == "endproc" || str == "enddef" || str == "endfunc" || str == "endfunction" || str == "endprocedure" || str == "endmethod") {
        return std::make_shared<token>(END_PROC, cstr, line);
    }
    if(str == "macro") {
        return std::make_shared<token>(MACRO, cstr, line);
    }
    if(str == "endloop") {
        return std::make_shared<token>(END_LOOP, cstr, line);
    }
    if(str == "final") {
        return std::make_shared<token>(FINAL, cstr, line);
    }
    if(str == "native") {
        return std::make_shared<token>(NATIVE, cstr, line);
    }
    if(str == "byval") {
        return std::make_shared<token>(BYVAL, cstr, line);
    }
    if(str == "discard") {
        return std::make_shared<token>(DISCARD, cstr, line);
    }
    if(str == "persistent") {
        return std::make_shared<token>(PERSISTENT, cstr, line);
    }
    if(str == "break") {
        return std::make_shared<token>(BREAK, cstr, line);
    }
    if(str == "continue") {
        return std::make_shared<token>(CONTINUE, cstr, line);
    }
    if(str == "dowhile")
        return std::make_shared<token>(DO_WHILE, cstr, line);
    if (str == "until")
        return std::make_shared<token>(UNTIL, cstr, line);
    if (str == "dountil")
        return std::make_shared<token>(DO_UNTIl, cstr, line);
    if (str == "loop")
        return std::make_shared<token>(LOOP, cstr, line);
    if(str == "foreach")
        return std::make_shared<token>(FOREACH, cstr, line);
    if(str == "and") {
        return std::make_shared<token>(AND, cstr, line);
    }
    if(str == "or") {
        return std::make_shared<token>(OR, cstr, line);
    }
    if(str == "not") {
        return std::make_shared<token>(BANG, cstr, line);
    }
    if(str == "xor") {
        return std::make_shared<token>(XOR, cstr, line);
    }
    if(str == "print") {
        return std::make_shared<token>(PRINT, cstr, line);
    }
    if(str == "cast") {
        return std::make_shared<token>(CAST, cstr, line);
    }
    if(str == "input") {
        return std::make_shared<token>(INPUT, cstr, line);
    }
    if(str == "rawinput") {
        return std::make_shared<token>(RAWINPUT, cstr, line);
    }
    if(str == "typeof") {
        return std::make_shared<token>(TYPEOF, cstr, line);
    }
    if(str == "sizeof") {
        return std::make_shared<token>(SIZEOF, cstr, line);
    }
    if(str == "delete") {
        return std::make_shared<token>(DELETE, cstr, line);
    }
    if(str == "define") {
        return std::make_shared<token>(DEFINE, cstr, line);
    }
    if(str == "undefine") {
        return std::make_shared<token>(UNDEFINE, cstr, line);
    }
    if(str == "isdefined") {
        return std::make_shared<token>(ISDEFINED, cstr, line);
    }
    if(str == "import") {
        return std::make_shared<token>(IMPORT, cstr, line);
    }
    if(str == "id") {
        return std::make_shared<token>(ID, cstr, line);
    }
    if(str == "assert") {
        return std::make_shared<token>(ASSERT, cstr, line);
    }
    if(str == "int") {
        return std::make_shared<token>(INT_KEYW, cstr, line);
    }
    if(str == "float") {
        return std::make_shared<token>(FLOAT_KEYW, cstr, line);
    }
    if(str == "double") {
        return std::make_shared<token>(DOUBLE_KEYW, cstr, line);
    }
    if(str == "long") {
        return std::make_shared<token>(LONG_KEYW, cstr, line);
    }
    if(str == "string") {
        return std::make_shared<token>(STRING_KEYW, cstr, line);
    }
    if(str == "char") {
        return std::make_shared<token>(CHAR_KEYW, cstr, line);
    }
    if(str == "bool") {
        return std::make_shared<token>(BOOL_KEYW, cstr, line);
    }
    if(str == "ulong64") {
        return std::make_shared<token>(ULONG64_KEYW, cstr, line);
    }
    if(str == "void") {
        return std::make_shared<token>(NOTHING_KEYW, cstr, line);
    }
    if(str == "type") {
        return std::make_shared<token>(TYPE_KEYW, cstr, line);
    }
    if(str == "not")
        return std::make_shared<token>(BANG, "!", line, nullptr);
    if(str == "is")
        return std::make_shared<token>(TEQUAL, "===", line, nullptr);
    if(str == "isnt")
        return std::make_shared<token>(BANG_EQUAL, "!=", line, nullptr);


    return std::make_shared<token>(IDENTIFIER, cstr, line);
}

void lang::scanner::skip(int amount) {
    current+= amount;
    if(current >= end) {
        current = end;
    }
}

std::shared_ptr<token> lang::scanner::trigger_error(char c) {
    err = true;
    interpreter::error("Unexpected character: " + std::string(1, c));
    return nullptr;
}

std::shared_ptr<token> lang::scanner::get_string() {
    char c = next();
    std::vector<char> chars = std::vector<char>();

    while (c != '"' && !at_end()) {
        chars.push_back(c);
        c = next();
    }

    if(c != '"') {
        err = true;
        interpreter::error("Unterminated string");
        return nullptr;
    }
    std::string str(chars.begin(), chars.end());
    const char* cstr = strcpy(new char[str.length() + 1], str.c_str());
    return std::make_shared<token>(STRING, cstr, line, std::string(cstr));
}


std::shared_ptr<token> lang::scanner::parse_token() {
    char c = next();

    if(in_multi_comment()) {
        if(c == '*' && peek() == '/') {
            skip();
            in_multi_com = false;
        }
        return nullptr;
    }

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
                case '\t':
                    return nullptr;
                case '(':
                    return std::make_shared<token>(LEFT_PAREN, "(", line, nullptr);
                case ')':
                    return std::make_shared<token>(RIGHT_PAREN, ")", line, nullptr);
                case '{':
                    return std::make_shared<token>(LEFT_BRACE, "{", line, nullptr);
                case '}':
                    return std::make_shared<token>(RIGHT_BRACE, "}", line, nullptr);
                case '[':
                    return std::make_shared<token>(LEFT_BRACKET, "[", line, nullptr);
                case ']':
                    return std::make_shared<token>(RIGHT_BRACKET, "]", line, nullptr);
                case ';':
                    return std::make_shared<token>(SEMICOLON, ";", line, nullptr);
                case ',':
                    return std::make_shared<token>(COMMA, ",", line, nullptr);
                case '.':
                    return std::make_shared<token>(DOT, ".", line, nullptr);
                case '!':
                    case '~': {
                    if(peek() == '=') {
                        skip();
                        return std::make_shared<token>(BANG_EQUAL, "!=", line, nullptr);
                    }
                    return std::make_shared<token>(BANG, "!", line, nullptr);
                }
                case '=': {
                    if(peek(1) == '=') {
                        skip(2);
                        return std::make_shared<token>(TEQUAL, "===", line, nullptr);
                    }
                    if(peek() == '=') {
                        skip();
                        return std::make_shared<token>(DEQUAL, "==", line, nullptr);
                    }
                    return std::make_shared<token>(EQUAL, "=", line, nullptr);
                }
                case '>': {
                    if(peek() == '=') {
                        skip();
                        return std::make_shared<token>(GTE, ">=", line, nullptr);
                    }
                    else if(peek() == '>') {
                        skip();
                        return std::make_shared<token>(FORWARD, ">>", line, nullptr);
                    }
                    return std::make_shared<token>(GT, ">", line, nullptr);
                }
                case '<': {
                    if(peek() == '=') {
                        skip();
                        return std::make_shared<token>(LTE, "<=", line, nullptr);
                    }
                    return std::make_shared<token>(LT, "<", line, nullptr);
                }
                case '+':
                    return std::make_shared<token>(PLUS, "+", line, nullptr);
                case '-':
                    return std::make_shared<token>(MINUS, "-", line, nullptr);
                case '*':
                    return std::make_shared<token>(STAR, "*", line, nullptr);
                case '%':
                    return std::make_shared<token>(MOD, "*", line, nullptr);

                case '/': {
                    // We ignore comments
                    if(peek() == '/') {
                        while (!past_end())
                            next();
                        return nullptr;
                    }
                    if(peek() == '*') {
                        in_multi_com = true;
                        next();
                        return nullptr;
                    }
                    if(peek() == 'i') {
                        skip();
                        return std::make_shared<token>(SLASHI, "/i", line, nullptr);
                    }
                    return std::make_shared<token>(SLASH, "/", line, nullptr);
                }
                case ':': {
                    return std::make_shared<token>(COLON, "&&", line, nullptr);
                }
                case '&': {
                    if(peek() == '&') {
                        skip();
                        return std::make_shared<token>(AND, "&&", line, nullptr);
                    }
                    return std::make_shared<token>(ID_GRAB, "&", line, nullptr);
                }
                case '|': {
                    if(peek() == '|') {
                        skip();
                        return std::make_shared<token>(OR, "||", line, nullptr);
                    }
                    return trigger_error(c);

                }
                case '^': {
                    if(peek() == '^') {
                        skip();
                        return std::make_shared<token>(XOR, "^^", line, nullptr);
                    }
                    return std::make_shared<token>(EXPONENT, "^", line, nullptr);
                }

                default: {
                    if(isdigit(c)) {
                        return get_digit(c);
                    }
                    if(c == '"') {
                        return get_string();
                    }
                    if(isalpha(c) || c == '_') {
                        return get_identifier(c);
                    }
                    return trigger_error(c);
                }
            }
}

lang::scanner::scanner() = default;

std::vector<std::shared_ptr<token>> lang::scanner::scan_line(std::string *data) {
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
    std::vector<std::shared_ptr<token>> tokens = std::vector<std::shared_ptr<token>>();
    err = false;
    for (current = 0; current < end;) {
        if(err)
            break;
        auto t = parse_token();
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

auto lang::scanner::in_multi_line() const -> bool {
    return in_multi_line_expression;
}

auto lang::scanner::in_multi_string() const -> bool {
    return in_multi_str;
}

auto lang::scanner::in_multi_comment() const -> bool {
    return in_multi_com;
}
