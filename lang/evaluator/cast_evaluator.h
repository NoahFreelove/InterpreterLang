#ifndef CAST_EVALUATOR_H
#define CAST_EVALUATOR_H
#include "../lang.h"
class cast_evaluator{
public:
    static std::pair<std::any, bool> eval(const std::any & any, int current_type, int target_type) {
        bool success = false;
        std::any val;
        if(current_type == INT) {
            int curr_val = std::any_cast<int>(any);
            switch (target_type) {
                case LONG: {
                    val = (long)(curr_val);
                    success = true;
                    break;
                }
                case BOOL_KEYW: {
                    val = curr_val != 0;
                    success = true;
                    break;
                }
                case FLOAT: {
                    val = (float)curr_val;
                    success = true;
                    break;
                }
                case DOUBLE: {
                    val = (double)curr_val;
                    success = true;
                    break;
                }
                case STRING: {
                    val = std::to_string(curr_val);
                    success = true;
                    break;
                }
                case INT: {
                    val = curr_val;
                    success = true;
                    break;
                }
                default: {
                    success = false;
                    break;
                }
            }
        }
        else if (current_type == LONG) {
            long curr_val = std::any_cast<long>(any);
            switch (target_type) {
                case INT: {
                    val = (int)(curr_val);
                    success = true;
                    break;
                }
                case BOOL_KEYW: {
                    val = curr_val != 0;
                    success = true;
                    break;
                }
                case FLOAT: {
                    val = (float)curr_val;
                    success = true;
                    break;
                }
                case DOUBLE: {
                    val = (double)curr_val;
                    success = true;
                    break;
                }
                case STRING: {
                    val = std::to_string(curr_val);
                    success = true;
                    break;
                }
                case LONG: {
                    val = curr_val;
                    success = true;
                    break;
                }
                default: {
                    success = false;
                    break;
                }
            }
        }
        else if(current_type == FLOAT) {
            float curr_val = std::any_cast<float>(any);
            switch (target_type) {
                case INT: {
                    val = (int)(curr_val);
                    success = true;
                    break;
                }
                case BOOL_KEYW: {
                    val = curr_val != 0;
                    success = true;
                    break;
                }
                case LONG: {
                    val = (long)(curr_val);
                    success = true;
                    break;
                }
                case DOUBLE: {
                    val = (double)curr_val;
                    success = true;
                    break;
                }
                case STRING: {
                    val = std::to_string(curr_val);
                    success = true;
                    break;
                }
                case FLOAT: {
                    val = curr_val;
                    success = true;
                    break;
                }
                default: {
                    success = false;
                    break;
                }
            }
        }
        else if (current_type == DOUBLE) {
            double curr_val = std::any_cast<double>(any);
            switch (target_type) {
                case INT: {
                    val = (int)(curr_val);
                    success = true;
                    break;
                }
                case BOOL_KEYW: {
                    val = curr_val != 0;
                    success = true;
                    break;
                }
                case LONG: {
                    val = (long)(curr_val);
                    success = true;
                    break;
                }
                case FLOAT: {
                    val = (float)curr_val;
                    success = true;
                    break;
                }
                case STRING: {
                    val = std::to_string(curr_val);
                    success = true;
                    break;
                }
                case DOUBLE: {
                    val = curr_val;
                    success = true;
                    break;
                }
                default: {
                    success = false;
                    break;
                }
            }
        }
        else if (current_type == STRING) {
            std::string curr_val = std::any_cast<std::string>(any);
            switch (target_type) {
                case INT: {
                    try {
                        val = std::stoi(curr_val);
                        success = true;
                    }
                    catch (std::invalid_argument & e) {
                        success = false;
                    }
                    break;
                }
                case BOOL_KEYW: {
                    val = curr_val == "true";
                    success = true;
                    break;
                }
                case LONG: {
                    try {
                        val = std::stol(curr_val);
                        success = true;
                    }
                    catch (std::invalid_argument & e) {
                        success = false;
                    }
                    break;
                }
                case FLOAT: {
                    try {
                        val = std::stof(curr_val);
                        success = true;
                    }
                    catch (std::invalid_argument & e) {
                        success = false;
                    }
                    break;
                }
                case DOUBLE: {
                    try {
                        val = std::stod(curr_val);
                        success = true;
                    }
                    catch (std::invalid_argument & e) {
                        success = false;
                    }
                    break;
                }
                case STRING: {
                    val = curr_val;
                    success = true;
                    break;
                }
                default: {
                    success = false;
                    break;
                }
            }
        }
        else if (current_type == BOOL_KEYW) {
            bool curr_val = std::any_cast<bool>(any);
            switch (target_type) {
                case INT: {
                    val = curr_val ? 1 : 0;
                    success = true;
                    break;
                }
                case BOOL_KEYW: {
                    val = curr_val;
                    success = true;
                    break;
                }
                case LONG: {
                    val = curr_val ? 1 : 0;
                    success = true;
                    break;
                }
                case FLOAT: {
                    val = curr_val ? 1.0f : 0.0f;
                    success = true;
                    break;
                }
                case DOUBLE: {
                    val = curr_val ? 1.0 : 0.0;
                    success = true;
                    break;
                }
                case STRING: {
                    val = curr_val ? "true" : "false";
                    success = true;
                    break;
                }
                default: {
                    success = false;
                    break;
                }
            }
        }
        else {
            success = false;
        }
        return std::make_pair(val, success);
    }
};
#endif //CAST_EVALUATOR_H
