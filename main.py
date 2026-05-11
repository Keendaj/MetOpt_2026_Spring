import sys
import numpy as np
from functions import TestFunction2D, SynthFunction3D, HardcoreFunction3D, CustomFunction
from optimizers import ConditionalGradient
from analitics import InteractiveVisualizer2D, Logger

def print_help():
    print("\n--- Доступные команды ---")
    print("help              - Показать это меню")
    print("show_task         - Вывести полную информацию о текущей задаче")
    print("show_funcs        - Показать доступные функции")
    print("show_consts       - Показать доступные ограничения")
    print("pick_func [name]  - Выбрать активную функцию")
    print("pick_const [name] - Выбрать набор ограничений")
    print("set_start [x...]  - Задать начальную точку (например: set_start 1.0 1.0 1.0)")
    print("set_epsilon [v]   - Задать точность epsilon")
    print("set_fw [a0] [lam] - Задать alpha_0 и lambda для Франка-Вульфа")
    print("add_func [name] [vars] [expr] - Добавить новую функцию (пример: add_func my x1,x2 x1**2+x2**2)")
    print("add_const [name] [m_rows] [n_cols] - Добавить новую матрицу ограничений (интерактивно)")
    print("solve [--print]   - Решить текущую задачу (print для вывода логов решения)")
    print("log               - Вывести численный лог")
    print("visualize         - Открыть графики (только для 2d и 3d)")
    print("analitics         - Анализ Гессиана и миноров")
    print("exit              - Выход")
    print("-------------------------\n")

def main():
    functions = {
        "2d": TestFunction2D(),
        "3d": SynthFunction3D(),
        "3d_hard": HardcoreFunction3D()
    }
    
    start_points = {
        "2d": np.array([4.5, 4.5]),          
        "3d": np.array([3.0, -2.0, 4.0]),    
        "3d_hard": np.array([3.0, 2.0, 1.0])
    }
    
    constraints_db = {
        "2d_inside": (
            np.array([
                [ 1.0,  1.0],
                [ 1.0, -2.0],
                [-2.0,  1.0],
                [-1.0, -1.0]
            ]),
            np.array([15.0, 15.0, 15.0, 15.0]),
            ['<=', '<=', '<=', '<='],
            [True, True] 
        ),
        "2d_boundary": (
            np.array([
                [ 1.0,  2.0], 
                [ 2.0,  1.0],
                [ 1.0,  1.0],  
                [-1.0,  2.0], 
                [ 2.0, -1.0]  
            ]),
            np.array([6.0, 6.0, 15.0, 10.0, 10.0]),
            ['>=', '>=', '<=', '<=', '<='],
            [True, True]
        ),
        
        "3d_inside": (
            np.array([
                [ 1.0,  1.0,  1.0],
                [ 1.0,  1.0, -1.0],
                [ 1.0, -1.0,  1.0],
                [ 1.0, -1.0, -1.0],
                [-1.0,  1.0,  1.0],
                [-1.0,  1.0, -1.0],
                [-1.0, -1.0,  1.0],
                [-1.0, -1.0, -1.0]
            ]), 
            np.array([20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0]),
            ['<=', '<=', '<=', '<=', '<=', '<=', '<=', '<='],
            [True, True, True] 
        ),
        "3d_boundary": (
            np.array([
                [ 1.0,  1.0,  1.0],  
                [ 2.0, -1.0,  1.0], 
                [ 1.0,  2.0,  3.0],  
                [ 1.0,  1.0,  1.0],  
                [ 1.0, -1.0,  1.0], 
                [-1.0, -1.0, -1.0],  
                [-1.0,  1.0, -1.0]   
            ]),
            np.array([3.0, 7.0, 6.0, 15.0, 15.0, 15.0, 15.0]),
            ['>=', '>=', '>=', '<=', '<=', '<=', '<='],
            [True, True, True]
        )
    }
    
    active_func = "2d"
    active_opt = "grad"
    active_const = "2d_boundary"
    active_epsilon = 1e-2

    optimizers = {
        "grad": ConditionalGradient(
            A=np.eye(1), b=np.array([1.0]), signs=['<='], is_free=[False],
            eps=active_epsilon, alpha_0=1.0, lam=0.5
        )
    }

    results = {}

    def print_current_task():
        func = functions.get(active_func)
        x0 = start_points.get(active_func)
        print("\n" + "="*65)
        print(" "*16 + "[ ТЕКУЩАЯ ЗАДАЧА ОПТИМИЗАЦИИ ]")
        print("="*65)
        print(f"[*] Функция:         {active_func}")
        if func:
            print(f"[*] Формула:         {func.expr}")
        if x0 is not None:
            print(f"[*] Стартовая точка: {x0}")
        print(f"[*] Точность (eps):  {active_epsilon}")
        print(f"[*] Метод решения:   {active_opt.upper()} (Условный градиент)")
        print("-" * 65)
        print(f"[*] Ограничения:     {active_const}")
        
        if active_const in constraints_db:
            A_current, b_current, signs_current, free_current = constraints_db[active_const]
            print("\n    Матрица A:")
            for row in A_current:
                print("      " + "  ".join(f"{v:>6.2f}" for v in row))
            print("\n    Знаки:           " + "  ".join(f"{s:>6}" for s in signs_current))
            print("    Вектор b:        " + "  ".join(f"{v:>6.2f}" for v in b_current))
            
            free_vars = [f"x{i+1}" for i, f in enumerate(free_current) if f]
            pos_vars = [f"x{i+1}" for i, f in enumerate(free_current) if not f]
            print()
            if free_vars:
                print(f"    Свободные (∈ ℝ): {', '.join(free_vars)}")
            if pos_vars:
                print(f"    Только x >= 0:   {', '.join(pos_vars)}")
        else:
            print("    [!] Ограничения не заданы или отсутствуют.")
        print("="*65 + "\n")

    print("Программа минимизации функций запущена. Инженерный режим.")
    print_help()

    while True:
        prompt = f"\n[{active_func} | {active_opt} | const={active_const} | eps={active_epsilon}] > "
        try:
            line = input(prompt).strip()
        except EOFError:
            break

        if not line:
            continue

        parts = line.split()
        command = parts[0].lower()

        if command == "exit":
            break
        elif command == "help":
            print_help()
        elif command == "show_task":
            print_current_task()
        elif command == "show_funcs":
            print("\n[*] Доступные функции:")
            for k, v in functions.items():
                print(f"    - {k} ({len(v.vars)}D)")

        elif command == "show_consts":
            print("\n[*] Доступные ограничения:")
            for k, v in constraints_db.items():
                A, b, signs, is_free = v
                print(f"    - {k}: Матрица {A.shape[0]}x{A.shape[1]}")
        elif command == "add_func":
            if len(parts) >= 4:
                name = parts[1]
                vars_str = parts[2].replace(",", " ")
                expr_str = " ".join(parts[3:])
                try:
                    functions[name] = CustomFunction(expr_str, vars_str)
                    n_vars = len(functions[name].vars)
                    start_points[name] = np.zeros(n_vars)
                    print(f"[+] Функция '{name}' ({expr_str}) успешно сохранена в историю.")
                except Exception as e:
                    print(f"[!] Ошибка разбора функции: {e}")
            else:
                print("[!] Использование: add_func [name] [vars_comma_separated] [expression]")
                
        elif command == "add_const":
            if len(parts) == 4:
                name = parts[1]
                try:
                    m = int(parts[2])
                    n = int(parts[3])
                    print(f"[*] Ввод матрицы A ({m} строк по {n} элементов через пробел):")
                    A_rows = []
                    for i in range(m):
                        row = list(map(float, input(f"    Строка {i+1}: ").strip().split()))
                        if len(row) != n:
                            raise ValueError(f"Ожидалось {n} элементов, введено {len(row)}.")
                        A_rows.append(row)
                    
                    print(f"[*] Ввод вектора b ({m} элементов через пробел):")
                    b_vals = list(map(float, input("    Вектор b: ").strip().split()))
                    if len(b_vals) != m:
                        raise ValueError(f"Ожидалось {m} элементов, введено {len(b_vals)}.")
                        
                    print(f"[*] Ввод знаков (<=, >=, ==) для каждой из {m} строк через пробел:")
                    signs = input("    Знаки: ").strip().split()
                    if len(signs) != m:
                        raise ValueError(f"Ожидалось {m} знаков.")
                        
                    print(f"[*] Ввод типа переменных (0 - неотрицательная, f - свободная) для каждой из {n} переменных через пробел:")
                    types = input("    Типы: ").strip().split()
                    if len(types) != n:
                        raise ValueError(f"Ожидалось {n} типов.")
                    is_free = [t.lower() == 'f' for t in types]
                        
                    constraints_db[name] = (np.array(A_rows), np.array(b_vals), signs, is_free)
                    print(f"[+] Ограничение '{name}' успешно добавлено в историю.")
                except Exception as e:
                    print(f"[!] Ошибка ввода: {e}")
            else:
                print("[!] Использование: add_const [name] [m_rows] [n_cols]")

        elif command == "pick_func":
            if len(parts) > 1 and parts[1] in functions:
                active_func = parts[1]
                print(f"[*] Активная функция: {active_func}")
            else:
                print(f"[!] Ошибка: функции нет. Доступны: {list(functions.keys())}")

        elif command == "pick_opt":
            if len(parts) > 1 and parts[1] in optimizers:
                active_opt = parts[1]
                print(f"[*] Активный метод: {active_opt}")
            else:
                print(f"[!] Ошибка: метода нет. Доступны: {list(optimizers.keys())}")
                
        elif command == "pick_const":
            if len(parts) > 1 and parts[1] in constraints_db:
                active_const = parts[1]
                print(f"[*] Активные ограничения: {active_const}")
            else:
                print(f"[!] Ошибка: ограничений нет. Доступны: {list(constraints_db.keys())}")

        elif command == "set_start":
            if len(parts) > 1:
                try:
                    coords = [float(x) for x in parts[1:]]
                    start_points[active_func] = np.array(coords)
                    print(f"[+] Начальная точка обновлена: {start_points[active_func]}")
                except ValueError:
                    print("[!] Ошибка: Координаты должны быть числами.")
            else:
                print("[!] Ошибка: укажите координаты.")

        elif command == "set_epsilon":
            if len(parts) > 1:
                try:
                    new_eps = float(parts[1])
                    active_epsilon = new_eps
                    for opt in optimizers.values():
                        opt.epsilon = active_epsilon
                    print(f"[+] Точность обновлена: {active_epsilon}")
                except ValueError:
                    print("[!] Ошибка: Значение должно быть числом.")

        elif command == "set_fw":
            if len(parts) == 3:
                try:
                    optimizers["frank_wolfe"].alpha_0 = float(parts[1])
                    optimizers["frank_wolfe"].lam = float(parts[2])
                    print(f"[+] Параметры обновлены: alpha_0={parts[1]}, lambda={parts[2]}")
                except ValueError:
                    print("[!] Ошибка: Значения должны быть числами.")

        elif command == "solve" or command == "solve --print":
            is_verbose = "--print" in line
            
            func = functions[active_func]
            opt = optimizers[active_opt]
            x0 = start_points[active_func]

            n_vars = len(func.vars)
            if len(x0) != n_vars:
                print(f"[!] ОШИБКА: Функция '{active_func}' зависит от {n_vars} переменных, а x0 имеет размерность {len(x0)}.")
                continue

            if active_opt == "grad":
                if active_const not in constraints_db:
                    print("[!] ОШИБКА: Ограничения не выбраны или удалены.")
                    continue
                    
                A_current, b_current, signs_current, free_current = constraints_db[active_const]
                
                opt.A = A_current
                opt.b = b_current
                opt.signs = signs_current
                opt.is_free = free_current
                
                if A_current.shape[1] != n_vars:
                    print(f"\n[!] ОШИБКА РАЗМЕРНОСТИ: Функция требует {n_vars} переменных, а столбцов в матрице A — {A_current.shape[1]}.")
                    continue

                is_valid = True
                for i in range(len(b_current)):
                    val = np.dot(A_current[i], x0)
                    if signs_current[i] == '<=' and val > b_current[i] + 1e-6:
                        print(f"\n[!] ОШИБКА: Точка x0 нарушает {i+1}-е ограничение (<=)")
                        is_valid = False
                    elif signs_current[i] == '>=' and val < b_current[i] - 1e-6:
                        print(f"\n[!] ОШИБКА: Точка x0 нарушает {i+1}-е ограничение (>=)")
                        is_valid = False
                    elif signs_current[i] == '==' and abs(val - b_current[i]) > 1e-6:
                        print(f"\n[!] ОШИБКА: Точка x0 нарушает {i+1}-е ограничение (==)")
                        is_valid = False
                
                for j in range(n_vars):
                    if not free_current[j] and x0[j] < -1e-6:
                        print(f"\n[!] ОШИБКА: Точка x0 содержит отрицательную координату x{j+1}, хотя она не свободная.")
                        is_valid = False
                        
                if not is_valid:
                    continue

            print_current_task()

            print(f"[*] Выполняется символьный анализ Гессиана...")
            try:
                func.analyze_convexity_symbolic()
            except AttributeError:
                pass
            
            is_continue = input("Введите Да, чтобы начать спуск (любой другой ввод отменит запуск):\n")
            if is_continue.lower() not in ["да", "yes", "y", "lf"]:
                print("Запуск отменен.")
                continue

            print(f"\n[*] Запуск метода '{active_opt}' из точки {x0}...")
            try:
                trajectory = opt.optimize(func, x0, verbose=is_verbose) 
                
                results_key = f"{active_func}_{active_const}_{active_opt}"
                results[results_key] = trajectory
                
                if not is_verbose:
                    print(f"\n[+] Решение завершено! Найдена точка: {trajectory[-1]}")
                    print(f"    Значение функции в минимуме: {func(trajectory[-1]):.6f}")
                    
            except Exception as e:
                print(f"[!] Ошибка в процессе решения: {e}")

        elif command == "log":
            results_key = f"{active_func}_{active_const}_{active_opt}"
            if results_key in results:
                Logger.print_log(functions[active_func], results[results_key], method_name=active_opt.upper())
            else:
                print("[!] Нет данных. Запустите 'solve' с текущими настройками.")

        elif command == "visualize":
            results_key = f"{active_func}_{active_const}_{active_opt}"
            if results_key not in results:
                print("[!] Нет данных. Запустите 'solve' с текущими настройками.")
                continue

            traj = results[results_key]
            func = functions[active_func]

            A_current, b_current, signs_current, free_current = None, None, None, None
            if active_const in constraints_db: 
                A_current, b_current, signs_current, free_current = constraints_db[active_const]

            print("[*] Подготовка графиков...")

            if len(func.vars) == 2:
                vis = InteractiveVisualizer2D(
                    func, traj, 
                    title=f"Метод: {active_opt.upper()}", 
                    epsilon=active_epsilon,
                    A=A_current, b=b_current, signs=signs_current, is_free=free_current
                )
                vis.show()
                
            elif len(func.vars) == 3:
                from analitics import InteractiveVisualizer3D
                vis = InteractiveVisualizer3D(
                    func, traj, 
                    title=f"Метод: {active_opt.upper()} (3D)", 
                    epsilon=active_epsilon,
                    A=A_current, b=b_current
                )
                vis.show()
            else:
                print(f"[!] Визуализация для {len(func.vars)} мерного пространства не поддерживается.")
                
        elif command == "analitics":
            try:
                functions[active_func].analyze_convexity_symbolic()
            except AttributeError:
                 print("[!] Внимание: Метод символьного анализа недоступен.")
        else:
            print("[?] Неизвестная команда. Введите 'help'.")

if __name__ == "__main__":
    main()