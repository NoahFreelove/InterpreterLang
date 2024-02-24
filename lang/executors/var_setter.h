#ifndef VAR_SETTER_H
#define VAR_SETTER_H
#include <vector>
#include <memory>
#include "../tokenizer/token.h"
#include "../tokenizer/token_group.h"
#include "../tokenizer/token_grouper.h"
#include "../evaluator/group_evaluator.h"
inline std::vector<int> get_flags(const std::vector<std::shared_ptr<token>> &tokens, int offset = 2) {
    std::vector<int> flags;
    // Tokens 0 and 1 are type and identifier
    for (int i = offset; i < tokens.size(); ++i) {
        if(tokens[i]->get_name() == EQUAL) {
            return flags;
        }
        if(tokens[i]->get_name() == PERSISTENT || tokens[i]->get_name() == FINAL) {
            flags.push_back(tokens[i]->get_name());
        }
        else {
            lang::interpreter::error("illegal flag: " + id_to_name( tokens[i]->get_name()));
            return flags;
        }
    }
    return flags;
}

inline void implicit_upcast(const std::string& target_type, std::shared_ptr<token_group>& group) {
    if(target_type == "float" && (group->type == INT || group->type == LONG)) {
        if(group->type == INT) {
            group->value = (float)std::any_cast<int>(group->value);
        }
        else if (group->type == LONG) {
            group->value = (float)std::any_cast<long>(group->value);
        }
        group->type = FLOAT;
        return;
    }
    else if(target_type == "double" && (group->type == INT || group->type == LONG || group->type == FLOAT)) {
        if(group->type == INT) {
            group->value = (double)std::any_cast<int>(group->value);
        }
        else if (group->type == LONG) {
            group->value = (double)std::any_cast<long>(group->value);
        }
        else if (group->type == FLOAT) {
            group->value = (double)std::any_cast<float>(group->value);
        }
        group->type = DOUBLE;
        return;
    }
    else if(target_type == "long" && group->type == INT) {
        group->value = (long)std::any_cast<int>(group->value);
        group->type = LONG;
        return;
    }
    if(target_type == "float" && group->type == DOUBLE && lang::interpreter::is_defined("IMPLICIT_DOUBLE_TO_FLOAT")) {
        group->value = (float)std::any_cast<double>(group->value);
        group->type = FLOAT;
        return;
    }
}

inline std::shared_ptr<token> token_representation_from_data(data* dat) {
    switch (dat->get_type_int()) {
        case INT: {
            return std::make_shared<token>(INT, "int", 0, dat->get_int());
        }
        case FLOAT: {
            return std::make_shared<token>(FLOAT, "float", 0, dat->get_float());
        }
        case DOUBLE: {
            return std::make_shared<token>(DOUBLE, "double", 0, dat->get_double());
        }
        case LONG: {
            return std::make_shared<token>(LONG, "long", 0, dat->get_long());
        }
        case STRING: {
            return std::make_shared<token>(STRING, "string", 0, dat->get_string());
        }
        case CHAR_KEYW: {
            return std::make_shared<token>(CHAR_KEYW, "char", 0, dat->get_char());
        }
        case TRUE: {
            return std::make_shared<token>(TRUE, "true", 0, dat->get_bool());
        }
        case FALSE: {
            return std::make_shared<token>(FALSE, "false", 0, dat->get_bool());
        }

        case BOOL_KEYW: {
            return std::make_shared<token>(BOOL_KEYW, "bool", 0, dat->get_bool());
        }
        case ULONG64: {
            return std::make_shared<token>(ULONG64, "ulong64", 0, dat->get_ulonglong());
        }
        default: {
            return std::make_shared<token>(NOTHING, "nothing", 0, nullptr);
        }
    }
}

inline bool set_literal(const std::vector<std::shared_ptr<token>> &tokens, data *d, int offset = 2) {
    if(d) {
        auto group = lang::interpreter::evaluate_tokens(tokens, offset);
        if(group->type == ERROR) {
            lang::interpreter::error("error evaluating group");
            return false;
        }

        if(d->get_type() != token::type_tostr(group->type)) {
            // The default value for literals with decimals is a double which can be inconvinent if you have a float
            // as in most cases the float value can use the double value.
            if(d->get_type() == "float" && group->type == DOUBLE && lang::interpreter::is_defined("IMPLICIT_DOUBLE_TO_FLOAT")) {
                group->value = (float)std::any_cast<double>(group->value);
                group->type = FLOAT;
            }
            else if (lang::interpreter::is_defined("IMPLICIT_UPCAST")) {
                implicit_upcast(d->get_type(), group);
            }
            else {
                lang::interpreter::error("incompatible types, cannot set");
                return false;
            }
        }

        // token value is std::any, so we need to cast it to the correct type
        if(d->get_type() == "int" && group->type == INT) {
            //std::cout << "cast" << std::endl;
            d->set_value_int(std::any_cast<int>(group->value));
        }
        else if (d->get_type() == "float" && group->type == FLOAT) {
            d->set_value_float(std::any_cast<float>(group->value));
        }
        else if (d->get_type() == "double" && group->type == DOUBLE) {
            d->set_value_double(std::any_cast<double>(group->value));
        }
        else if (d->get_type() == "long"&& group->type == LONG) {
            d->set_value_long(std::any_cast<long>(group->value));
        }
        else if (d->get_type() == "string"&& group->type == STRING) {
            // copy str
            //char* str = (char*)malloc(sizeof(char)*strlen(const_char_convert(std::any_cast<const char*>(tokens[2]->get_lexeme())))+1);
            d->set_value_string(std::any_cast<std::string>(group->value));
        }
        else if (d->get_type() == "char") {
            // value is going to be a string so we take the first character
            auto val = std::any_cast<std::string>(tokens[2]->get_lexeme());
            if(!val.empty()) {
                d->set_value_char(val[0]);
            }
            else {
                lang::interpreter::error("Invalid char value");
                return true;
            }
        }
        else if (d->get_type() == "bool"&& (group->type == TRUE || group->type == FALSE)) {
            if(group->type == TRUE) {
                d->set_value_bool(true);
            }
            else if(group->type == FALSE) {
                d->set_value_bool(false);
            }
            else {
                lang::interpreter::error("invalid bool type");
            }
        }
        else if (d->get_type() == "unsigned long long") {
            d->set_value_ulonglong(std::any_cast<unsigned long long>(tokens[2]->get_value()));
        }
        else {
            std::string s = "Invalid type: ";
            s += d->get_type();
            s += " for group type: ";
            s += id_to_name(group->type);
            lang::interpreter::error(s);
            return true;
        }
    }
    return false;
}

inline void set_var(data* d, std::vector<std::shared_ptr<token>> tokens, const char* name) {
    if(!d) {
        char c[strlen("Undefined variable with name: ")+ strlen(tokens[0]->get_lexeme())+1];
        strcpy(c, "Undefined variable with name: ");
        strcat(c,tokens[0]->get_lexeme());
        lang::interpreter::error(c);
        return;
    }
    if(tokens[2]->is_literal() || tokens[2]->get_name() == CAST
        || tokens[2]->get_name() == ID || tokens[2]->get_name() == MINUS
        || tokens[2]->get_name() == LEFT_PAREN || tokens[2]->get_name() == RIGHT_PAREN ||
        tokens[2]->get_name() == BANG) {
        if (set_literal(tokens, d)) return;
        }
    else if(tokens[2]->get_name() == BYVAL && tokens.size() == 4) { // Doing byval has no effect but its technically valid
        if (set_literal(tokens, d)) return;
    }
    else if(tokens[2]->get_name() == RAWINPUT) {
        auto str = new std::string();
        std::cout << "> ";
        std::getline(std::cin, *str);
        auto t = lang::interpreter::scan->scan_line(str);

        if (set_literal(t, d,0)) {
            delete str;
            return;
        }

        delete str;
    }
    else if(tokens[2]->get_name() == INPUT) {
        auto str = std::string();
        std::cout << "> ";
        std::getline(std::cin, str);
        auto t = std::vector<std::shared_ptr<token>>();
        // set_literal expects first two tokens to be identifier =, so we prepend those
        t.insert(t.begin(), tokens[0]);
        t.insert(t.begin() + 1, tokens[1]);
        t.insert(t.begin() + 2, std::make_shared<token>(STRING, str.c_str(), tokens[0]->get_line(), str));
        set_literal(t, d);

    }
    else if (tokens[2]->get_name() == ID_GRAB && tokens[3]->get_name() == IDENTIFIER) {
        assign_variable(name, tokens[3]->get_lexeme());
    }
}

inline void process_variable_update(const std::vector<std::shared_ptr<token>> &tokens) {
    if(tokens.size() < 3) {
        lang::interpreter::error("Not enough tokens for variable update");
        return;
    }
    if(tokens[0]->get_name() != IDENTIFIER || tokens[1]->get_name() != EQUAL) {
        lang::interpreter::error("Invalid variable update");
        return;
    }
    const char* name = tokens[0]->get_lexeme();
    data* d = resolve_variable(name);
    set_var(d, tokens, name);
}

data * create_array_of_dim(const std::vector<std::vector<std::shared_ptr<token>>> & dimension_data, int type);

int keyw_to_name(int keyw);

inline void process_variable_declaration(const std::vector<std::shared_ptr<token>> &tokens) {
    std::shared_ptr<token> type;
    const char* name = nullptr;

    std::vector<int> flags;
    bool persistent = false;
    bool is_final = false;
    int set_index = lang::interpreter::get_equal_index(tokens);
    if(!tokens[0]->is_typeword()) {
        lang::interpreter::error("Cannot declare variable without typeword.");
        return;
    }
    type = tokens[0];
    bool is_array = false;

    // TODO: add dimensional array declarations
    // (should already be supported just need to figure this part out)
    std::vector<std::vector<std::shared_ptr<token>>> dimensions;

    if (tokens.size() >= 2) {
        if(tokens[1]->get_name() == LEFT_BRACKET) {
            std::vector<std::shared_ptr<token>> grp;
            bool in_dim = false;
            int i = 1;
            for (; i < tokens.size(); ++i) {
                if(tokens[i]->get_name() == LEFT_BRACKET) {
                    if(in_dim) {
                        lang::interpreter::error("Invalid array dimension - already in dimension");
                        return;
                    }
                    in_dim = true;
                    grp.clear();
                }
                else if(tokens[i]->get_name() == RIGHT_BRACKET) {
                    if(!in_dim) {
                        lang::interpreter::error("Invalid array dimension - not in dimension");
                        return;
                    }
                    if(grp.empty()) {
                        lang::interpreter::error("Invalid array dimension - empty dimension size");
                        return;
                    }
                    dimensions.emplace_back(grp);
                    in_dim = false;
                    if(i+1 < tokens.size()) {
                        if(tokens[i+1]->get_name() == LEFT_BRACKET)
                            continue;
                        else if(tokens[i+1]->get_name() == IDENTIFIER) {
                            break;
                        }
                        else {
                            lang::interpreter::error("Invalid array dimension - expected another dimension or identifier for array name");
                            return;
                        }
                    }
                }
                else {
                    grp.push_back(tokens[i]);
                }
            }
            if(in_dim) {
                lang::interpreter::error("Invalid array dimension - expected ]");
                return;
            }
            std::shared_ptr<token> id;
            if(i+1 >= tokens.size()) {
                lang::interpreter::error("Invalid array declaration - expected identifier for array name, found end of tokens");
                return;
            }
            else if(tokens[i+1]->get_name() != IDENTIFIER) {
                lang::interpreter::error("Invalid array declaration - expected identifier for array name, found " + id_to_name(tokens[i+1]->get_name()));
                return;
            }
            id = tokens[i+1];

            auto frame = lang::interpreter::stack->back();
            data* dat = create_array_of_dim(dimensions, keyw_to_name(type->get_name()));
            if(dat)
                frame->set(std::string(id->get_lexeme()), dat);
            else {
                lang::interpreter::error("Error in array dimension parsing");
            }
            return;
        }
        else if(tokens[1]->get_name()== IDENTIFIER) {
            name = tokens[1]->get_lexeme();
            flags = get_flags(tokens, 2);
        }

        else if(tokens[1]->get_name() != IDENTIFIER || !tokens[0]->is_typeword()) {
            lang::interpreter::error("Invalid variable declaration");
            return;
        }
        else {
            name = tokens[1]->get_lexeme();
            flags = get_flags(tokens, 2);
        }
    }
    else {
        lang::interpreter::error("Not enough tokens for variable declaration");
        return;
    }

    if(name == nullptr) {
        lang::interpreter::error("Internal error, variable name null");
        return;
    }

    // if the name ends with _old, it is invalid
    if(strstr(name, "_old")) {
        lang::interpreter::error("Invalid variable name, _old is reserved");
        return;
    }

    for (auto flag: flags) {
        if(flag == PERSISTENT)
            persistent = true;
        else if(flag == FINAL)
            is_final = true;
    }

    stack_frame* frame;
    if(persistent) {
        frame = lang::interpreter::global_frame;
    }
    else {
        frame = lang::interpreter::stack->back();
    }

    data* d = data::create_default_from_type(type->type_tostr(type->get_name()));
    if(!d) {
        lang::interpreter::error("Invalid type word");
        return;
    }
    frame->set(name, d);

    if(set_index > 1) {
        std::vector concat_vec = {tokens[1]};
        // add everything after type index
        for(int i = set_index; i < tokens.size(); i++) {
            concat_vec.push_back(tokens[i]);
        }
        process_variable_update(concat_vec);
    }
    if(d && is_final) {
        d->set_final();
    }
        //std::cout << frame->get_data(name)->get() << std::endl;
}
#endif //VAR_SETTER_H
