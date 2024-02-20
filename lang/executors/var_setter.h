#ifndef VAR_SETTER_H
#define VAR_SETTER_H

inline void implicit_upcast(const std::string& type, std::shared_ptr<token_group>& group) {
    if(type == "float" && (group->type == INT || group->type == LONG)) {
        if(group->type == INT) {
            group->value = (float)std::any_cast<int>(group->value);
        }
        else if (group->type == LONG) {
            group->value = (float)std::any_cast<long>(group->value);
        }
        group->type = FLOAT;
    }
    else if(type == "double" && (group->type == INT || group->type == LONG || group->type == FLOAT)) {
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
    }
    else if(type == "long" && group->type == INT) {
        group->value = (long)std::any_cast<int>(group->value);
        group->type = LONG;
    }
}
inline bool set_literal(const std::vector<std::shared_ptr<token>> &tokens, data *d) {
    if(d) {
        auto group = lang::interpreter::evaluate_tokens(tokens, 2);
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

    if(!d) {
        char c[strlen("Undefined variable with name: ")+ strlen(tokens[0]->get_lexeme())+1];
        strcpy(c, "Undefined variable with name: ");
        strcat(c,tokens[0]->get_lexeme());
        lang::interpreter::error(c);
        return;
    }
    if(tokens[2]->is_literal() || tokens[2]->get_name() == CAST || tokens[2]->get_name() == ID || tokens[2]->get_name() == MINUS || tokens[2]->get_name() == LEFT_PAREN || tokens[2]->get_name() == RIGHT_PAREN) {
        if (set_literal(tokens, d)) return;
    }
    else if(tokens[2]->get_name() == BYVAL && tokens.size() == 4) { // Doing byval has no effect but its technically valid
        if (set_literal(tokens, d)) return;
    }
    else if(tokens[2]->get_name() == INPUT) {
        auto str = new std::string();
        std::cout << "> ";
        std::getline(std::cin, *str);
        auto t = lang::interpreter::scan->scan_line(str);
        // set_literal expects first two tokens to be identifier =, so we prepend those
        t.insert(t.begin(), tokens[0]);
        t.insert(t.begin() + 1, tokens[1]);
        if (set_literal(t, d)) {
            delete str;
            return;
        }

        delete str;
    }
    else if (tokens[2]->get_name() == ID_GRAB && tokens[3]->get_name() == IDENTIFIER) {
        assign_variable(name, tokens[3]->get_lexeme());
    }
}

inline void process_variable_declaration(const std::vector<std::shared_ptr<token>> &tokens) {
    std::shared_ptr<token> type;
    const char* name;

    std::vector<int> flags = lang::interpreter::get_flags(tokens);
    bool persistent = false;
    bool is_final = false;
    int set_index = lang::interpreter::get_equal_index(tokens);

    if (tokens.size() >= 2) {
        if(tokens[1]->get_name() != IDENTIFIER || !tokens[0]->is_typeword()) {
            lang::interpreter::error("Invalid variable declaration");
            return;
        }
        name = tokens[1]->get_lexeme();
        type = tokens[0];
    }
    else {
        lang::interpreter::error("Not enough tokens for variable declaration");
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
    data* d = nullptr;
    switch (type->get_name()) {
        case INT_KEYW: {
            int* val = new int;
            *val = 0;
            d = new data(val, "int");
             frame->set(name, d);
             break;
            }
        case FLOAT_KEYW: {
            float *val = new float;
            *val = 0.0f;
            d = new data(val, "float");
            frame->set(name, d);
            break;
        }
        case DOUBLE_KEYW: {
            double *val = new double;
            *val = 0.0;
            d = new data(val, "double");
            frame->set(name, d);
            break;
        }
        case LONG_KEYW: {
            long *val = new long;
            *val = 0L;
            d = new data(val, "long");
            frame->set(name, d);
            break;
        }
        case STRING_KEYW: {
            d = new data(new std::string(), "string");
            frame->set(name, d);
            break;
        }
        case CHAR_KEYW: {
            char*val = new char;
            *val = '\0';
            d = new data(val, "char");
            frame->set(name, d);
            break;
        }
        case BOOL_KEYW: {
            bool* val = new bool;
            *val = false;
            d = new data(val, "bool");
            frame->set(name, d);
            break;
        }
        case ULONG64_KEYW: {
            auto* val = new unsigned long long;
            *val = 0;
            d = new data(val, "unsigned long long");
            frame->set(name, d);
            break;
        }
        default: {
            lang::interpreter::error("Invalid type word");
            return;
        }
    }
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
