#include <iostream>
#include <sstream>
#include <limits>
#include <climits>
#include <string>
#include <map>
#include <vector>
#include "LinearProblemClass.h"

using std::cout;
using std::cin;
using std::string;
using std::vector;
using std::map;
using std::endl;
using std::istringstream;
using std::numeric_limits;
using std::streamsize;

#ifdef _WIN32
#include <windows.h>
#endif

void printHelp() {
    cout << "\n--- Доступные команды ---\n"
         << "help               - Показать это меню\n"
         << "input              - Ввести новую задачу (создает все формы)\n"
         << "pick [type]        - Выбрать активную задачу (original, symmetrical, canonical)\n"
         << "print [type]       - Вывести матрицу (original, dual)\n"
         << "solve_simplex --printSteps [num] [type]  - Решить задачу симплекс-методом (original, dual)\n"
         << "solve_vertices --printSteps [num] [type] - Решить задачу методом перебора крайних точек(original, dual)\n"
         << "print_result [type]- Показать ответ (original, dual)\n"
         << "exit               - Выход\n"
         << "-------------------------\n";
}

int main() {
    #ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    #endif


    map<string, LPProblem> problems;
    map<string, LPProblem> dualProblems;
    string activeType = "original";
    string command;

    cout << "Программа для решения задач ЛП запущена. Введите 'input' для начала.\n";
    printHelp();

   while (true) {
        cout << "\n[" << activeType << "] > ";
        
        string line;
        if (!getline(cin, line)) break; 

        istringstream iss(line);
        string command;
        
        if (!(iss >> command)) continue; 

        if (command == "exit") break;

        if (command == "help") {
            printHelp();
        } 
        else if (command == "input") {
            LPProblem orig;
            orig.input();

            problems["original"] = orig;
            dualProblems["original"] = orig.toDual();

            problems["symmetrical"] = orig.toSymmetric();
            dualProblems["symmetrical"] = problems["symmetrical"].toDual();
            problems["canonical"] = orig.toCanonical();
            dualProblems["canonical"] = problems["canonical"].toDual();
            
            cout << "[+] Задача сохранена во всех формах.\n";
        } 
        else if (command == "pick") {
            string type;
            if (iss >> type) {
                if (problems.count(type)) {
                    activeType = type;
                    cout << "[*] Активная задача: " << type << endl;
                } else {
                    cout << "[!] Ошибка: Тип '" << type << "' не найден. Сначала введите задачу.\n";
                }
            } else {
                cout << "[!] Ошибка: Укажите тип задачи (например, pick original).\n";
            }
        } 
        else if (command == "print") {
            string sub;
            if (iss >> sub) {
                if (sub == "original") {
                    problems[activeType].printOriginal();
                } else if (sub == "dual") {
                    dualProblems[activeType].printOriginal();
                } else {
                    cout << "[!] Ошибка: Укажите 'original' или 'dual'.\n";
                }
            } else {
                cout << "[!] Ошибка: Недостаточно аргументов.\n";
            }
        } 
        else if (command == "solve_simplex" || command == "solve_vertices") {
            string arg1;
            int step = 0;
            string sub = "";

            if (iss >> arg1) {
                if (arg1 == "--printSteps") {
                    string stepStr;
                    if (iss >> stepStr) {
                        try {
                            step = std::stoi(stepStr);
                        } catch (...) {
                            cout << "[!] Ошибка: Неверный формат шага.\n";
                            continue;
                        }

                        if (!(iss >> sub)) {
                            cout << "[!] Ошибка: После шага укажите 'original' или 'dual'.\n";
                            continue;
                        }
                    } else {
                        cout << "[!] Ошибка: Укажите число шагов после --printSteps.\n";
                        continue;
                    }
                } else {
                    sub = arg1;
                }

                bool success = false;
                if (sub == "original") {
                    success = (command == "solve_simplex") 
                              ? problems[activeType].solveSimplex(1e-6, step) 
                              : problems[activeType].solveByVertices(step);
                } else if (sub == "dual") {
                    success = (command == "solve_simplex") 
                              ? dualProblems[activeType].solveSimplex(1e-6, step) 
                              : dualProblems[activeType].solveByVertices(step);
                } else {
                    cout << "[!] Ошибка: Неизвестный параметр '" << sub << "'. Ожидается 'original' или 'dual'.\n";
                    continue;
                }

                if (success) cout << "[+] Решение завершено успешно.\n";
                else cout << "[!] Метод не смог найти решение.\n";
            } else {
                cout << "[!] Ошибка: Недостаточно аргументов.\n";
            }
        } 
        else if (command == "print_result") {
            string sub;
            if (iss >> sub) {
                if (sub == "dual" && dualProblems.count(activeType) && dualProblems[activeType].solved) {
                    dualProblems[activeType].printResult();
                } else if (sub == "original" && problems.count(activeType) && problems[activeType].solved) {
                    problems[activeType].printResult();
                } else {
                    cout << "[!] Такой проблемы нет, либо она еще не была решена.\n";
                }
            } else {
                cout << "[!] Ошибка: Укажите 'original' или 'dual'.\n";
            }
        } 
        else {
            cout << "[?] Неизвестная команда. Введите 'help'.\n";
        }
    }
    return 0;
}