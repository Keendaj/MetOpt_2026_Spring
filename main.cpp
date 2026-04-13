#include <iostream>
#include <sstream>
#include <string>
#include <functional>
#include <cmath>
#include <iomanip>
#include <vector>
#include <map>
#include <fstream>
#include "Minimize.h" 
#include "MathParser.h" 

using std::cout;
using std::cin;
using std::string;
using std::istringstream;
using std::function;
using std::map;
using std::vector;

#ifdef _WIN32
#include <windows.h>
#endif

void exportFibonacciPlotSVG(const function<double(double)>& func, double a, double b) {
    std::vector<double> eps_vals = {0.1, 0.01, 0.001};
    std::vector<string> eps_labels = {"10^-1 (0.1)", "10^-2 (0.01)", "10^-3 (0.001)"};
    
    std::vector<int> calls_n5;
    std::vector<int> calls_n25;
    
    int max_calls = 0;

    for (double eps : eps_vals) {
        SearchStats st;
        fibonacciSearch(func, a, b, eps, 5, true, &st);
        calls_n5.push_back(st.calls);
        if (st.calls > max_calls) max_calls = st.calls;
        
        fibonacciSearch(func, a, b, eps, 25, true, &st);
        calls_n25.push_back(st.calls);
        if (st.calls > max_calls) max_calls = st.calls;
    }

    max_calls = (max_calls / 10 + 1) * 10; 

    std::ofstream out("fibonacci_plot.svg");
    if (!out.is_open()) {
        cout << "[!] Ошибка создания файла.\n";
        return;
    }

    int width = 700, height = 500;
    int padX = 80, padY = 60;
    
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << width << "\" height=\"" << height << "\">\n";
    out << "  <rect width=\"100%\" height=\"100%\" fill=\"#fdfdfd\"/>\n"; // Белый фон
    
    out << "  <line x1=\"" << padX << "\" y1=\"" << padY << "\" x2=\"" << padX << "\" y2=\"" << height - padY << "\" stroke=\"black\" stroke-width=\"2\"/>\n";
    out << "  <line x1=\"" << padX << "\" y1=\"" << height - padY << "\" x2=\"" << width - padX + 20 << "\" y2=\"" << height - padY << "\" stroke=\"black\" stroke-width=\"2\"/>\n";
    
    for (int i = 0; i <= max_calls; i += 10) {
        double y = height - padY - (double)i / max_calls * (height - 2 * padY);
        out << "  <line x1=\"" << padX - 5 << "\" y1=\"" << y << "\" x2=\"" << width - padX << "\" y2=\"" << y << "\" stroke=\"#e0e0e0\" stroke-width=\"1\"/>\n";
        out << "  <text x=\"" << padX - 15 << "\" y=\"" << y + 5 << "\" font-family=\"Arial\" font-size=\"14\" text-anchor=\"end\">" << i << "</text>\n";
    }
    
    for (size_t i = 0; i < eps_vals.size(); ++i) {
        double x = padX + (double)i / (eps_vals.size() - 1) * (width - 2 * padX);
        out << "  <line x1=\"" << x << "\" y1=\"" << height - padY << "\" x2=\"" << x << "\" y2=\"" << height - padY + 5 << "\" stroke=\"black\" stroke-width=\"2\"/>\n";
        out << "  <text x=\"" << x << "\" y=\"" << height - padY + 25 << "\" font-family=\"Arial\" font-size=\"14\" text-anchor=\"middle\">" << eps_labels[i] << "</text>\n";
    }

    out << "  <polyline fill=\"none\" stroke=\"#e74c3c\" stroke-width=\"3\" points=\"";
    for (size_t i = 0; i < eps_vals.size(); ++i) {
        double x = padX + (double)i / (eps_vals.size() - 1) * (width - 2 * padX);
        double y = height - padY - (double)calls_n5[i] / max_calls * (height - 2 * padY);
        out << x << "," << y << " ";
    }
    out << "\"/>\n";

    out << "  <polyline fill=\"none\" stroke=\"#3498db\" stroke-width=\"3\" points=\"";
    for (size_t i = 0; i < eps_vals.size(); ++i) {
        double x = padX + (double)i / (eps_vals.size() - 1) * (width - 2 * padX);
        double y = height - padY - (double)calls_n25[i] / max_calls * (height - 2 * padY);
        out << x << "," << y << " ";
    }
    out << "\"/>\n";

    for (size_t i = 0; i < eps_vals.size(); ++i) {
        double x = padX + (double)i / (eps_vals.size() - 1) * (width - 2 * padX);
        
        double y5 = height - padY - (double)calls_n5[i] / max_calls * (height - 2 * padY);
        out << "  <circle cx=\"" << x << "\" cy=\"" << y5 << "\" r=\"5\" fill=\"#e74c3c\"/>\n";
        out << "  <text x=\"" << x << "\" y=\"" << y5 - 12 << "\" font-family=\"Arial\" font-weight=\"bold\" font-size=\"14\" fill=\"#c0392b\" text-anchor=\"middle\">" << calls_n5[i] << "</text>\n";
        
        double y25 = height - padY - (double)calls_n25[i] / max_calls * (height - 2 * padY);
        out << "  <circle cx=\"" << x << "\" cy=\"" << y25 << "\" r=\"5\" fill=\"#3498db\"/>\n";
        out << "  <text x=\"" << x << "\" y=\"" << y25 + 22 << "\" font-family=\"Arial\" font-weight=\"bold\" font-size=\"14\" fill=\"#2980b9\" text-anchor=\"middle\">" << calls_n25[i] << "</text>\n";
    }
    out << "  <text x=\"" << width/2 << "\" y=\"" << 30 << "\" font-family=\"Arial\" font-size=\"18\" font-weight=\"bold\" text-anchor=\"middle\">Зависимость числа вычислений от требуемой точности</text>\n";
    out << "  <text x=\"" << width/2 << "\" y=\"" << height - 10 << "\" font-family=\"Arial\" font-size=\"14\" text-anchor=\"middle\">Точность (eps)</text>\n";
    out << "  <text x=\"" << 25 << "\" y=\"" << height/2 << "\" font-family=\"Arial\" font-size=\"14\" text-anchor=\"middle\" transform=\"rotate(-90 25," << height/2 << ")\">Вызовы функции f(x)</text>\n";

    out << "  <rect x=\"" << width - 150 << "\" y=\"" << 50 << "\" width=\"120\" height=\"60\" fill=\"white\" stroke=\"black\"/>\n";
    out << "  <line x1=\"" << width - 140 << "\" y1=\"" << 65 << "\" x2=\"" << width - 110 << "\" y2=\"" << 65 << "\" stroke=\"#e74c3c\" stroke-width=\"3\"/>\n";
    out << "  <text x=\"" << width - 100 << "\" y=\"" << 70 << "\" font-family=\"Arial\" font-size=\"14\">N = 5</text>\n";
    out << "  <line x1=\"" << width - 140 << "\" y1=\"" << 90 << "\" x2=\"" << width - 110 << "\" y2=\"" << 90 << "\" stroke=\"#3498db\" stroke-width=\"3\"/>\n";
    out << "  <text x=\"" << width - 100 << "\" y=\"" << 95 << "\" font-family=\"Arial\" font-size=\"14\">N = 25</text>\n";

    out << "</svg>\n";
    out.close();
}

void exportToSVG(const std::function<double(double)>& f, double a, double b, const string& filename) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        cout << "[!] Не удалось создать файл " << filename << "\n";
        return;
    }

    int width = 800, height = 500;
    int padX = 80, padY = 60;
    int points = 500; 

    double min_y = f(a), max_y = f(a);
    for(int i = 0; i <= points; ++i) {
        double x = a + i * (b - a) / points;
        double y = f(x);
        if(y < min_y) min_y = y;
        if(y > max_y) max_y = y;
    }

    double y_range = max_y - min_y;
    if (y_range == 0) y_range = 1.0;
    min_y -= y_range * 0.1;
    max_y += y_range * 0.1;

    double x_range = b - a;
    double min_x = a - x_range * 0.1;
    double max_x = b + x_range * 0.1;

    auto getX = [&](double x) { return padX + (x - min_x) / (max_x - min_x) * (width - 2 * padX); };
    auto getY = [&](double y) { return height - padY - (y - min_y) / (max_y - min_y) * (height - 2 * padY); };

    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << width << "\" height=\"" << height << "\">\n";
    out << "  <rect width=\"100%\" height=\"100%\" fill=\"#fdfdfd\"/>\n";

    int x_ticks = 10;
    for (int i = 0; i <= x_ticks; ++i) {
        double val = min_x + i * (max_x - min_x) / x_ticks;
        double px = getX(val);
        out << "  <line x1=\"" << px << "\" y1=\"" << padY << "\" x2=\"" << px << "\" y2=\"" << height - padY << "\" stroke=\"#e0e0e0\" stroke-width=\"1\"/>\n";
        
        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", val);
        out << "  <text x=\"" << px << "\" y=\"" << height - padY + 20 << "\" font-family=\"Arial\" font-size=\"12\" text-anchor=\"middle\">" << buf << "</text>\n";
    }
    
    int y_ticks = 10;
    for (int i = 0; i <= y_ticks; ++i) {
        double val = min_y + i * (max_y - min_y) / y_ticks;
        double py = getY(val);
        out << "  <line x1=\"" << padX << "\" y1=\"" << py << "\" x2=\"" << width - padX << "\" y2=\"" << py << "\" stroke=\"#e0e0e0\" stroke-width=\"1\"/>\n";
        
        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", val);
        out << "  <text x=\"" << padX - 10 << "\" y=\"" << py + 4 << "\" font-family=\"Arial\" font-size=\"12\" text-anchor=\"end\">" << buf << "</text>\n";
    }
    double zero_x = getX(0.0);
    double zero_y = getY(0.0);
    
    if (0.0 >= min_y && 0.0 <= max_y)
        out << "  <line x1=\"" << padX << "\" y1=\"" << zero_y << "\" x2=\"" << width - padX << "\" y2=\"" << zero_y << "\" stroke=\"black\" stroke-width=\"2\"/>\n";
    if (0.0 >= min_x && 0.0 <= max_x)
        out << "  <line x1=\"" << zero_x << "\" y1=\"" << padY << "\" x2=\"" << zero_x << "\" y2=\"" << height - padY << "\" stroke=\"black\" stroke-width=\"2\"/>\n";

    out << "  <line x1=\"" << getX(a) << "\" y1=\"" << padY << "\" x2=\"" << getX(a) << "\" y2=\"" << height - padY << "\" stroke=\"#e74c3c\" stroke-width=\"2\" stroke-dasharray=\"5,5\"/>\n";
    out << "  <line x1=\"" << getX(b) << "\" y1=\"" << padY << "\" x2=\"" << getX(b) << "\" y2=\"" << height - padY << "\" stroke=\"#e74c3c\" stroke-width=\"2\" stroke-dasharray=\"5,5\"/>\n";
    out << "  <text x=\"" << getX(a) << "\" y=\"" << padY - 10 << "\" font-family=\"Arial\" font-weight=\"bold\" font-size=\"14\" fill=\"#e74c3c\" text-anchor=\"middle\">Гран. A</text>\n";
    out << "  <text x=\"" << getX(b) << "\" y=\"" << padY - 10 << "\" font-family=\"Arial\" font-weight=\"bold\" font-size=\"14\" fill=\"#e74c3c\" text-anchor=\"middle\">Гран. B</text>\n";

    out << "  <polyline fill=\"none\" stroke=\"#2ecc71\" stroke-width=\"3\" points=\"";
    for(int i = 0; i <= points; ++i) {
        double x = min_x + i * (max_x - min_x) / points;
        double y = f(x);
        out << getX(x) << "," << getY(y) << " ";
    }
    out << "\"/>\n";

    out << "  <text x=\"" << width/2 << "\" y=\"" << 30 << "\" font-family=\"Arial\" font-size=\"18\" font-weight=\"bold\" text-anchor=\"middle\">График целевой функции f(x)</text>\n";
    out << "  <text x=\"" << width/2 << "\" y=\"" << height - 15 << "\" font-family=\"Arial\" font-size=\"14\" text-anchor=\"middle\">Значения X</text>\n";
    out << "  <text x=\"" << 25 << "\" y=\"" << height/2 << "\" font-family=\"Arial\" font-size=\"14\" text-anchor=\"middle\" transform=\"rotate(-90 25," << height/2 << ")\">Значения f(x)</text>\n";

    out << "</svg>\n";
    out.close();
}

void printHelp() {
    cout << "\n--- Доступные команды ---\n"
         << "help                     - Показать это меню\n"
         << "help_expr                - Показать как вводить функции\n"
         << "status                   - Показать текущие параметры\n"
         << "load_variant             - Загрузить исходные данные варианта ([-2, -1], m=100, eps=0.1)\n"
         << "set_bounds <a> <b>       - Задать границы интервала поиска (например: set_bounds 0 10)\n"
         << "set_eps <eps>            - Задать точность (например: set_eps 0.0001)\n"
            << "set_m <m>             - Задать количество разбиений сетки (для равномерного, m >= 2)\n"
         << "set_n <N>                - Задать N чисел Фибоначчи (для метода Фибоначчи, N >= 4)\n"
         << "set_expr <формула>       - Задать функцию строкой (например: set_expr x^2 - 3*sin(x))\n"
         << "solve_uni [--silent]     - Решить методом РАВНОМЕРНОГО поиска\n"
         << "solve_fib [--silent]     - Решить методом ФИБОНАЧЧИ с возвратом\n"
         << "table                    - Печать исследовательской таблицы\n"
         << "plot              - Отрисовка графика функции\n"
         << "plot_fib                 - Создать график зависимости вычислений от eps\n"
         << "\n--- История функций ---\n"
         << "save <name>              - Сохранить текущую функцию в историю\n"
         << "load <name>              - Загрузить функцию из истории\n"
         << "history                  - Показать список сохраненных функций\n"
         << "exit                     - Выход\n"
         << "-------------------------\n";
}

void printExprHelp() {
    std::cout << "\n--- Справка по вводу формул (команда set_expr) ---\n"
              << "Формулы вводятся в виде обычной строки. Пробелы игнорируются.\n\n"
              << "[ Переменная ]\n"
              << "  В формуле должна использоваться только английская буква 'x' или 'X'.\n\n"
              << "[ Базовые операции ]\n"
              << "  +   Сложение         (x + 5)\n"
              << "  -   Вычитание        (x - 5)\n"
              << "  * Умножение        (3 * x)\n"
              << "  /   Деление          (x / 2)\n"
              << "  ^   Возведение в ст. (x ^ 2)\n\n"
              << "[ Поддерживаемые функции ]\n"
              << "  sin(x)  - Синус\n"
              << "  cos(x)  - Косинус\n"
              << "  tan(x)  - Тангенс\n"
              << "  exp(x)  - Экспонента (e^x)\n"
              << "  log(x)  - Натуральный логарифм (ln x)\n"
              << "  sqrt(x) - Квадратный корень\n\n"
              << "[ Приоритет и скобки ]\n"
              << "  Для управления порядком действий используйте круглые скобки ().\n"
              << "  Пример: 5 * (x - 3)^2 + sin(x/2)\n\n"
              << "[ Примеры команд ]\n"
              << "  set_expr x^2 - 4*x + 4\n"
              << "  set_expr -sin(x) + cos(2*x)\n"
              << "  set_expr exp(-x) * sqrt(x)\n"
              << "--------------------------------------------------\n";
}

int main() {
    #ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    #endif

    double a = 0.0;
    double b = 10.0;
    double eps = 1e-4;
    int m = 10;
    int N = 10;
    
    MathParser parser; 
    string current_expr_str = "(x - 3.14)^2 + 2.5"; 
    
    map<string, string> func_history;

    function<double(double)> current_func = [&parser, &current_expr_str](double x) {
        return parser.evaluate(current_expr_str, x);
    };

    cout << "Программа одномерной оптимизации (метод равномерного поиска) запущена.\n";
    cout << "Введите 'help' для списка команд.\n";

    while (true) {
        cout << "\n[Optimize] > ";
        
        string line;
        if (!getline(cin, line)) break; 

        istringstream iss(line);
        string command;
        
        if (!(iss >> command)) continue; 

        if (command == "exit") {
            break;
        }
        else if (command == "help") {
            printHelp();
        }
        else if (command == "help_expr") {
            printExprHelp();
        }
        else if (command == "status") {
            cout << "--- Текущие настройки ---\n";
            cout << "Интервал поиска: [" << a << ", " << b << "]\n";
            cout << "Точность (eps):  " << eps << "\n";
            cout << "Разбиений (m):   " << m << " (Для равномерного)\n";
            cout << "Длина последовательности Фибоначчи (N):  " << N << " (Для Фибоначчи)\n";
            cout << "Функция f(x)  =  " << current_expr_str << "\n";
            cout << "-------------------------\n";
        }
        else if (command == "load_variant") {
            a = -2.0;
            b = -1.0;
            m = 10;
            eps = 0.1;
            N = 5;
            current_expr_str = "exp(x) - x^3 / 3 + 2*x";
            
            try {
                parser.evaluate(current_expr_str, 0.0);
                cout << "[+] Данные варианта успешно загружены!\n";
                cout << "    Интервал: [" << a << ", " << b << "]\n";
                cout << "    Точность: " << eps << "\n";
                cout << "    m:        " << m << "\n";
                cout << "    N:        " << N << "\n";
                cout << "    Функция:  f(x) = " << current_expr_str << "\n";
            } catch (const std::exception& e) {
                cout << "[!] Ошибка синтаксиса при загрузке варианта: " << e.what() << "\n";
            }
        }
        else if (command == "set_bounds") {
            double new_a, new_b;
            if (iss >> new_a >> new_b) {
                if (new_a < new_b) {
                    a = new_a;
                    b = new_b;
                    cout << "[+] Границы успешно обновлены: [" << a << ", " << b << "]\n";
                } else {
                    cout << "[!] Ошибка: левая граница 'a' должна быть меньше правой 'b'.\n";
                }
            } else {
                cout << "[!] Ошибка: Формат команды 'set_bounds <a> <b>'.\n";
            }
        }
        else if (command == "set_n") {
            int new_n;
            if (iss >> new_n) {
                if (new_n >= 4) {
                    N = new_n;
                    cout << "[+] Число N обновлено: N = " << N << "\n";
                } else {
                    cout << "[!] Ошибка: N должно быть не меньше 4.\n";
                }
            } else {
                cout << "[!] Ошибка: Формат команды 'set_n <N>'.\n";
            }
        }
        else if (command == "set_eps") {
            double new_eps;
            if (iss >> new_eps) {
                if (new_eps > 0) {
                    eps = new_eps;
                    cout << "[+] Точность обновлена: eps = " << eps << "\n";
                } else {
                    cout << "[!] Ошибка: Точность должна быть строго больше 0.\n";
                }
            } else {
                cout << "[!] Ошибка: Формат команды 'set_eps <eps>'.\n";
            }
        }
        else if (command == "set_m") {
            int new_m;
            if (iss >> new_m) {
                if (new_m >= 2) {
                    m = new_m;
                    cout << "[+] Число разбиений обновлено: m = " << m << "\n";
                } else {
                    cout << "[!] Ошибка: m должно быть не меньше 2.\n";
                }
            } else {
                cout << "[!] Ошибка: Формат команды 'set_m <m>'.\n";
            }
        }
        else if (command == "set_expr") {
            string expr;
            if (getline(iss, expr)) {
                expr.erase(0, expr.find_first_not_of(" \t")); 
                if (!expr.empty()) {
                    try {
                        parser.evaluate(expr, 0.0); 
                        current_expr_str = expr;
                        std::cout << "[+] Успешно! Новая функция: f(x) = " << current_expr_str << "\n";
                    } catch (const std::exception& e) {
                        std::cout << "[!] Ошибка синтаксиса: " << e.what() << "\n";
                        std::cout << "[i] Введите 'help_expr', чтобы посмотреть правила ввода формул.\n"; 
                    }
                } else {
                    std::cout << "[!] Ошибка: Формула не может быть пустой.\n";
                }
            } else {
                std::cout << "[!] Ошибка: Формат команды 'set_expr <формула>'.\n";
            }
        }
        else if (command == "save") {
            string name;
            if (iss >> name) {
                func_history[name] = current_expr_str;
                cout << "[+] Текущая функция сохранена под именем '" << name << "'.\n";
            } else {
                cout << "[!] Ошибка: Укажите имя для сохранения. (Например: save my_func)\n";
            }
        }
        else if (command == "load") {
            string name;
            if (iss >> name) {
                if (func_history.count(name)) {
                    current_expr_str = func_history[name];
                    cout << "[+] Функция '" << name << "' успешно загружена: f(x) = " << current_expr_str << "\n";
                } else {
                    cout << "[!] Ошибка: Функция с именем '" << name << "' не найдена в истории.\n";
                }
            } else {
                cout << "[!] Ошибка: Укажите имя для загрузки. (Например: load my_func)\n";
            }
        }
        else if (command == "history") {
            cout << "--- Сохраненные функции ---\n";
            if (func_history.empty()) {
                cout << "  (пусто)\n";
            } else {
                for (const auto& pair : func_history) {
                    cout << "  - " << pair.first << ": f(x) = " << pair.second << "\n";
                }
            }
            cout << "---------------------------\n";
        }
        else if (command == "solve_uni") {
            cout << "[*] Запуск метода равномерного поиска...\n\n";
            bool silent_mode = false;
            string arg;
            if (iss >> arg && arg == "--silent") {
                silent_mode = true;
            }
            try {
                double min_x = uniformSearch(current_func, a, b, eps, m, silent_mode);
                
                cout << "\n[+] Решение найдено!\n";
                cout << std::fixed << std::setprecision(5);
                cout << "Приближенная точка минимума: x* = " << min_x << "\n";
                cout << "Значение функции в точке: f(x*) = " << current_func(min_x) << "\n";
            } catch (const std::exception& e) {
                cout << "[!] Ошибка во время вычисления функции: " << e.what() << "\n";
            }
        }
        else if (command == "solve_fib") {
            bool silent_mode = false;
            string arg;
            if (iss >> arg && arg == "--silent") {
                silent_mode = true;
            }

            if (!silent_mode) cout << "[*] Запуск метода Фибоначчи с возвратом...\n\n";
            else cout << "[*] Вычисление в тихом режиме (Фибоначчи)...\n";
            
            try {
                double min_x = fibonacciSearch(current_func, a, b, eps, N, silent_mode);
                
                if (!silent_mode) cout << "\n";
                cout << "[+] Решение найдено!\n";
                cout << std::fixed << std::setprecision(5);
                cout << "Приближенная точка минимума: x* = " << min_x << "\n";
                cout << "Значение функции в точке: f(x*) = " << current_func(min_x) << "\n";
            } catch (const std::exception& e) {
                cout << "[!] Ошибка во время вычисления функции: " << e.what() << "\n";
            }
        }
        else if (command == "table") {
            cout << "\n=========================================================================\n";
            cout << " Метод              | Параметр | eps    | K_теор | K_факт | Итер/циклы \n";
            cout << "--------------------+----------+--------+--------+--------+----------------\n";
            
            vector<double> eps_vals = {1e-1, 1e-2, 1e-3};
            
            for (size_t i = 0; i < eps_vals.size(); ++i) {
                SearchStats st;
                uniformSearch(current_func, a, b, eps_vals[i], 10, true, &st);

                if (i == 0) cout << " Равномерный поиск  | m = 10   | ";
                else        cout << "                    |          | ";

                cout << "10^-" << (i+1) << " | " 
                     << std::left << std::setw(6) << st.k_teor << " | " 
                     << std::setw(6) << st.calls << " | " 
                     << st.iters << "\n";
            }
            cout << "--------------------+----------+--------+--------+--------+----------------\n";

            for (size_t i = 0; i < eps_vals.size(); ++i) {
                SearchStats st;
                fibonacciSearch(current_func, a, b, eps_vals[i], 5, true, &st);
                
                if (i == 0) cout << " Фибоначчи          | N = 5    | ";
                else        cout << "                    |          | ";
                
                cout << "10^-" << (i+1) << " | " 
                     << std::left << std::setw(6) << st.k_teor << " | " 
                     << std::setw(6) << st.calls << " | " 
                     << st.iters << "\n";
            }
            cout << "--------------------+----------+--------+--------+--------+----------------\n";

            for (size_t i = 0; i < eps_vals.size(); ++i) {
                SearchStats st;
                fibonacciSearch(current_func, a, b, eps_vals[i], 25, true, &st);
                
                if (i == 0) cout << " Фибоначчи          | N = 25   | ";
                else        cout << "                    |          | ";
                
                cout << "10^-" << (i+1) << " | " 
                     << std::left << std::setw(6) << st.k_teor << " | " 
                     << std::setw(6) << st.calls << " | " 
                     << st.iters << "\n";
            }
            cout << "=========================================================================\n";
        }
        else if (command == "plot") {
            string filename = "graph.svg";
            exportToSVG(current_func, a, b, filename);
            cout << "[+] График успешно сгенерирован и сохранен в файл: " << filename << "\n";
            cout << "[i] Вы можете открыть " << filename << " в любом браузере (Chrome, Edge, Яндекс) или перетащить в Word.\n";
        }
        else if (command == "plot_fib") {
            cout << "[*] Сбор данных и генерация графика...\n";
            exportFibonacciPlotSVG(current_func, a, b);
            cout << "[+] График успешно сгенерирован!\n";
            cout << "[i] Файл 'fibonacci_plot.svg' сохранен в папке с программой.\n";
            cout << "    Откройте его в браузере (Chrome, Edge) или перетащите прямо в Word-отчет.\n";
        }
        else {
            cout << "[?] Неизвестная команда '" << command << "'. Введите 'help' для вызова справки.\n";
        }
    }

    return 0;
}