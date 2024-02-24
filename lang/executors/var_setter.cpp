#include "var_setter.h"

data * create_array_of_dim(const std::vector<std::vector<std::shared_ptr<token>>> &dimension_data, int type)  {
    std::queue<int> sizes;
    for (auto& vec : dimension_data) {
        auto group = token_grouper::gen_group(vec);
        int length = -1;
        group_evaluator::eval_group(group);
        int result_type = group->type;
        if(result_type == INT || result_type == FLOAT || result_type == LONG || result_type == DOUBLE) {
            if(group->type == INT) {
                length = std::any_cast<int>(group->value);
            }
            else if(group->type == LONG) {
                length = std::any_cast<long>(group->value);
            }
            else if(group->type == FLOAT) {
                length = (int)std::any_cast<float>(group->value);
            }
            else if(group->type == DOUBLE) {
                length = (int)std::any_cast<double>(group->value);
            }
            if(length > 0) {
                sizes.push(length);
            }
            else if(length == 0) {
                lang::interpreter::error("Attempted to use a zero size for array dimension!");
                return nullptr;
            }
            else {
                lang::interpreter::error("Attempted to use a negative size for array dimension!");
                return nullptr;
            }
        }
        else {
            lang::interpreter::error("Attempted to use a non-numeric size for array dimension!");
            return nullptr;
        }
    }

    return data::create_recursive_dimensional_array(sizes, type);
}

int keyw_to_name(int keyw) {
    switch (keyw) {
        case INT_KEYW:
            return INT;
        case LONG_KEYW:
            return LONG;
        case FLOAT_KEYW:
            return FLOAT;
        case DOUBLE_KEYW:
            return DOUBLE;
        case STRING_KEYW:
            return STRING;
        case CHAR_KEYW:
            return CHAR_KEYW;
        case BOOL_KEYW:
            return BOOL_KEYW;
        case ULONG64_KEYW:
            return ULONG64;
        case TRUE:
            return TRUE;
        case FALSE:
            return FALSE;
        default:
            return -1;
    }
}