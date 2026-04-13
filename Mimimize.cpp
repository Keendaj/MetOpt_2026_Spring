#include "Minimize.h"
#include <iostream>
#include <cmath>
#include <iomanip>
#include <string>

double uniformSearch(const std::function<double(double)>& f, double a, double b, double eps = std::pow(10,-1), int m = 10) {
    if (m < 2) {
        std::cerr << "Ошибка: m должно быть >= 2. Установлено m=2.\n";
        m = 2;
    }

    double L0 = b - a; 
    int iter = 0;      
    int calls = 0;     

    std::cout << std::string(80, '-') << "\n";
    std::cout << std::left 
              << std::setw(5)  << "k" 
              << std::setw(12) << "a" 
              << std::setw(12) << "b" 
              << std::setw(12) << "L_k" 
              << std::setw(18) << "Текущий x*" 
              << std::setw(18) << "f(x*)" << "\n";
    std::cout << std::string(80, '-') << "\n";

    std::cout << std::fixed << std::setprecision(5);

    while ((b - a) > eps) {
        double L_k = b - a;
        double h = L_k / m; 
        
        double min_f = f(a);
        int min_idx = 0;
        double min_x = a;
        calls++;

        for (int i = 1; i <= m; ++i) {
            double x_i = a + i * h;
            double f_i = f(x_i);
            calls++;
            
            if (f_i < min_f) {
                min_f = f_i;
                min_idx = i;
                min_x = x_i;
            }
        }

        std::cout << std::left 
                  << std::setw(5)  << iter 
                  << std::setw(12) << a 
                  << std::setw(12) << b 
                  << std::setw(12) << L_k 
                  << std::setw(18) << min_x 
                  << std::setw(18) << min_f << "\n";

        if (min_idx == 0) {
            b = a + 2 * h; 
        } else if (min_idx == m) {
            a = b - 2 * h; 
        } else {
            a = a + (min_idx - 1) * h;
            b = a + 2 * h;
        }
        
        iter++;
    }

    std::cout << std::string(80, '-') << "\n";

    double r_teor_raw = std::log(L0 / eps) / std::log(m / 2.0);
    int r_teor = static_cast<int>(std::ceil(r_teor_raw));
    int k_teor = r_teor * (m + 1);

    std::cout << "--- Статистика поиска ---\n";
    std::cout << "Практическое число итераций: " << iter << "\n";
    std::cout << "Теоретическое число итераций (r): " << r_teor << "\n";
    std::cout << "Практическое число обращений: " << calls << "\n";
    std::cout << "Теоретическое число обращений (K_теор): " << k_teor << "\n";
    std::cout << "-------------------------\n";

    return (a + b) / 2.0; 
}