import numpy as np
from abc import ABC, abstractmethod
from typing import List
from functions import ObjectiveFunction
from scipy.optimize import minimize
from simplex import LPProblem

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
        res = minimize(func, x0, method='BFGS', tol=1e-14)
        return res.x

    def _log_step(self, step: int, x: np.ndarray, x_star: np.ndarray,
                   f_prev: float, f_current: float, grad: np.ndarray):
        norm_diff = np.linalg.norm(x - x_star)
        norm_diff_sq = norm_diff ** 2
        delta_f = f_prev - f_current
        grad_norm = np.linalg.norm(grad)
        
        print(f"  {step:<4} | "
              f"||x - x*||: {norm_diff:12.6e} | "
              f"||x - x*||^2: {norm_diff_sq:12.6e} | "
              f"f_prev - f_curr: {delta_f:12.6e} | "
              f"||grad||: {grad_norm:12.6e}")

class ConditionalGradient(BaseOptimizer):
    def __init__(self, A: np.ndarray, b: np.ndarray, signs: List[str] = None, is_free: List[bool] = None, eps: float = 1e-5, max_iter: int = 1000, 
                 alpha_0: float = 1.0, lam: float = 0.5):
        super().__init__(epsilon=eps, max_iter=max_iter) 
        self.A = np.array(A, dtype=float)
        self.b = np.array(b, dtype=float)
        self.signs = signs if signs is not None else ['<='] * len(self.b)
        self.is_free = is_free if is_free is not None else [False] * self.A.shape[1]
        self.alpha_0 = alpha_0  
        self.lam = lam

    def optimize(self, func: ObjectiveFunction, x0: np.ndarray, verbose: bool = False) -> np.ndarray:
        x_k = np.array(x0, dtype=float)
        trajectory = [x_k.copy()]
        
        try:
            x_star = self._get_exact_x_star(func, x0)
        except Exception:
            x_star = np.zeros_like(x0)
            
        phi_prev = func(x_k)
        
        if verbose:
            print("\n" + "="*85)
            print(f" ЗАПУСК: Метод условного градиента")
            print(f" Параметры: alpha_0={self.alpha_0}, lambda={self.lam}, eps={self.epsilon}")
            print("=" * 85)
        
        for k in range(self.max_iter):
            phi_x_k = func(x_k)
            grad_phi = func.gradient(x_k)
            
            if verbose:
                print(f"\n[Итерация {k}] Текущая точка x = {x_k}")
                print(f"              Градиент ∇f(x) = {grad_phi}")
            
            lp = LPProblem(c=grad_phi, A=self.A, b=self.b, signs=self.signs, is_free=self.is_free, is_min=True, verbose=verbose)
            y_k = lp.solve()
            
            if y_k is None:
                if verbose:
                    print(f"[-] ОШИБКА на шаге {k}: Вспомогательная задача ЛП не имеет решения.")
                break

            s_k = y_k - x_k
            eta_k = np.dot(grad_phi, s_k)
            
            if verbose:
                print(f"  --> Направление спуска s = y* - x = {s_k}")
                print(f"  --> Зазор эта (∇f * s) = {eta_k:.6e}")
            
            if eta_k >= -self.epsilon:
                if verbose:
                    print(f"\n[+] Оптимум достигнут на шаге {k}. (Зазор {eta_k:.6e} удовлетворяет eps {self.epsilon})")
                break
                
            alpha = self.alpha_0
            
            while func(x_k + alpha * s_k) - phi_x_k > alpha * eta_k / 2:
                alpha *= self.lam 
                if alpha < 1e-18:
                    break

            if verbose:
                print(f"  --> Длина шага alpha = {alpha:.6f}")

            x_k = x_k + alpha * s_k
            trajectory.append(x_k.copy())
            
        else:
            if verbose:
                print(f"\n[-] Достигнуто максимальное число итераций ({self.max_iter}).")
            
        return np.array(trajectory)