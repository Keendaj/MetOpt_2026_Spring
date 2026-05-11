import numpy as np

class LPProblem:
    def __init__(self, c, A, b, signs=None, is_free=None, is_min=True, eps=1e-9, verbose=False):
        self.c = np.array(c, dtype=float)
        self.A = np.array(A, dtype=float)
        self.b = np.array(b, dtype=float)
        self.m, self.n_orig = self.A.shape
        
        self.signs = signs if signs else ['<='] * self.m
        self.is_free = is_free if is_free else [False] * self.n_orig
        
        self.is_min = is_min
        self.eps = eps
        self.verbose = verbose
        self.optimal_solution = None
        self.optimal_value = None

    def _canonicalize(self):
        n_canon = 0
        self.orig_to_canon = []
        for i in range(self.n_orig):
            if self.is_free[i]:
                self.orig_to_canon.append((n_canon, n_canon + 1))
                n_canon += 2
            else:
                self.orig_to_canon.append((n_canon,))
                n_canon += 1
        
        A_canon = []
        for i in range(self.m):
            row = []
            for j in range(self.n_orig):
                if self.is_free[j]:
                    row.extend([self.A[i, j], -self.A[i, j]])
                else:
                    row.append(self.A[i, j])
            A_canon.append(row)
        A_canon = np.array(A_canon, dtype=float)
        
        c_canon = []
        for j in range(self.n_orig):
            if self.is_free[j]:
                c_canon.extend([self.c[j], -self.c[j]])
            else:
                c_canon.append(self.c[j])
        c_canon = np.array(c_canon, dtype=float)
        
        n_slacks = sum(1 for s in self.signs if s != '==')
        total_vars = n_canon + n_slacks
        
        A_full = np.zeros((self.m, total_vars))
        A_full[:, :n_canon] = A_canon
        
        slack_idx = n_canon
        for i in range(self.m):
            if self.signs[i] == '<=':
                A_full[i, slack_idx] = 1.0
                slack_idx += 1
            elif self.signs[i] == '>=':
                A_full[i, slack_idx] = -1.0
                slack_idx += 1
                
        b_full = self.b.copy()
        for i in range(self.m):
            if b_full[i] < 0:
                A_full[i] *= -1.0
                b_full[i] *= -1.0
                
        return c_canon, A_full, b_full, n_canon

    def _print_table(self, table, basis, phase, step, min_row, min_col, n_real):
        """Красивый вывод симплекс-таблицы (как в C++)"""
        m = self.m
        total_vars = n_real + m
        
        print(f"\n{'='*80}")
        print(f"[ ФАЗА {phase} | ШАГ {step} ]")
        if min_col != -1 and min_row != -1:
            v_in = f"x{min_col+1}" if min_col < n_real else f"a{min_col-n_real+1}"
            v_out = f"x{basis[min_row]+1}" if basis[min_row] < n_real else f"a{basis[min_row]-n_real+1}"
            print(f"Входит в базис: {v_in} | Выходит из базиса: {v_out} | Разрешающий элемент: {table[min_row, min_col]:.4f}")
        else:
            print("Оптимальный план для текущей фазы достигнут.")
        print(f"{'='*80}")
        
        headers = ["Базис", "b"] + [f"x{i+1}" for i in range(n_real)] + [f"a{i+1}" for i in range(m)]
        row_format = "{:>6} | {:>8} | " + " ".join(["{:>8}"] * total_vars)
        
        print(row_format.format(*headers))
        print("-" * (18 + 9 * total_vars))
        
        for i in range(m):
            b_val = f"{table[i, -1]:.4f}"
            b_name = f"x{basis[i]+1}" if basis[i] < n_real else f"a{basis[i]-n_real+1}"
            
            row_vals = []
            for j in range(total_vars):
                val_str = f"{table[i, j]:.4f}"
                if i == min_row and j == min_col:
                    val_str = f"*{val_str}*"
                row_vals.append(val_str)
                
            print(row_format.format(b_name, b_val, *row_vals))
            
        print("-" * (18 + 9 * total_vars))
        
        z_val = f"{-table[-1, -1]:.4f}"
        z_row_vals = [f"{v:.4f}" for v in table[-1, :-1]]
        print(row_format.format("Z", z_val, *z_row_vals))
        print(f"{'='*80}")

    def _jordan_step(self, table, basis, step_row, step_col):
        table[step_row] /= table[step_row, step_col]
        mask = np.arange(table.shape[0]) != step_row
        factors = table[mask, step_col][:, np.newaxis]
        table[mask] -= factors * table[step_row]
        basis[step_row] = step_col

    def _simplex_phase(self, table, basis, phase, n_vars):
        m = self.m
        step = 1
        while True:
            z_row = table[-1, :n_vars]
            
            if np.min(z_row) >= -self.eps:
                if self.verbose:
                    self._print_table(table, basis, phase, step, -1, -1, n_vars)
                break
                
            min_col = np.argmin(z_row)
            col_vals = table[:-1, min_col]
            rhs_vals = table[:-1, -1]
            
            valid_rows = col_vals > self.eps
            if not np.any(valid_rows):
                if self.verbose:
                    print(f"\n[!] ОШИБКА (Фаза {phase}): Целевая функция не ограничена снизу.")
                return False
                    
            ratios = np.full(m, np.inf)
            ratios[valid_rows] = rhs_vals[valid_rows] / col_vals[valid_rows]
            min_row = np.argmin(ratios)

            if self.verbose:
                self._print_table(table, basis, phase, step, min_row, min_col, n_vars)
                if abs(table[min_row, -1]) < self.eps:
                    print(f"[*] Шаг {step} (Фаза {phase}): Смена базиса для ВЫРОЖДЕННОГО опорного вектора (b = 0).")

            self._jordan_step(table, basis, min_row, min_col)
            step += 1
            
        return True

    def solve(self):
        c_canon, A_full, b_full, n_canon = self._canonicalize()
        m, n = A_full.shape
        
        cols = n + m + 1
        table = np.zeros((m + 1, cols), dtype=float)

        table[:m, :n] = A_full
        table[:m, n:n+m] = np.eye(m)
        table[:m, -1] = b_full
        
        basis = np.arange(n, n + m)

        table[-1, :] = -np.sum(table[:m, :], axis=0)
        table[-1, n:n+m] = 0.0 
        
        if self.verbose:
            print(f"\n    === СТАРТ СИМПЛЕКС-МЕТОДА (m={m}, n_real={n}) ===")

        if not self._simplex_phase(table, basis, phase=1, n_vars=n):
            return None
            
        if abs(table[-1, -1]) > self.eps:
            if self.verbose:
                print(f"\n[!] СИСТЕМА НЕСОВМЕСТНА! Минимум суммы иск. переменных W* = {-table[-1, -1]:.4f} (а должен быть 0).")
            return None
            
        if self.verbose:
            print("\n[+] Фаза 1 успешно завершена. Начальный опорный план найден!")

        table[-1, :] = 0.0
        table[-1, :n_canon] = c_canon if self.is_min else -c_canon
        
        for i in range(m):
            table[-1] -= table[-1, basis[i]] * table[i]

        if not self._simplex_phase(table, basis, phase=2, n_vars=n):
            return None

        x_canon = np.zeros(n)
        for i in range(m):
            if basis[i] < n:
                x_canon[basis[i]] = table[i, -1]
                
        self.optimal_solution = np.zeros(self.n_orig)
        for i in range(self.n_orig):
            mapping = self.orig_to_canon[i]
            if len(mapping) == 1:
                self.optimal_solution[i] = x_canon[mapping[0]]
            else:
                self.optimal_solution[i] = x_canon[mapping[0]] - x_canon[mapping[1]]
                
        self.optimal_value = -table[-1, -1] if self.is_min else table[-1, -1]
        
        if self.verbose:
            print(f"    === КОНЕЦ СИМПЛЕКС-МЕТОДА. Найдено y* = {self.optimal_solution} ===")
            
        return self.optimal_solution