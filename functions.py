import numpy as np
import sympy as sp
from sympy.parsing.sympy_parser import parse_expr
from abc import ABC, abstractmethod

class ObjectiveFunction(ABC):
    def __init__(self):
        if self.vars is not None and self.expr is not None:
            self._compile_functions()

    def _compile_functions(self):
        grad_expr = [sp.diff(self.expr, var) for var in self.vars]
        
        hessian_expr = [[sp.diff(g, var) for var in self.vars] for g in grad_expr]
        
        self._f_lambda = sp.lambdify([self.vars], self.expr, "numpy")
        self._grad_lambda = sp.lambdify([self.vars], grad_expr, "numpy")
        self._hessian_lambda = sp.lambdify([self.vars], hessian_expr, "numpy")

    def __call__(self, x: np.ndarray) -> float:
        return float(self._f_lambda(x))

    def gradient(self, x: np.ndarray) -> np.ndarray:
        return np.array(self._grad_lambda(x), dtype=float)

    def hessian(self, x: np.ndarray) -> np.ndarray:
        return np.array(self._hessian_lambda(x), dtype=float)
    
    def analyze_convexity_symbolic(self):
        print(f"\n[{self.__class__.__name__}] Символьный анализ строгой выпуклости:")
        print(f"Функция f(x) = {self.expr}")
        
        grad_expr = [sp.diff(self.expr, var) for var in self.vars]
        hessian_expr = sp.Matrix([[sp.diff(g, var) for var in self.vars] for g in grad_expr])
        
        print("\nСимвольная матрица Гессе (Гессиан):")
        sp.pprint(hessian_expr)
        
        print("\nУгловые миноры (Критерий Сильвестра):")
        n = len(self.vars)
        for i in range(1, n + 1):
            minor = hessian_expr[:i, :i]
            det_minor = sp.simplify(minor.det())
            print(f"  Δ{i} = {det_minor}")
            
        print("\n-> Для доказательства строгой выпуклости убедитесь, что все Δi > 0 при любых x.\n")


class TestFunction2D(ObjectiveFunction):
    def __init__(self):
        self.vars = sp.symbols('x1 x2')
        x1, x2 = self.vars
        self.expr = x1**2 + 3 * x2**2 + sp.cos(x1 + x2)
        super().__init__()


class SynthFunction3D(ObjectiveFunction):
    def __init__(self):
        self.vars = sp.symbols('x1 x2 x3')
        x1, x2, x3 = self.vars
        self.expr = 2*x1**2 + 3*x2**2 + 4*x3**2 + x1*x2 + x1*x3 + x2*x3 + sp.exp(0.5*x1 - 0.3*x2 + 0.2*x3)
        super().__init__()

class CustomFunction(ObjectiveFunction):
    def __init__(self, expr_str: str, vars_str: str):
        self.vars = sp.symbols(vars_str)
        
        if not isinstance(self.vars, (list, tuple)):
            self.vars = [self.vars]
            
        local_dict = {symbol.name: symbol for symbol in self.vars}
        
        try:
            self.expr = parse_expr(expr_str, local_dict=local_dict)
        except Exception as e:
            raise ValueError(f"Ошибка при разборе функции: {e}")

        super().__init__()

class HardcoreFunction3D(ObjectiveFunction):
    def __init__(self):
        self.vars = sp.symbols('x1 x2 x3')
        x1, x2, x3 = self.vars
        self.expr = (sp.exp(0.5 * x1) + 
                     2 * sp.exp(0.2 * x2) + 
                     (x1 - x2 + x3)**4 + 
                     3 * x2**2 + 
                     x3**2 - 
                     x1 * x3 + 
                     10 * x1)
        super().__init__()