import numpy as np
from scipy.optimize import minimize
from abc import ABC, abstractmethod
from typing import Optional
from objective import PortfolioObjective

class QPSolver(ABC):
    
    @abstractmethod
    def solve(self, objective: 'PortfolioObjective', mu: np.ndarray, sigma: np.ndarray) -> np.ndarray:
        pass


class SciPyReferenceSolver(QPSolver):
    """
    Эталонный решатель на базе библиотеки SciPy.
    """
    def solve(self, objective: 'PortfolioObjective', mu: np.ndarray, sigma: np.ndarray) -> np.ndarray:
        num_assets = len(mu)
        x0 = np.ones(num_assets) / num_assets

        constraints = ({'type': 'eq', 'fun': lambda x: np.sum(x) - 1.0})

        bounds = tuple((0.0, 1.0) for _ in range(num_assets))
        
        def cost_function(x):
            return objective.evaluate(x, mu, sigma)
            
        def jacobian(x):
            return objective.gradient(x, mu, sigma)

        result = minimize(
            fun=cost_function,
            x0=x0,
            jac=jacobian,
            method='SLSQP',
            bounds=bounds,
            constraints=constraints,

            options={'ftol': 1e-7, 'disp': False, 'maxiter': 1000}
        )
        
        if not result.success:
            print(f"DEBUG - mu: {mu}")
            raise ValueError(f"Оптимизатор не сошелся: {result.message}")
            
        return result.x