#ifndef TYPE_ARITHMETIC_H
#define TYPE_ARITHMETIC_H
class type_arithmetic {
public:
    static int result(int a, int b, int op) {
        if(op == EXPONENT) {
            return std::pow<int>(a, b);
        }
        else if(op == STAR) {
            return a * b;
        }
        else if(op == SLASH) {
            return a / b;
        }
        else if(op == PLUS) {
            //std::cout << "adding: " << ant_val << " " << cons_val << std::endl;
            return a + b;
        }
        else if(op == MINUS) {
            return a-b;
        }
        else {
            return 0;
        }
    }
    static double result(double a, double b, int op) {
        if(op == EXPONENT) {
            return std::pow<double>(a, b);
        }
        else if(op == STAR) {
            return a * b;
        }
        else if(op == SLASH) {
            return a / b;
        }
        else if(op == PLUS) {
            return a + b;
        }
        else if(op == MINUS) {
            return a-b;
        }
        else {
            return 0;
        }
    }
    static float result(float a, float b, int op) {
        if(op == EXPONENT) {
            return std::pow<float>(a, b);
        }
        else if(op == STAR) {
            return a * b;
        }
        else if(op == SLASH) {
            return a / b;
        }
        else if(op == PLUS) {
            return a + b;
        }
        else if(op == MINUS) {
            return a-b;
        }
        else {
            return 0;
        }
    }
    static long result(long a, long b, int op) {
        if(op == EXPONENT) {
            return std::pow<long>(a, b);
        }
        else if(op == STAR) {
            return a * b;
        }
        else if(op == SLASH) {
            return a / b;
        }
        else if(op == PLUS) {
            return a + b;
        }
        else if(op == MINUS) {
            return a-b;
        }
        else {
            return 0;
        }
    }
};
#endif //TYPE_ARITHMETIC_H
