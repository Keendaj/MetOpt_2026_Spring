#include "LinearProblemClass.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
    #ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    #endif

    LPProblem Problem;
    Problem.input();

    Problem.printForms();
    return 0;
}