#ifndef MATH_PARSER_H
#define MATH_PARSER_H

#include <string>
#include <cmath>
#include <stdexcept>
#include <cctype>
#include <algorithm>

class MathParser {
private:
    const char* p;
    double x_val;

    double parseExpression() {
        double val = parseTerm();
        while (*p == '+' || *p == '-') {
            char op = *p++;
            double val2 = parseTerm();
            if (op == '+') val += val2;
            else val -= val2;
        }
        return val;
    }

    double parseTerm() {
        double val = parseFactor();
        while (*p == '*' || *p == '/') {
            char op = *p++;
            double val2 = parseFactor();
            if (op == '*') val *= val2;
            else val /= val2;
        }
        return val;
    }

    double parseFactor() {
        double val = parsePrimary();
        if (*p == '^') {
            p++;
            double val2 = parseFactor();
            val = std::pow(val, val2);
        }
        return val;
    }

    double parsePrimary() {
        while (std::isspace(*p)) p++;
        
        if (*p == '(') {
            p++;
            double val = parseExpression();
            if (*p == ')') p++;
            return val;
        }
        
        if (*p == '-') { p++; return -parsePrimary(); }
        if (*p == '+') { p++; return parsePrimary(); }
        
        if (*p == 'x' || *p == 'X') {
            p++;
            return x_val;
        }
        
        if (std::isalpha(*p)) {
            std::string funcName;
            while (std::isalpha(*p)) funcName += *p++;
            if (*p == '(') {
                p++;
                double arg = parseExpression();
                if (*p == ')') p++;
                
                if (funcName == "sin") return std::sin(arg);
                if (funcName == "cos") return std::cos(arg);
                if (funcName == "tan") return std::tan(arg);
                if (funcName == "exp") return std::exp(arg);
                if (funcName == "log") return std::log(arg);
                if (funcName == "sqrt") return std::sqrt(arg);
                throw std::runtime_error("Неизвестная функция: " + funcName);
            }
        }
        
        // Числа
        char* end;
        double val = std::strtod(p, &end);
        if (p == end) throw std::runtime_error(std::string("Неожиданный символ: ") + *p);
        p = end;
        return val;
    }

public:
    double evaluate(const std::string& expr, double x) {
        std::string clean_expr = expr;
        clean_expr.erase(std::remove_if(clean_expr.begin(), clean_expr.end(), ::isspace), clean_expr.end());
        
        p = clean_expr.c_str();
        x_val = x;
        double result = parseExpression();
        
        if (*p != '\0') throw std::runtime_error("Некорректное окончание выражения");
        return result;
    }
};

#endif