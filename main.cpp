#include <iostream>
#include <sstream>
#include <limits>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include "TransportProblem.h"

using std::cout;
using std::cin;
using std::string;
using std::vector;
using std::map;
using std::endl;
using std::setw;
using std::istringstream;

#ifdef _WIN32
#include <windows.h>
#endif

void printHelp() {
    cout << "\n--- Доступные команды ---\n"
         << "help                        - Показать это меню\n"
         << "input                       - Ввести матрицу тарифов и векторы A, B вручную\n"
         << "load_variant33              - Загрузить данные нужного варианта\n"
         << "print_task                  - Вывести текущие векторы A, B и матрицу C\n"
         << "solve_potential [--print k] - Решить методом потенциалов (k - вывод каждой k-й итерации)\n"
         << "solve_simplex [--print k]   - Решить двухфазным симплекс-методом (k - вывод каждой k-й итерации)\n"
         << "print_potential             - Вывести результат метода потенциалов\n"
         << "print_simplex               - Вывести результат симплекс-метода\n"
         << "make_open <var> <p1..pm>    - Преобразовать в открытую задачу (var - номер варианта, далее вектор штрафов)\n"
         << "\n--- История и редактирование ---\n"
         << "save <name>                 - Сохранить текущую задачу в историю\n"
         << "load <name>                 - Загрузить задачу из истории\n"
         << "history                     - Показать список сохраненных задач\n"
         << "edit_a <a1> <a2> ...        - Изменить вектор запасов A\n"
         << "edit_b <b1> <b2> ...        - Изменить вектор потребностей B\n"
         << "edit_c <row> <col> <val>    - Изменить элемент матрицы тарифов C (индексы от 1)\n"
         << "exit                        - Выход\n"
         << "-------------------------\n";
}

int main() {
    #ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    #endif

    TransportProblem tp;
    map<string, TransportProblem> history;
    bool dataLoaded = false;

    cout << "Программа для решения транспортной задачи запущена. Введите 'help' для списка команд.\n";

    while (true) {
        cout << "\n[Transport] > ";
        
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
        else if (command == "input") {
            tp.input();
            dataLoaded = true;
            cout << "[+] Данные успешно загружены.\n";
        } 
        else if (command == "load_variant33") {
            vector<double> A = {21, 19, 23, 13};
            vector<double> B = {5, 13, 4, 22, 32};
            vector<vector<double>> C = {
                {9, 2, 21, 10, 6},
                {8, 3, 19, 9, 12},
                {11, 8, 11, 4, 2},
                {2, 8, 7, 6, 1}
            };
            tp.setData(A, B, C);
            dataLoaded = true;
            cout << "[+] Загружен вариант 33 (сбалансированная задача 4x5).\n";
        }
        else if (command == "print_task") {
            if (!dataLoaded) {
                cout << "[!] Ошибка: Нет активных данных.\n";
                continue;
            }
            cout << "Вектор запасов A (m=" << tp.m << "): ";
            for (double a : tp.A) cout << a << " ";
            cout << "\nВектор потребностей B (n=" << tp.n << "): ";
            for (double b : tp.B) cout << b << " ";
            cout << "\nМатрица тарифов C:\n";
            for (int i = 0; i < tp.m; ++i) {
                for (int j = 0; j < tp.n; ++j) {
                    cout << setw(6) << tp.C[i][j];
                }
                cout << "\n";
            }
        }
        else if (command == "save") {
            string name;
            if (iss >> name) {
                if (!dataLoaded) {
                    cout << "[!] Ошибка: Нет данных для сохранения.\n";
                } else {
                    history[name] = tp;
                    cout << "[+] Текущая задача сохранена под именем '" << name << "'.\n";
                }
            } else {
                cout << "[!] Ошибка: Укажите имя для сохранения.\n";
            }
        }
        else if (command == "load") {
            string name;
            if (iss >> name) {
                if (history.count(name)) {
                    tp = history[name];
                    dataLoaded = true;
                    cout << "[+] Задача '" << name << "' успешно загружена.\n";
                } else {
                    cout << "[!] Ошибка: Сохранение с именем '" << name << "' не найдено.\n";
                }
            } else {
                cout << "[!] Ошибка: Укажите имя для загрузки.\n";
            }
        }
        else if (command == "history") {
            cout << "Сохраненные задачи:\n";
            if (history.empty()) cout << "  (пусто)\n";
            for (const auto& pair : history) {
                cout << "  - " << pair.first << " (m=" << pair.second.m << ", n=" << pair.second.n << ")\n";
            }
        }
        else if (command == "edit_a") {
            vector<double> newA;
            double val;
            while (iss >> val) newA.push_back(val);
            
            if (newA.empty()) {
                cout << "[!] Ошибка: Введите новые значения вектора A через пробел.\n";
            } else {
                tp.A = newA;
                tp.m = newA.size();
                tp.C.resize(tp.m, vector<double>(tp.n, 0.0));
                dataLoaded = true;
                cout << "[+] Вектор A обновлен. Текущий размер m = " << tp.m << ".\n";
            }
        }
        else if (command == "edit_b") {
            vector<double> newB;
            double val;
            while (iss >> val) newB.push_back(val);
            
            if (newB.empty()) {
                cout << "[!] Ошибка: Введите новые значения вектора B через пробел.\n";
            } else {
                tp.B = newB;
                tp.n = newB.size();
                for (int i = 0; i < tp.m; ++i) {
                    tp.C[i].resize(tp.n, 0.0);
                }
                dataLoaded = true;
                cout << "[+] Вектор B обновлен. Текущий размер n = " << tp.n << ".\n";
            }
        }
        else if (command == "edit_c") {
            int r, c;
            double val;
            if (iss >> r >> c >> val) {
                if (r >= 1 && r <= tp.m && c >= 1 && c <= tp.n) {
                    tp.C[r-1][c-1] = val;
                    cout << "[+] Элемент C[" << r << "][" << c << "] успешно изменен на " << val << ".\n";
                } else {
                    cout << "[!] Ошибка: Индексы выходят за границы. Ожидается строка (1.." << tp.m << ") и столбец (1.." << tp.n << ").\n";
                }
            } else {
                cout << "[!] Ошибка: Формат команды 'edit_c <строка> <столбец> <значение>'.\n";
            }
        }
        else if (command == "print_potential") {
            if (!dataLoaded) cout << "[!] Ошибка: Нет активных данных.\n";
            else tp.printPotentialPlan();
        }
        else if (command == "print_simplex") {
            if (!dataLoaded) cout << "[!] Ошибка: Нет активных данных.\n";
            else tp.printSimplexPlan();
        }
        else if (command == "solve_potential") {
            if (!dataLoaded) {
                cout << "[!] Ошибка: Сначала загрузите данные.\n";
                continue;
            }

            string arg1;
            int step = 0;
            if (iss >> arg1 && arg1 == "--print") {
                string stepStr;
                if (iss >> stepStr) {
                    try { step = std::stoi(stepStr); } 
                    catch (...) { cout << "[!] Ошибка: Неверный формат шага.\n"; continue; }
                }
            }

            cout << "[*] Запуск метода потенциалов...\n";
            if (tp.solvePotential(step)) {
                cout << "\n[+] Решение найдено успешно!\n";
                tp.printPotentialPlan(); 
            } else {
                cout << "[!] Метод не смог найти оптимальное решение.\n";
            }
        } 
        else if (command == "solve_simplex") {
            if (!dataLoaded) {
                cout << "[!] Ошибка: Сначала загрузите данные.\n";
                continue;
            }

            string arg1;
            int step = 0;
            if (iss >> arg1 && arg1 == "--print") {
                string stepStr;
                if (iss >> stepStr) {
                    try { step = std::stoi(stepStr); } 
                    catch (...) { cout << "[!] Ошибка: Неверный формат шага.\n"; continue; }
                }
            }

            cout << "[*] Запуск двухфазного симплекс-метода...\n";
            if (tp.solveSimplex(1e-7, step)) {
                cout << "\n[+] Решение найдено успешно!\n";
                tp.printSimplexPlan();
            } else {
                cout << "[!] Симплекс-метод не смог найти решение.\n";
            }
        }
        else if (command == "make_open") {
            if (!dataLoaded) {
                cout << "[!] Ошибка: Сначала загрузите исходные данные закрытой задачи.\n";
                continue;
            }

            string varStr;
            if (iss >> varStr) {
                int variant_num;
                try { variant_num = std::stoi(varStr); } 
                catch (...) { cout << "[!] Ошибка: Неверно указан номер варианта.\n"; continue; }

                vector<double> penalties;
                double p;
                while (iss >> p) penalties.push_back(p);

                if ((int)penalties.size() != tp.m) {
                    cout << "[!] Ошибка: Количество штрафов (" << penalties.size() << ") должно совпадать с m=" << tp.m << ".\n";
                    continue;
                }

                tp.convertToOpenWithPenalties(variant_num, penalties);
                cout << "[+] Задача преобразована в открытую (добавлен фиктивный потребитель и штрафы).\n";
            } else {
                cout << "[!] Ошибка: Укажите номер варианта и вектор штрафов.\n";
            }
        }
        else {
            cout << "[?] Неизвестная команда. Введите 'help'.\n";
        }
    }
    return 0;
}