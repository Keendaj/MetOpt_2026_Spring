#include "Minimize.h"
#include <iostream>
#include <cmath>
#include <iomanip>
#include <string>
#include <vector>


double uniformSearch(const std::function<double(double)>& f, double a, double b, double eps, int m, bool silent, SearchStats* stats) {
    if (m < 2) {
        if (!silent) std::cerr << "Ошибка: m должно быть >= 2. Установлено m=2.\n";
        m = 2;
    }

    double L0 = b - a; 
    int iter = 0;      
    int calls = 0; 

    if (!silent) {
        std::cout << std::fixed << std::setprecision(5);
    }

    while ((b - a) > eps) {
        double L_k = b - a;
        double h = L_k / m; 
        
        if (!silent) {
            std::cout << "\n=================================================================\n";
            std::cout << " ИТЕРАЦИЯ " << iter << " | Интервал: [" << a << ", " << b << "] | Длина L_k = " << L_k << "\n";
            std::cout << "-----------------------------------------------------------------\n";
            std::cout << " Узел i | Координата x | Значение f(x) | Примечание\n";
            std::cout << "--------+--------------+---------------+-------------------------\n";
        }

        std::vector<double> x_vals(m + 1);
        std::vector<double> f_vals(m + 1);
        
        double min_f = std::numeric_limits<double>::max();
        int min_idx = 0;

        for (int i = 0; i <= m; ++i) {
            x_vals[i] = a + i * h;
            f_vals[i] = f(x_vals[i]); 
            calls++;                  

            if (f_vals[i] < min_f) {
                min_f = f_vals[i];
                min_idx = i;
            }
        }

        if (!silent) {
            for (int i = 0; i <= m; ++i) {
                std::cout << " " << std::left 
                          << std::setw(6)  << i << " | "
                          << std::setw(12) << x_vals[i] << " | "
                          << std::setw(13) << f_vals[i] << " | ";
                
                if (i == min_idx) {
                    std::cout << "<-- ЛОКАЛЬНЫЙ МИНИМУМ";
                }
                std::cout << "\n";
            }
            std::cout << "-----------------------------------------------------------------\n";
        }

        if (min_idx == 0) {
            b = a + 2 * h; 
        } else if (min_idx == m) {
            a = b - 2 * h; 
        } else {
            a = a + (min_idx - 1) * h;
            b = a + 2 * h;
        }
        
        if (!silent) {
            std::cout << "[*] Сужение: новый интервал [" << a << ", " << b << "]\n";
        }
        iter++;
    }

    double r_teor_raw = std::log(L0 / eps) / std::log(m / 2.0);
    int r_teor = static_cast<int>(std::ceil(r_teor_raw));
    int k_teor = r_teor * (m + 1);
    
    if (!silent) {
        std::cout << "\n=================================================================\n";
        std::cout << "--- ИТОГОВАЯ СТАТИСТИКА ПОИСКА ---\n";
        std::cout << "Практическое число итераций: " << iter << "\n";
        std::cout << "Теоретическое число итераций (r): " << r_teor << "\n";
        std::cout << "Практическое число вызовов функции: " << calls << "\n"; 
        std::cout << "Теоретическое число обращений (K_теор): " << k_teor << "\n";
        std::cout << "----------------------------------\n";
    }
    
    if (stats) {
        stats->iters = iter;
        stats->calls = calls;
        stats->k_teor = k_teor;
    }
    return (a + b) / 2.0; 
}

double fibonacciSearch(const std::function<double(double)>& f, double a, double b, double eps, int N, bool silent, SearchStats* stats) {
    if (N < 4) {
        if (!silent) std::cout << "[!] Внимание: Для метода с возвратом требуется N >= 4. Установлено N = 4.\n";
        N = 4;
    }

    std::vector<double> F(N + 1);
    F[1] = 1;
    F[2] = 2;
    for (int i = 3; i <= N; ++i) {
        F[i] = F[i - 1] + F[i - 2];
    }

    int cycle = 1;
    int calls = 0;

    if (!silent) {
        std::cout << std::fixed << std::setprecision(4); // Как в вашей таблице, 4 знака
        std::cout << "\n====================================================================================================\n";
        std::cout << " Цикл | Шаг |     a     |     b     |    x1     |    x2     |   f(x1)   |   f(x2)   | Действие\n";
        std::cout << "------+-----+-----------+-----------+-----------+-----------+-----------+-----------+---------------\n";
    }

    while ((b - a) > eps) {
        double L = b - a;
        
        double x1 = a + (F[N - 2] / F[N]) * L;
        double x2 = a + (F[N - 1] / F[N]) * L;
        
        double f1 = f(x1);
        double f2 = f(x2);
        calls += 2;

        double rollback_a = a;
        double rollback_b = b;

        for (int k = 1; k <= N - 2; ++k) {
            
            if (k == N - 2) {
                rollback_a = a;
                rollback_b = b;
            }

            std::string action = (f1 <= f2) ? "Сужение вправо" : "Сужение влево";

            if (!silent) {
                std::cout << " " << std::left 
                          << std::setw(4) << cycle << " | "
                          << std::setw(3) << k << " | "
                          << std::setw(9) << a << " | "
                          << std::setw(9) << b << " | "
                          << std::setw(9) << x1 << " | "
                          << std::setw(9) << x2 << " | "
                          << std::setw(9) << f1 << " | "
                          << std::setw(9) << f2 << " | "
                          << action << "\n";
            }

            if (f1 <= f2) {
                b = x2;
                x2 = x1;
                f2 = f1;
                x1 = a + b - x2; 
                f1 = f(x1);
                calls++;
            } else {
                a = x1;
                x1 = x2;
                f1 = f2;
                x2 = a + b - x1;
                f2 = f(x2);
                calls++;
            }
        }

        a = rollback_a;
        b = rollback_b;
        
        bool is_finish = ((b - a) <= eps);
        std::string rollback_action = is_finish ? "Финиш (Останов)" : "Возврат базиса";

        if (!silent) {
            std::cout << " " << std::left 
                      << std::setw(4) << cycle << " | "
                      << std::setw(3) << "Отк" << " | "
                      << std::setw(9) << a << " | "
                      << std::setw(9) << b << " | "
                      << std::setw(9) << "---" << " | "
                      << std::setw(9) << "---" << " | "
                      << std::setw(9) << "---" << " | "
                      << std::setw(9) << "---" << " | "
                      << rollback_action << "\n";
        }
        
        cycle++;
    }

    if (!silent) {
        std::cout << "====================================================================================================\n";
        std::cout << "--- ИТОГОВАЯ СТАТИСТИКА (МЕТОД ФИБОНАЧЧИ) ---\n";
        std::cout << "Заданное N (чисел Фибоначчи): " << N << "\n";
        std::cout << "Число выполненных циклов: " << (cycle - 1) << "\n";
        std::cout << "Число вызовов функции: " << calls << "\n"; 
        std::cout << "---------------------------------------------\n";
    }
    if (stats) {
        stats->iters = cycle - 1; 
        stats->calls = calls;
        stats->k_teor = calls;
    }
    return (a + b) / 2.0; 
}