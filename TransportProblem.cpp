#include "TransportProblem.h"
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <limits>

using namespace std;

void TransportProblem::input() {
    
    cout << "\n--- РУЧНОЙ ВВОД ДАННЫХ ---\n";
    cout << "[!] Подсказки:\n";
    cout << "  - Вводите числа через пробел или клавишу Enter.\n";
    cout << "  - Для дробных чисел используйте точку (например, 2.5, а не 2,5).\n";
    cout << "  - Все значения (запасы, потребности, тарифы) должны быть неотрицательными.\n\n";

    cout << "Введите количество поставщиков (m): ";
    while (!(cin >> m) || m <= 0) {
        cout << "Ошибка! Введите целое положительное число: ";
        cin.clear();
        cin.ignore(10000, '\n');
    }

    cout << "Введите количество потребителей (n): ";
    while (!(cin >> n) || n <= 0) {
        cout << "Ошибка! Введите целое положительное число: ";
        cin.clear();
        cin.ignore(10000, '\n');
    }

    A.resize(m);
    cout << "\nВведите запасы поставщиков (A) - " << m << " чисел:\n> ";
    for (int i = 0; i < m; ++i) cin >> A[i];

    B.resize(n);
    cout << "\nВведите потребности (B) - " << n << " чисел:\n> ";
    for (int j = 0; j < n; ++j) cin >> B[j];

    C.assign(m, vector<double>(n));
    cout << "\nВведите матрицу тарифов (C) - " << m << " строк по " << n << " чисел:\n";
    for (int i = 0; i < m; ++i) {
        cout << "Строка " << i + 1 << ": ";
        for (int j = 0; j < n; ++j) {
            cin >> C[i][j];
        }
    }
}

void TransportProblem::setData(const vector<double>& sup, const vector<double>& dem, const vector<vector<double>>& costs) {
    A = sup; B = dem; C = costs;
    m = A.size(); n = B.size();
}

void TransportProblem::buildNorthWestCorner() {
    vector<double> a_copy = A; vector<double> b_copy = B;
    plan.assign(m, vector<TransportCell>(n));
    int i = 0, j = 0;
    while (i < m && j < n) {
        plan[i][j].isBasic = true;
        double x = min(a_copy[i], b_copy[j]);
        plan[i][j].amount = x;
        a_copy[i] -= x;
        b_copy[j] -= x;

        if (abs(a_copy[i]) < 1e-9 && abs(b_copy[j]) < 1e-9) {
            if (i + 1 < m && j + 1 < n) 
            {
                i++;
                plan[i][j].isBasic = true; 
            } 
            else 
            { 
                i++;
                j++;
            }
        } 
        else if (abs(a_copy[i]) < 1e-9) i++;
        else j++;
    }
}

bool TransportProblem::calculatePotentials(vector<double>& u, vector<double>& v) {
    vector<bool> u_calc(m, false), v_calc(n, false);
    u[0] = 0.0; 
    u_calc[0] = true;
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                if (plan[i][j].isBasic) {
                    if (u_calc[i] && !v_calc[j]) 
                    { 
                        v[j] = C[i][j] - u[i]; 
                        v_calc[j] = true; 
                        changed = true; 
                    } 
                    else if (!u_calc[i] && v_calc[j]) {
                        u[i] = C[i][j] - v[j]; 
                        u_calc[i] = true; 
                        changed = true; 
                    }
                }
            }
        }
    }
    for (bool b : u_calc) if (!b) return false;
    for (bool b : v_calc) if (!b) return false;
    return true;
}

bool TransportProblem::findCycleIterative(int start_r, int start_c, bool startHorizontal, vector<pair<int, int>>& path) {
    struct State {
        int r, c;
        bool isHoriz;
        int iter;
    };
    
    vector<State> st;
    vector<vector<bool>> visited(m, vector<bool>(n, false));
    
    st.push_back({start_r, start_c, startHorizontal, 0});
    visited[start_r][start_c] = true;
    path.push_back({start_r, start_c});

    while (!st.empty()) {
        auto& curr = st.back();
        int r = curr.r;
        int c = curr.c;
        bool isHoriz = curr.isHoriz;

        bool foundNext = false;
        int max_iter = isHoriz ? n : m;
        cout << "(" << r << "," << c << ") -> ";
        while (curr.iter < max_iter) {
            int next_idx = curr.iter++;
            int nr = isHoriz ? r : next_idx;
            int nc = isHoriz ? next_idx : c;
            
            if ((isHoriz && nc == c) || (!isHoriz && nr == r)) continue;

            if (plan[nr][nc].isBasic || (nr == start_r && nc == start_c)) {
                if (nr == start_r && nc == start_c && path.size() >= 4) {
                    path.push_back({nr, nc});
                    cout << "\n";
                    return true;
                }
                
                if (!visited[nr][nc]) {
                    visited[nr][nc] = true;
                    path.push_back({nr, nc});
                    st.push_back({nr, nc, !isHoriz, 0});
                    foundNext = true;
                    break;
                }
            }
        }
        
        if (!foundNext) {
            visited[r][c] = false;
            path.pop_back();
            st.pop_back();
        }
    }
    cout << "\n";
    return false;
}

void TransportProblem::printTransportTable(int step, const vector<double>& u, const vector<double>& v) {
    cout << "\n--- Итерация " << step << " ---\n";
    cout << setw(8) << "v_j |";
    for (int j = 0; j < n; ++j) cout << setw(10) << v[j];
    cout << "\n";
    for (int i = 0; i < 10 + n * 10; ++i) cout << "-";
    cout << "\n";

    for (int i = 0; i < m; ++i) {
        cout << "u" << i+1 << "=" << setw(3) << u[i] << " |";
        for (int j = 0; j < n; ++j) {
            string cell = "";
            if (plan[i][j].isBasic) {
                cell = to_string((int)plan[i][j].amount);
            } else {
                cell = "-";
            }
            cell += "(" + to_string((int)C[i][j]) + ")";
            cout << setw(10) << cell;
        }
        cout << "\n";
    }
    for (int i = 0; i < 10 + n * 10; ++i) cout << "-";
    cout << "\n";
}

bool TransportProblem::solvePotential(int print_k) {
    buildNorthWestCorner();
    int iteration = 1;

    while (true) {
        vector<double> u(m, 0.0), v(n, 0.0);
        if (!calculatePotentials(u, v)) return false;

        if (print_k > 0 && (iteration == 1 || iteration % print_k == 0)) {
            printTransportTable(iteration, u, v);
        }

        double minDelta = 0.0;
        int enter_r = -1, enter_c = -1;

        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                if (!plan[i][j].isBasic) {
                    double delta = C[i][j] - (u[i] + v[j]);
                    if (delta < minDelta - 1e-9) {
                        minDelta = delta;
                        enter_r = i; 
                        enter_c = j;
                    }
                }
            }
        }

        if (enter_r == -1) {
            optimalPotentialValue = 0.0;
            for (int i = 0; i < m; ++i)
                for (int j = 0; j < n; ++j)
                    if (plan[i][j].isBasic) 
                        optimalPotentialValue += plan[i][j].amount * C[i][j];
            
            if (print_k > 0) {
                cout << "[+] План оптимален! (Все оценки Delta >= 0)\n";
            }
            return true;
        }

        vector<pair<int, int>> path;
        if (!findCycleIterative(enter_r, enter_c, true, path)) {
            path.clear();
            findCycleIterative(enter_r, enter_c, false, path);
        }
        path.pop_back();

        double theta = numeric_limits<double>::max();
        int leave_r = -1, leave_c = -1;
        for (size_t k = 1; k < path.size(); k += 2) {
            int r = path[k].first, c = path[k].second;
            if (plan[r][c].amount < theta) {
                theta = plan[r][c].amount; 
                leave_r = r; 
                leave_c = c;
            }
        }

        for (size_t k = 0; k < path.size(); ++k) {
            if (k % 2 == 0) plan[path[k].first][path[k].second].amount += theta;
            else plan[path[k].first][path[k].second].amount -= theta;
        }

        plan[enter_r][enter_c].isBasic = true;
        plan[leave_r][leave_c].isBasic = false;
        
        if (print_k > 0 && iteration % print_k == 0) {
            cout << " -> Ввод: x[" << enter_r+1 << "][" << enter_c+1 << "], Вывод: x[" 
                 << leave_r+1 << "][" << leave_c+1 << "], Сдвиг Theta = " << theta << "\n";
        }
        iteration++;
    }
}

void TransportProblem::printSimplexTable(const vector<vector<double>>& table, const vector<int>& basis, int vars, int eqs, int phase, int step) {
    int cols = table[0].size();
    cout << "\nТаблица " << step << " (Фаза " << phase << ")\n";
    
    cout << setw(6) << "Базис" << " | " << setw(8) << "b" << " | ";
    for (int j = 0; j < vars; ++j) cout << setw(8) << "x" + to_string(j + 1);
    
    if (phase == 1) {
        for (int j = vars; j < cols - 1; ++j) cout << setw(8) << "a" + to_string(j - vars + 1);
    }
    cout << "\n";

    int table_width = 18 + vars * 8 + (phase == 1 ? (cols - 1 - vars) * 8 : 0);
    for (int i = 0; i < table_width; ++i) cout << "-";
    cout << "\n";

    for (int i = 0; i < eqs; ++i) {
        string bName = (basis[i] < vars) ? "x" + to_string(basis[i] + 1) : "a" + to_string(basis[i] - vars + 1);
        cout << setw(6) << bName << " | " << setw(8) << fixed << setprecision(2) << table[i].back() << " | ";
        
        for (int j = 0; j < vars; ++j) cout << setw(8) << table[i][j];
        if (phase == 1) {
            for (int j = vars; j < cols - 1; ++j) cout << setw(8) << table[i][j];
        }
        cout << "\n";
    }

    cout << setw(6) << "D_j" << " | " << setw(8) << fixed << setprecision(2) << table[eqs].back() << " | ";
    for (int j = 0; j < vars; ++j) cout << setw(8) << table[eqs][j];
    if (phase == 1) {
        for (int j = vars; j < cols - 1; ++j) cout << setw(8) << table[eqs][j];
    }
    cout << "\n";
}

void TransportProblem::JordanStep(vector<vector<double>>& table, vector<int>& basis, int pivotRow, int pivotCol) {
    int cols = table[0].size();
    double pVal = table[pivotRow][pivotCol];
    for (int j = 0; j < cols; ++j) table[pivotRow][j] /= pVal;
    
    for (size_t i = 0; i < table.size(); ++i) {
        if (i != pivotRow) {
            double factor = table[i][pivotCol];
            for (int j = 0; j < cols; ++j) table[i][j] -= factor * table[pivotRow][j];
        }
    }
    basis[pivotRow] = pivotCol;
}

bool TransportProblem::phase1(vector<vector<double>>& table, vector<int>& basis, int vars, int eqs, double eps, int print_k) {
    int cols = table[0].size();

    for (int j = vars; j < vars + eqs; ++j) table[eqs][j] = -1.0;
    for (int i = 0; i < eqs; ++i) {
        for (int j = 0; j < cols; ++j) table[eqs][j] -= table[i][j];
    }

    int step = 1;
    while (true) {
        if (print_k > 0 && (step == 1 || step % print_k == 0)) {
            printSimplexTable(table, basis, vars, eqs, 1, step);
        }

        int pivotCol = -1; double minVal = -eps;
        for (int j = 0; j < vars; ++j) {
            if (table[eqs][j] < minVal) { minVal = table[eqs][j]; pivotCol = j; }
        }
        if (pivotCol == -1) break;

        int pivotRow = -1; double minRatio = 1e18;
        for (int i = 0; i < eqs; ++i) {
            if (table[i][pivotCol] > eps) {
                double ratio = table[i].back() / table[i][pivotCol];
                if (ratio < minRatio) { minRatio = ratio; pivotRow = i; }
            }
        }
        if (pivotRow == -1) return false;
        JordanStep(table, basis, pivotRow, pivotCol);
        step++;
    }

    if (table[eqs].back() < -eps) return false; 
    return true;
}

bool TransportProblem::phase2(vector<vector<double>>& table, vector<int>& basis, int vars, int eqs, double eps, int print_k) {
    int cols = table[0].size();
    
    for (int j = 0; j < cols; ++j) table[eqs][j] = 0.0;
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) table[eqs][i * n + j] = C[i][j];
    }

    for (int i = 0; i < eqs; ++i) {
        if (basis[i] < vars) {
            double factor = table[eqs][basis[i]];
            for (int j = 0; j < cols; ++j) table[eqs][j] -= factor * table[i][j];
        }
    }

    int step = 1;
    while (true) {
        if (print_k > 0 && (step == 1 || step % print_k == 0)) {
            printSimplexTable(table, basis, vars, eqs, 2, step);
        }

        int pivotCol = -1; double minVal = -eps;
        for (int j = 0; j < vars; ++j) {
            if (table[eqs][j] < minVal) { minVal = table[eqs][j]; pivotCol = j; }
        }
        if (pivotCol == -1) break;

        int pivotRow = -1; double minRatio = 1e18;
        for (int i = 0; i < eqs; ++i) {
            if (table[i][pivotCol] > eps) {
                double ratio = table[i].back() / table[i][pivotCol];
                if (ratio < minRatio) { minRatio = ratio; pivotRow = i; }
            }
        }
        if (pivotRow == -1) return false;
        JordanStep(table, basis, pivotRow, pivotCol);
        step++;
    }
    return true;
}

bool TransportProblem::solveSimplex(double eps, int print_k) {
    int vars = m * n;
    int eqs = m + n - 1; 

    int cols = vars + eqs + 1;
    vector<vector<double>> table(eqs + 1, vector<double>(cols, 0.0));
    vector<int> basis(eqs);

    int row = 0;
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) table[row][i * n + j] = 1.0;
        table[row][vars + row] = 1.0; 
        table[row].back() = A[i];
        basis[row] = vars + row;
        row++;
    }
    for (int j = 0; j < n - 1; ++j) {
        for (int i = 0; i < m; ++i) table[row][i * n + j] = 1.0;
        table[row][vars + row] = 1.0; 
        table[row].back() = B[j];
        basis[row] = vars + row;
        row++;
    }

    if (print_k > 0) cout << "\n--- ЗАПУСК ФАЗЫ 1 (Искусственный базис) ---\n";
    if (!phase1(table, basis, vars, eqs, eps, print_k)) return false;

    if (print_k > 0) cout << "\n--- ЗАПУСК ФАЗЫ 2 (Поиск оптимума) ---\n";
    if (!phase2(table, basis, vars, eqs, eps, print_k)) return false;

    optimalSimplexPlan.assign(vars, 0.0);
    for (int i = 0; i < eqs; ++i) {
        if (basis[i] < vars) optimalSimplexPlan[basis[i]] = table[i].back();
    }
    optimalSimplexValue = -table[eqs].back();
    return true;
}

void TransportProblem::convertToOpenWithPenalties(int variant_number, const vector<vector<PenaltyTier>>& supplier_tiers) {
    using namespace std;
    
    double max_cost = 0.0;
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            if (C[i][j] != -1 && C[i][j] > max_cost) {
                max_cost = C[i][j];
            }
        }
    }

    double INF = (max_cost == 0.0) ? 1000.0 : max_cost * 10.0; 

    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            if (C[i][j] == -1) {
                C[i][j] = INF;
            }
        }
    }

    double rem = variant_number;
    for (int j = n - 1; j >= 0 && rem > 0; --j) {
        if (B[j] >= rem) { B[j] -= rem; rem = 0; } 
        else { rem -= B[j]; B[j] = 0; }
    }
    
    double sumA = 0, sumB = 0;
    for (double sup : A) sumA += sup;
    for (double dem : B) sumB += dem;

    double excess = sumA - sumB;

    if (excess < -1e-9) {
        double deficit = sumB - sumA;
        A.push_back(deficit); 
        
        vector<double> dummy_row(n, 0.0);
        C.push_back(dummy_row);
        m++;
        
        return; 
    }

    if (abs(excess) <= 1e-9) return;

    int original_m = m;
    int original_n = n;
    double total_dummy_capacity = 0.0;

    for (int i = 0; i < original_m; ++i) {
        if (i >= supplier_tiers.size()) continue; 

        for (const auto& tier : supplier_tiers[i]) {
            double cap = min(tier.capacity, excess); 
            if (cap <= 1e-9) continue;

            B.push_back(cap);
            total_dummy_capacity += cap;
            n++;

            for (int k = 0; k < original_m; ++k) {
                if (k == i) {
                    C[k].push_back(tier.penalty); 
                } else {
                    C[k].push_back(INF);         
                }
            }
        }
    }

    if (total_dummy_capacity > excess + 1e-9) {
        A.push_back(total_dummy_capacity - excess);
        
        vector<double> dummy_row(n, 0.0);
        for (int j = 0; j < n; ++j) {
            if (j < original_n) {
                dummy_row[j] = INF; 
            } else {
                dummy_row[j] = 0.0;
            }
        }
        C.push_back(dummy_row);
        m++;
    }
    else if (total_dummy_capacity < excess - 1e-9) {
        B.push_back(excess - total_dummy_capacity);
        n++;
        for (int i = 0; i < original_m; ++i) {
            if (i < supplier_tiers.size() && !supplier_tiers[i].empty()) {
                C[i].push_back(INF);
            } else {
                C[i].push_back(0.0);
            }
        }
    }
}

void TransportProblem::printPotentialPlan() {
    using namespace std;
    if (plan.empty()) {
        cout << "[!] План не построен. Сначала запустите solve_potential.\n";
        return;
    }
    cout << "Оптимальные транспортные расходы (F_min): " << optimalPotentialValue << "\n";
    cout << "Матрица оптимального плана X* (метод потенциалов):\n";
    cout << "--------------------------------------------------\n";
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            if (plan[i][j].isBasic) cout << setw(6) << fixed << setprecision(1) << plan[i][j].amount;
            else cout << setw(6) << "-";
        }
        cout << "\n";
    }
    cout << "--------------------------------------------------\n";
}

void TransportProblem::printSimplexPlan() {
    using namespace std;
    if (optimalSimplexPlan.empty()) {
        cout << "[!] Решение не найдено. Сначала запустите solve_simplex.\n";
        return;
    }
    cout << "Экстремум целевой функции (F_min): " << optimalSimplexValue << "\n";
    cout << "Матрица оптимального плана X* (симплекс-метод):\n";
    cout << "--------------------------------------------------\n";
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            double val = optimalSimplexPlan[i * n + j];
            if (abs(val) > 1e-9) {
                cout << setw(6) << fixed << setprecision(1) << val;
            } else {
                cout << setw(6) << "-";
            }
        }
        cout << "\n";
    }
    cout << "--------------------------------------------------\n";
}