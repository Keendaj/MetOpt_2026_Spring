import sys
import numpy as np
from functions import TestFunction2D, SynthFunction3D
from optimizers import GradientDescent, HookeJeeves, NewtonMethod, DFPMethod
from analitics import InteractiveVisualizer2D, Logger

def print_help():
    print("\n--- Доступные команды ---")
    print("help             - Показать это меню")
    print("pick_func [type] - Выбрать функцию: '2d' (тестовая) или '3d' (синтезируемая)")
    print("pick_opt [type]  - Выбрать метод: 'grad', 'hooke', 'newton', 'dfp'")
    print("set_start [x...] - Задать начальную точку (например: set_start -2.0 2.0)")
    print("set_epsilon [v]  - Задать точность epsilon (например: set_epsilon 1e-6)")
    print("solve            - Решить текущую задачу выбранным методом")
    print("log              - Вывести численный лог (траекторию спуска) для текущей задачи")
    print("visualize        - Открыть графики (только для 2d задачи)")
    print("analitics        - Вывод аналитических представлений Гессиана и угловых миноров для критерия Сильвестра")
    print("exit             - Выход")
    print("-------------------------\n")

def main():
    functions = {
        "2d": TestFunction2D(),
        "3d": SynthFunction3D()
    }
    
    start_points = {
        "2d": np.array([3.0, -8.0]),
        "3d": np.array([-1.0, 1.0, -1.0])
    }
    
    active_epsilon = 1e-5
    optimizers = {
        "grad": GradientDescent(epsilon=active_epsilon),
        "hooke": HookeJeeves(epsilon=active_epsilon),
        "newton": NewtonMethod(epsilon=active_epsilon),
        "dfp": DFPMethod(epsilon=active_epsilon)
    }

    active_func = "2d"
    active_opt = "newton"

    results = {}

    print("Программа минимизации функций запущена.")
    print_help()

    while True:
        prompt = f"\n[{active_func} | {active_opt} | x0={start_points[active_func]} | eps={active_epsilon}] > "
        try:
            line = input(prompt).strip()
        except EOFError:
            break

        if (not(line)):
            continue

        parts = line.split()
        command = parts[0].lower()

        if command == "exit":
            break

        elif command == "help":
            print_help()

        elif command == "pick_func":
            if len(parts) > 1:
                func_type = parts[1].lower()
                if func_type in functions:
                    active_func = func_type
                    print(f"[*] Активная функция: {active_func}")
                else:
                    print(f"[!] Ошибка: функции '{func_type}' нет. Доступны: 2d, 3d.")
            else:
                print("[!] Ошибка: укажите тип (pick_func 2d).")

        elif command == "pick_opt":
            if len(parts) > 1:
                opt_type = parts[1].lower()
                if opt_type in optimizers:
                    active_opt = opt_type
                    print(f"[*] Активный метод: {active_opt}")
                else:
                    print(f"[!] Ошибка: метода '{opt_type}' нет. Доступны: grad, hooke, newton, dfp.")
            else:
                print("[!] Ошибка: укажите метод (pick_opt newton).")

        elif command == "set_start":
            if len(parts) > 1:
                try:
                    coords = [float(x) for x in parts[1:]]
                    
                    expected_dim = len(start_points[active_func])
                    if len(coords) != expected_dim:
                        print(f"[!] Ошибка: Для '{active_func}' ожидается {expected_dim} координат, получено {len(coords)}.")
                        continue
                        
                    start_points[active_func] = np.array(coords)
                    print(f"[+] Начальная точка обновлена: {start_points[active_func]}")
                except ValueError:
                    print("[!] Ошибка: Координаты должны быть числами.")
            else:
                print("[!] Ошибка: укажите координаты (например, set_start 1.5 -2.0).")

        elif command == "set_epsilon":
            if len(parts) > 1:
                try:
                    new_eps = float(parts[1])
                    if new_eps <= 0:
                        print("[!] Ошибка: epsilon должен быть строго больше нуля.")
                    else:
                        active_epsilon = new_eps
                        for opt in optimizers.values():
                            opt.epsilon = active_epsilon
                        print(f"[+] Точность epsilon успешно обновлена: {active_epsilon}")
                except ValueError:
                    print("[!] Ошибка: Значение должно быть числом (например, set_epsilon 1e-6).")
            else:
                print("[!] Ошибка: укажите значение (например, set_epsilon 1e-6).")

        elif command == "solve":
            print(f"\n[*] Подготовка задачи '{active_func}'...")
            func = functions[active_func]
            opt = optimizers[active_opt]
            x0 = start_points[active_func]
            
            print("--- Аналитическая проверка ---")
            try:
                func.analyze_convexity_symbolic()
            except AttributeError:
                print("[!] Внимание: Метод analyze_convexity_symbolic не найден. Возможно, вы забыли добавить его в functions.py")
            print("------------------------------")
            is_continue = input("Введите Да, чтобы продолжить, если условия выполняются, любой другой ввод прекратит работу:\n")
            if is_continue != "Да":
                print("Подсчёт закончен, функция не подходит аналитически")
                continue

            print(f"\n[*] Запуск метода '{active_opt}' из точки {x0} (epsilon = {active_epsilon})...")
            try:
                trajectory = opt.optimize(func, x0)
                results_key = f"{active_func}_{active_opt}"
                results[results_key] = trajectory
                print(f"[+] Решение завершено! Найдена точка: {trajectory[-1]}")
                print(f"    Значение функции в минимуме: {func(trajectory[-1]):.6f}")
                print("    Используй 'log' для просмотра шагов или 'visualize' (для 2d).")
            except Exception as e:
                print(f"[!] Ошибка в процессе решения: {e}")

        elif command == "log":
            results_key = f"{active_func}_{active_opt}"
            if results_key in results:
                func = functions[active_func]
                traj = results[results_key]
                Logger.print_log(func, traj, method_name=active_opt.upper())
            else:
                print("[!] Нет данных. Сначала запустите 'solve'.")

        elif command == "visualize":
            if active_func != "2d":
                print("[!] Визуализация доступна только для функции 2d.")
                continue
                
            results_key = f"2d_{active_opt}"
            if results_key in results:
                print("[*] Подготовка графиков...")
                func = functions["2d"]
                traj = results[results_key]
                print(traj)
                vis = InteractiveVisualizer2D(func, traj, title=f"Метод: {active_opt.upper()}", epsilon=active_epsilon)
                vis.show()
            else:
                print("[!] Нет данных. Сначала запустите 'solve' для 2d задачи.")
                
        elif command == "analitics":
            func = functions[active_func]
            print("--- Аналитическая проверка ---")
            try:
                func.analyze_convexity_symbolic()
            except AttributeError:
                 print("[!] Внимание: Метод analyze_convexity_symbolic не найден в классе функции.")
            print("------------------------------")
        else:
            print("[?] Неизвестная команда. Введите 'help'.")

if __name__ == "__main__":
    main()