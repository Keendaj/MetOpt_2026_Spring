import numpy as np
from abc import ABC, abstractmethod
from typing import List
from functions import ObjectiveFunction
from scipy.optimize import minimize


class BaseOptimizer(ABC):
    def __init__(self, epsilon: float = 1e-5, max_iter: int = 1000):
        self.epsilon = epsilon
        self.max_iter = max_iter

    @abstractmethod
    def optimize(self, func: ObjectiveFunction, x0: np.ndarray) -> np.ndarray:
        pass

    def _step_splitting(self, func: ObjectiveFunction, x: np.ndarray, direction: np.ndarray, 
                        alpha_init: float = 1.0, beta: float = 0.5, c1: float = 1e-4) -> float:
        alpha = alpha_init
        f_current = func(x)
        grad_current = func.gradient(x)
        directional_derivative = np.dot(grad_current, direction)

        while func(x + alpha * direction) > f_current + c1 * alpha * directional_derivative:
            alpha *= beta
            if alpha <= 1e-10:
                break
            
        return alpha

    def _get_exact_x_star(self, func: ObjectiveFunction, x0: np.ndarray) -> np.ndarray:
        """Вспомогательный метод для получения точного x* перед циклом (для логов)."""
        res = minimize(func, x0, method='BFGS', tol=1e-14)
        return res.x

    def _log_step(self, step: int, x: np.ndarray, x_star: np.ndarray, 
                  f_prev: float, f_current: float, grad: np.ndarray):
        """Метод для вывода логов на каждом шаге."""
        norm_diff = np.linalg.norm(x - x_star)
        norm_diff_sq = norm_diff ** 2
        delta_f = f_prev - f_current
        grad_norm = np.linalg.norm(grad)
        
        print(f"Шаг {step:<4} | "
              f"||x - x*||: {norm_diff:12.6e} | "
              f"||x - x*||^2: {norm_diff_sq:12.6e} | "
              f"f_prev - f_curr: {delta_f:12.6e} | "
              f"||grad||: {grad_norm:12.6e}")


class GradientDescent(BaseOptimizer):
    def optimize(self, func: ObjectiveFunction, x0: np.ndarray) -> np.ndarray:
        x = np.array(x0, dtype=float)
        trajectory = [x.copy()]
        
        x_star = self._get_exact_x_star(func, x0)
        f_prev = func(x)
        
        print("\n" + "-"*85)
        print(" ЛОГИРОВАНИЕ: Градиентный спуск")
        print("-" * 85)
        
        for i in range(self.max_iter):
            f_current = func(x)
            grad = func.gradient(x)

            self._log_step(i, x, x_star, f_prev, f_current, grad)
            f_prev = f_current

            if np.linalg.norm(grad) < self.epsilon:
                print(f"\n[+] Градиентный спуск сошелся за {i} итераций.")
                break
                
            direction = -grad
            alpha = self._step_splitting(func, x, direction)
            x = x + alpha * direction
            trajectory.append(x.copy())
            
        else:
            print(f"\n[-] Градиентный спуск не сошелся за {self.max_iter} итераций.")
            
        return np.array(trajectory)


class HookeJeeves(BaseOptimizer):
    def __init__(self, epsilon: float = 1e-5, max_iter: int = 1000, 
                 step_size: float = 0.5, step_reduce: float = 2.0):
        super().__init__(epsilon, max_iter)
        self.step_size = step_size
        self.step_reduce = step_reduce

    def _exploratory_search(self, func: ObjectiveFunction, base_point: np.ndarray, step: float) -> np.ndarray:
        x = base_point.copy()
        n = len(x)
        f_base = func(x)
        
        for i in range(n):
            x[i] += step
            f_new = func(x)
            
            if f_new < f_base:
                f_base = f_new
            else:
                x[i] -= 2 * step
                f_new = func(x)
                if f_new < f_base:
                    f_base = f_new 
                else:
                    x[i] += step
                    
        return x

    def optimize(self, func: ObjectiveFunction, x0: np.ndarray) -> np.ndarray:
        x_base = np.array(x0, dtype=float)
        trajectory = [x_base.copy()]
        step = self.step_size
        
        x_star = self._get_exact_x_star(func, x0)
        f_prev = func(x_base)
        
        print("\n" + "-"*85)
        print(" ЛОГИРОВАНИЕ: Метод Хука-Дживса")
        print("-" * 85)
        
        for i in range(self.max_iter):
            f_current = func(x_base)
            grad = func.gradient(x_base) 
            self._log_step(i, x_base, x_star, f_prev, f_current, grad)
            f_prev = f_current
            
            x_new = self._exploratory_search(func, x_base, step)
            
            if func(x_new) < func(x_base):
                direction = x_new - x_base
                x_pattern = x_new + direction
                
                x_base = x_new.copy()
                trajectory.append(x_base.copy())
                
                x_pattern_explored = self._exploratory_search(func, x_pattern, step)

                if func(x_pattern_explored) < func(x_base):
                    x_base = x_pattern_explored.copy()
                    trajectory.append(x_base.copy())
            else:
                step /= self.step_reduce
                if step < self.epsilon:
                    print(f"\n[+] Метод Хука-Дживса сошелся за {i} итераций.")
                    break
        else:
            print(f"\n[-] Метод Хука-Дживса не сошелся за {self.max_iter} итераций.")
            
        return np.array(trajectory)
    

class NewtonMethod(BaseOptimizer):
    def optimize(self, func: ObjectiveFunction, x0: np.ndarray) -> np.ndarray:
        x = np.array(x0, dtype=float)
        trajectory = [x.copy()]
        
        x_star = self._get_exact_x_star(func, x0)
        f_prev = func(x)
        
        print("\n" + "-"*85)
        print(" ЛОГИРОВАНИЕ: Метод Ньютона")
        print("-" * 85)
        
        for i in range(self.max_iter):
            f_current = func(x)
            grad = func.gradient(x)
            
            self._log_step(i, x, x_star, f_prev, f_current, grad)
            f_prev = f_current

            if np.linalg.norm(grad) < self.epsilon:
                print(f"\n[+] Метод Ньютона сошелся за {i} итераций.")
                break
                
            hessian = func.hessian(x)
            
            try:
                hessian_inv = np.linalg.inv(hessian)
            except np.linalg.LinAlgError:
                print(f"\nОшибка на итерации {i}: Гессиан вырожден (сингулярен), обращение невозможно.")
                break

            direction = -hessian_inv.dot(grad)
            
            if np.dot(grad, direction) > 0:
                print(f"Внимание на итерации {i}: Направление не является убывающим. Переход на антиградиент.")
                direction = -grad
            
            alpha = self._step_splitting(func, x, direction, alpha_init=1.0)
            x = x + alpha * direction
            trajectory.append(x.copy())
            
        else:
            print(f"\n[-] Метод Ньютона не сошелся за {self.max_iter} итераций.")
            
        return np.array(trajectory)
    

class DFPMethod(BaseOptimizer):
    def optimize(self, func: ObjectiveFunction, x0: np.ndarray) -> np.ndarray:
        x = np.array(x0, dtype=float)
        trajectory = [x.copy()]
        n = len(x)
        A = np.eye(n)
        
        x_star = self._get_exact_x_star(func, x0)
        grad_current = func.gradient(x)
        f_prev = func(x)
        
        print("\n" + "-"*85)
        print(" ЛОГИРОВАНИЕ: Метод ДФП")
        print("-" * 85)
        
        for i in range(self.max_iter):
            f_current = func(x)
            self._log_step(i, x, x_star, f_prev, f_current, grad_current)
            f_prev = f_current

            if np.linalg.norm(grad_current) < self.epsilon:
                print(f"\n[+] Метод ДФП сошелся за {i} итераций.")
                break
                
            direction = -A.dot(grad_current)
            
            if np.dot(grad_current, direction) > 0:
                A = np.eye(n)
                direction = -grad_current

            alpha = self._step_splitting(func, x, direction)
            
            delta_x = alpha * direction
            x_new = x + delta_x
            trajectory.append(x_new.copy())
            
            grad_new = func.gradient(x_new)
            delta_g = grad_new - grad_current
            
            dx_col = delta_x.reshape(-1, 1)
            dg_col = delta_g.reshape(-1, 1)
            
            den1 = np.dot(delta_x, delta_g)
            den2 = np.dot(delta_g, A.dot(delta_g))
            
            if abs(den1) > 1e-10 and abs(den2) > 1e-10:
                term1 = (dx_col @ dx_col.T) / den1
                Adg = A.dot(dg_col)
                term2 = (Adg @ Adg.T) / den2
                A = A + term1 - term2
            
            x = x_new
            grad_current = grad_new
            
        else:
            print(f"\n[-] Метод ДФП не сошелся за {self.max_iter} итераций.")
            
        return np.array(trajectory)


if __name__ == "__main__":
    class DummyFunc(ObjectiveFunction):
        def __call__(self, x): return x[0]**2 + 3*x[1]**2
        def gradient(self, x): return np.array([2*x[0], 6*x[1]])
        def hessian(self, x): return np.array([[2, 0], [0, 6]])
        
    func = DummyFunc()
    x0 = np.array([5.0, 5.0])

    optimizer = GradientDescent(epsilon=1e-5)
    traj = optimizer.optimize(func, x0)
    
    print(f"\nНачальная точка: {traj[0]}")
    print(f"Конечная точка: {traj[-1]}")
    print(f"Минимум функции: {func(traj[-1]):.6f}")