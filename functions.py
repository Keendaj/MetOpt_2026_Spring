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

    def is_strictly_convex(self, x: np.ndarray) -> bool:
        H = self.hessian(x)
        n = H.shape[0]
        for i in range(1, n + 1):
            minor = H[:i, :i]
            if np.linalg.det(minor) <= 0:
                return False
        return True


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
        self.expr = x1**2 + 2*x2**2 + 3*x3**2 + x1*x2 + x2*x3 + x1*x3 + sp.exp(x1) + sp.exp(x2) + sp.exp(x3)
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

if __name__ == "__main__":
    f2d = TestFunction2D()
    f3d = SynthFunction3D()
    
    x0_2d = np.array([1.0, 1.0])
    x0_3d = np.array([1.0, 1.0, 1.0])
    
    print("TestFunction2D:")
    print(f"  Значение в {x0_2d}: {f2d(x0_2d)}")
    print(f"  Градиент в {x0_2d}: {f2d.gradient(x0_2d)}")
    print(f"  Гессиан в {x0_2d}:\n{f2d.hessian(x0_2d)}")
    print(f"  Строго выпукла: {f2d.is_strictly_convex(x0_2d)}\n")
    
    print("SynthFunction3D:")
    print(f"  Значение в {x0_3d}: {f3d(x0_3d)}")
    print(f"  Градиент в {x0_3d}: {f3d.gradient(x0_3d)}")
    print(f"  Гессиан в {x0_3d}:\n{f3d.hessian(x0_3d)}")
    print(f"  Строго выпукла: {f3d.is_strictly_convex(x0_3d)}")