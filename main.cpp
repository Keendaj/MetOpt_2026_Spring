#include <iostream>
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

#ifdef _WIN32
#include <windows.h>
#endif

void printHelp() {
    cout << "\n--- Доступные команды ---\n"
         << "help               - Показать это меню\n"
         << "input              - Ввести новую задачу (создает все формы)\n"
         << "pick [type]        - Выбрать активную задачу (original, symmetrical, canonical)\n"
         << "print [type]       - Вывести матрицу (original, dual)\n"
         << "solve --printSteps [num] [type]       - Решить задачу (original, dual)\n"
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
        cin >> command;

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
            cin >> type;
            if (problems.count(type)) {
                activeType = type;
                cout << "[*] Активная задача: " << type << endl;
            } else {
                cout << "[!] Ошибка: Тип '" << type << "' не найден. Сначала введите задачу.\n";
            }
        } 
        else if (command == "print") {
            string sub;
            cin >> sub;
            if (sub == "original") {
                problems[activeType].printOriginal();
            } 
            else if (sub == "dual") {
                dualProblems[activeType].printOriginal();
            }
        } 
        else if (command == "solve") {
            string sub;
            cin >> sub;
            if(sub == "--printSteps"){
                cin >> sub;
                if(int step = std::stoi(sub)){
                    cin >> sub;
                    if (sub == "original") {
                        if (problems[activeType].solveSimplex(1e-6, step)) {
                            cout << "[+] Решение завершено успешно.\n";
                        }
                    } 
                    else if (sub == "dual") {
                        if (dualProblems[activeType].solveSimplex(1e-6, step)) {
                            cout << "[+] Решение завершено успешно.\n";
                        }
                    }
                }
            }
            else if (sub == "original") {
                if (problems[activeType].solveSimplex(1e-6, 0)) {
                    cout << "[+] Решение завершено успешно.\n";
                }
            } 
            else if (sub == "dual") {
                if (dualProblems[activeType].solveSimplex(1e-6, 0)) {
                    cout << "[+] Решение завершено успешно.\n";
                }
            }
        } 
        else if (command == "print_result") {
            string sub;
            cin >> sub;
            if(sub == "dual" && dualProblems.count(activeType) && dualProblems[activeType].solved){
                dualProblems[activeType].printResult();
            }
            else if (sub == "original" && problems.count(activeType) && problems[activeType].solved) {
                problems[activeType].printResult();
            }
            else{
                cout << "[!] Такой проблемы нет, либо она ещё не была решена.\n";
            }
        }
        else {
            cout << "[?] Неизвестная команда. Введите 'help'.\n";
        }
    }

    return 0;
}