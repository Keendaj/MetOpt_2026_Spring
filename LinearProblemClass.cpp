#include "LinearProblemClass.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <limits>

using namespace std;

string LPProblem::getSignStr(ConstraintType type) {
    switch (type) {
        case ConstraintType::GEQ: return ">=";
        case ConstraintType::LEQ: return "<=";
        case ConstraintType::EQ:  return "=";
    }
    return "";
}

string LPProblem::normalizeExpression(const string& input) {
    string res;
    for (size_t i = 0; i < input.length(); ++i) {
        char ch = input[i];
        if (ch == 'X') {
            ch = 'x';
        }

        if (ch == '+' || ch == '-') {
            res += " "; 
            res += ch; 
            res += " ";
        } 
        else if (ch == '<' || ch == '>') {
            res += " "; 
            res += ch;
            if (i + 1 < input.length() && input[i + 1] == '=') {
                res += '=';
                i++;
            }
            res += " ";
        }
        else if (ch == '=') {
            res += " = ";
        }
        else if (isdigit(ch) || ch == 'x' || ch == '.') {
            res += ch;
        }
        else if (!isspace(ch)) {
            cout << "[!] Предупреждение: обнаружен и пропущен посторонний символ '" << ch << "'\n";
        }
    }
    return res;
}

void LPProblem::parseExpression(const string& expr, vector<double>& row) {
    string normalized = normalizeExpression(expr);
    stringstream ss(normalized);
    string token;
    double currentSign = 1.0;

    while (ss >> token) {
        if (token == "+") {
            currentSign = 1.0;
        } 
        else if (token == "-") {
            currentSign = -1.0;
        }
        else {
            size_t xPos = token.find('x');
            if (xPos != string::npos) {
                string coeffStr = token.substr(0, xPos);
                string idxStr = token.substr(xPos + 1);

                double coeff = 1.0;
                if (!coeffStr.empty()){
                    coeff = stod(coeffStr);
                }

                int varIdx = stoi(idxStr) - 1;

                if (varIdx >= row.size()) {
                    row.resize(varIdx + 1, 0.0);
                }
                
                row[varIdx] += currentSign * coeff;

                n = max(n, varIdx + 1);
            }
        }
    }
}

void LPProblem::parseObjectiveFunction() {
    cout << "1. Введите целевую функцию (например 10x1 + 10x2 + 10x3 + 10x4):\nF(x) = ";
    string funcStr;
    getline(cin, funcStr);
    
    parseExpression(funcStr, c);

    cout << "2. Выберите цель (введите min или max): ";
    string goal;
    cin >> goal;
    isMin = (goal == "min");

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void LPProblem::parseConstraints() {
    cout << "Введите количество ограничений (m): ";
    while (!(cin >> m) || m <= 0) {
        cout << "Ошибка! Введите положительное целое число: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    A.resize(m);
    b.resize(m);
    signs.resize(m);

    cout << "3. Введите " << m << " ограничений (например 10x1 + 2x2 <= 20, знаки: =, <=, >=):\n";
    for (int i = 0; i < m; ++i) {
        cout << i + 1 << ") ";
        string constrStr;
        getline(cin, constrStr);

        size_t signPos;
        if ((signPos = constrStr.find("<=")) != string::npos) {
            signs[i] = ConstraintType::LEQ;
            parseExpression(constrStr.substr(0, signPos), A[i]);
            b[i] = stod(constrStr.substr(signPos + 2));
        } 
        else if ((signPos = constrStr.find(">=")) != string::npos) {
            signs[i] = ConstraintType::GEQ;
            parseExpression(constrStr.substr(0, signPos), A[i]);
            b[i] = stod(constrStr.substr(signPos + 2));
        } 
        else if ((signPos = constrStr.find("=")) != string::npos) {
            signs[i] = ConstraintType::EQ;
            parseExpression(constrStr.substr(0, signPos), A[i]);
            b[i] = stod(constrStr.substr(signPos + 1));
        }
        else {
            cout << "Ошибка! Строка должна содержать знак '<=', '>=' или '='. Попробуйте еще раз.\n";
        }
    }
}

void LPProblem::normalizeVectorSizes() {
    c.resize(n, 0.0);
    for (int i = 0; i < m; ++i) {
        A[i].resize(n, 0.0);
    }
}

void LPProblem::parseVariableSigns() {
    var_signs.resize(n, VarSign::POSITIVE);
    
    cout << "\n4. Выберите ограничения для переменных (найдено переменных: " << n << ").\n";
    cout << "Введите 1 (>= 0), -1 (<= 0) или 0 (свободна от ограничений) для каждой:\n";
    
    for (int i = 0; i < n; ++i) {
        cout << "x" << i + 1 << ": ";
        int signInput;
        cin >> signInput;
        
        if (signInput == 1){ 
            var_signs[i] = VarSign::POSITIVE;
        }
        else if (signInput == -1) {
            var_signs[i] = VarSign::NEGATIVE;
        }
        else if (signInput == 0){
            var_signs[i] = VarSign::FREE;
        }
        else{
            cout << "Неправильный ввод, выбран по умолчанию 0" << std::endl;
            var_signs[i] = VarSign::FREE;
        }
    }
}

void LPProblem::input() {
    parseObjectiveFunction();
    parseConstraints();
    normalizeVectorSizes();
    
    parseVariableSigns();
    
    cout << "\nДанные успешно считаны!\n";
}

ProblemForm LPProblem::determineForm() {
    bool allVarsPositive = true;
    for (VarSign vs : var_signs) {
        if (vs != VarSign::POSITIVE) {
            allVarsPositive = false;
            break;
        }
    }

    bool allEq = true;
    bool allGeq = true;
    bool allLeq = true;

    for (ConstraintType ct : signs) {
        if (ct != ConstraintType::EQ) {
            allEq = false;
        }
        if (ct != ConstraintType::GEQ) {
            allGeq = false;
        }
        if (ct != ConstraintType::LEQ) {
            allLeq = false;
        }
    }

    if (allVarsPositive && allEq){ 
        return ProblemForm::CANONICAL;
    }

    if (allVarsPositive) {
        if (isMin && allGeq) {
            return ProblemForm::SYMMETRIC;
        }
        if (!isMin && allLeq) { 
            return ProblemForm::SYMMETRIC;
        }
    }

    return ProblemForm::GENERAL;
}

void LPProblem::printTerm(double coeff, const string& varStr, bool& isFirst) {
    if (coeff == 0) {
        return;
    }
    
    if (isFirst) {
        if (coeff < 0) {
            cout << "-";
        }
    } else {
        if (coeff > 0) {
            cout << " + ";
        } else {
            cout << " - ";
        }
    }
    
    double val = abs(coeff);
    if (val != 1.0) {
        cout << val;
    }
    
    cout << varStr;
    isFirst = false;
}

void LPProblem::printObjective() {
    cout << "F(x) = ";
    bool first = true;
    for (int j = 0; j < n; ++j) {
        string name = (j < var_names.size()) ? var_names[j] : "x" + to_string(j + 1);
        printTerm(c[j], name, first);
    }
    if (first) {
        cout << "0";
    }
    cout << " -> " << (isMin ? "min" : "max") << "\n";
}

void LPProblem::printConstraints() {
    for (int i = 0; i < m; ++i) {
        bool first = true;
        for (int j = 0; j < n; ++j) {
            string name = (j < var_names.size()) ? var_names[j] : "x" + to_string(j + 1);
            printTerm(A[i][j], name, first);
        }
        if (first) {
            cout << "0";
        }
        cout << " " << getSignStr(signs[i]) << " " << b[i] << "\n";
    }
}

void LPProblem::printVariableConstraints() {
    cout << "Ограничения на переменные:\n";
    for (int j = 0; j < n; ++j) {
        if (j < (int)var_names.size()) {
            cout << var_names[j];
        } else {
            cout << "x" << j + 1;
        }

        if (var_signs[j] == VarSign::POSITIVE) {
            cout << " >= 0";
        } else if (var_signs[j] == VarSign::NEGATIVE) {
            cout << " <= 0";
        } else {
            cout << " - свободная";
        }
        
        cout << "\n";
    }
}

void LPProblem::printOriginal() {
    printObjective();
    cout << "\n";
    printConstraints();
    cout << "\n";
    printVariableConstraints();
}

void LPProblem::printDual() {
    cout << "G(y) = ";
    bool first = true;
    for (int i = 0; i < m; ++i) {
        printTerm(b[i], "y" + to_string(i + 1), first);
    }
    if (first) {
        cout << "0";
    }
    cout << " -> " << (isMin ? "max" : "min") << "\n\n";

    cout << "Ограничения двойственной задачи:\n";
    for (int j = 0; j < n; ++j) {
        first = true;
        for (int i = 0; i < m; ++i) {
            printTerm(A[i][j], "y" + to_string(i + 1), first);
        }
        if (first) {
            cout << "0";
        }

        if (var_signs[j] == VarSign::POSITIVE) {
            cout << (isMin ? " <= " : " >= ") << c[j] << "\n";
        } 
        else if (var_signs[j] == VarSign::NEGATIVE) {
            cout << (isMin ? " >= " : " <= ") << c[j] << "\n";
        } 
        else {
            cout << " = " << c[j] << "\n";
        }
    }

    cout << "\nОграничения на переменные y_i:\n";
    for (int i = 0; i < m; ++i) {
        cout << "y" << i + 1;

        if (signs[i] == ConstraintType::GEQ) {
            cout << (isMin ? " >= 0" : " <= 0");
        } 
        else if (signs[i] == ConstraintType::LEQ) {
            cout << (isMin ? " <= 0" : " >= 0");
        } 
        else {
            cout << " - свободная";
        }
        
        cout << "\n";
    }
}

LPProblem LPProblem::toGeneral() {
    LPProblem res = *this;
    
    if (res.m > 0) {
        if (res.signs[0] == ConstraintType::EQ) {
            res.signs[0] = ConstraintType::LEQ;
        } else {
            res.signs[0] = ConstraintType::EQ;
        }
    }

    if (res.n > 0) {
        res.var_signs[0] = VarSign::FREE;
    }

    return res;
}

LPProblem LPProblem::toCanonical() {
    LPProblem res;
    res.isMin = this->isMin;
    res.m = this->m;
    
    for (int j = 0; j < n; ++j) {
        if (var_signs[j] == VarSign::FREE) {
            res.c.push_back(this->c[j]);
            res.var_names.push_back("x" + to_string(j + 1) + "'");
            res.var_signs.push_back(VarSign::POSITIVE);

            res.c.push_back(-this->c[j]);
            res.var_names.push_back("x" + to_string(j + 1) + "''");
            res.var_signs.push_back(VarSign::POSITIVE);
        } 
        else if (var_signs[j] == VarSign::NEGATIVE) {
            res.c.push_back(-this->c[j]);
            res.var_names.push_back("x" + to_string(j + 1) + "*");
            res.var_signs.push_back(VarSign::POSITIVE);
        } 
        else {
            res.c.push_back(this->c[j]);
            res.var_names.push_back("x" + to_string(j + 1));
            res.var_signs.push_back(VarSign::POSITIVE);
        }
    }

    res.A.resize(m);
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            if (var_signs[j] == VarSign::FREE) {
                res.A[i].push_back(this->A[i][j]);
                res.A[i].push_back(-this->A[i][j]);
            } 
            else if (var_signs[j] == VarSign::NEGATIVE) {
                res.A[i].push_back(-this->A[i][j]);
            }
            else {
                res.A[i].push_back(this->A[i][j]);
            }
        }
    }

    res.b = this->b;
    int slackCount = 1;
    for (int i = 0; i < m; ++i) {
        if (signs[i] == ConstraintType::LEQ) {
            for (int row = 0; row < m; ++row) {
                res.A[row].push_back( (row == i) ? 1.0 : 0.0 );
            }
            res.c.push_back(0.0);
            res.var_names.push_back("s" + to_string(slackCount++));
            res.var_signs.push_back(VarSign::POSITIVE);
        } 
        else if (signs[i] == ConstraintType::GEQ) {
            for (int row = 0; row < m; ++row) {
                res.A[row].push_back( (row == i) ? -1.0 : 0.0 );
            }
            res.c.push_back(0.0);
            res.var_names.push_back("s" + to_string(slackCount++));
            res.var_signs.push_back(VarSign::POSITIVE);
        }
        res.signs.push_back(ConstraintType::EQ);
    }

    res.n = res.c.size();
    return res;
}

LPProblem LPProblem::toSymmetric() {
    LPProblem res;
    res.isMin = this->isMin;
    
   for (int j = 0; j < n; ++j) {
        if (var_signs[j] == VarSign::FREE) {
            res.c.push_back(this->c[j]);
            res.var_names.push_back("x" + to_string(j + 1) + "'");
            res.var_signs.push_back(VarSign::POSITIVE);

            res.c.push_back(-this->c[j]);
            res.var_names.push_back("x" + to_string(j + 1) + "''");
            res.var_signs.push_back(VarSign::POSITIVE);
        } else if (var_signs[j] == VarSign::NEGATIVE) {
            res.c.push_back(-this->c[j]);
            res.var_names.push_back("x" + to_string(j + 1) + "*");
            res.var_signs.push_back(VarSign::POSITIVE);
        } else {
            res.c.push_back(this->c[j]);
            res.var_names.push_back("x" + to_string(j + 1));
            res.var_signs.push_back(VarSign::POSITIVE);
        }
    }
    res.n = res.c.size();

    ConstraintType targetSign = res.isMin ? ConstraintType::GEQ : ConstraintType::LEQ;
    for (int i = 0; i < m; ++i) {
        auto prepareRow = [&](double multiplier) {
            vector<double> newRow;
            for (int j = 0; j < n; ++j) {
                if (var_signs[j] == VarSign::FREE) {
                    newRow.push_back(this->A[i][j] * multiplier);
                    newRow.push_back(-this->A[i][j] * multiplier);
                } 
                else if (var_signs[j] == VarSign::NEGATIVE) {
                    newRow.push_back(-this->A[i][j] * multiplier);
                } 
                else {
                    newRow.push_back(this->A[i][j] * multiplier);
                }
            }
            return newRow;
        };

        if (signs[i] == ConstraintType::EQ) {
            double mult1 = (res.isMin) ? 1.0 : 1.0;
            res.A.push_back(prepareRow(mult1));
            res.b.push_back(this->b[i] * mult1);
            res.signs.push_back(targetSign);

            double mult2 = -1.0;
            res.A.push_back(prepareRow(mult2));
            res.b.push_back(this->b[i] * mult2);
            res.signs.push_back(targetSign);
        } 
        else {
            double mult = (signs[i] == targetSign) ? 1.0 : -1.0;
            res.A.push_back(prepareRow(mult));
            res.b.push_back(this->b[i] * mult);
            res.signs.push_back(targetSign);
        }
    }
    
    res.m = res.A.size();
    return res;
}

void LPProblem::printForms() {
    cout << "\n========================================\n";
    cout << "       ПОЛНЫЙ АНАЛИЗ ФОРМ ЗАДАЧИ        \n";
    cout << "========================================\n";

    ProblemForm currentForm = determineForm();


    cout << "\n>>> ИСХОДНАЯ ЗАДАЧА (ВВЕДЕННАЯ ПОЛЬЗОВАТЕЛЕМ) <<<\n";
    printOriginal();
    cout << "\n>>> ВЫВОД ДВОЙСТЕННОЙ ЗАДАЧИ К ТЕКУЩЕЙ <<<\n";
    printDual();

    if (currentForm != ProblemForm::GENERAL) {
        cout << "\n>>> ПРИВЕДЕНИЕ К ОБЩЕЙ ФОРМЕ (ИСКУССТВЕННОЕ УСЛОЖНЕНИЕ) <<<\n";
        LPProblem general = toGeneral();
        general.printOriginal();
        cout << "\n>>> ВЫВОД ДВОЙСТЕННОЙ ЗАДАЧИ К ТЕКУЩЕЙ <<<\n";
        general.printDual();
    } 
    else {
        cout << "\n[!] Задача уже в общей форме, пропуск шага приведения.\n";
    }

    if (currentForm != ProblemForm::CANONICAL) {
        cout << "\n>>> ПРИВЕДЕНИЕ К КАНОНИЧЕСКОЙ ФОРМЕ <<<\n";
        LPProblem canonical = toCanonical();
        canonical.printOriginal();
        cout << "\n>>> ВЫВОД ДВОЙСТЕННОЙ ЗАДАЧИ К ТЕКУЩЕЙ <<<\n";
        canonical.printDual();
    } 
    else {
        cout << "\n[!] Задача уже в канонической форме, пропуск шага приведения.\n";
    }

    if (currentForm != ProblemForm::SYMMETRIC) {
        cout << "\n>>> ПРИВЕДЕНИЕ К СИММЕТРИЧНОЙ ФОРМЕ <<<\n";
        LPProblem symmetric = toSymmetric();
        symmetric.printOriginal();
        cout << "\n>>> ВЫВОД ДВОЙСТЕННОЙ ЗАДАЧИ К ТЕКУЩЕЙ <<<\n";
        symmetric.printDual();
    } 
    else {
        cout << "\n[!] Задача уже в симметричной форме, пропуск шага приведения.\n";
    }
}

void LPProblem::solveSimplex(bool verbose) {
    // Симплекс метод
}

void LPProblem::solveByVertices() {
    // Метод перебора крайних точек
}