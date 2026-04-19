import numpy as np
from abc import ABC, abstractmethod
from typing import Tuple

class PortfolioObjective(ABC):
    @abstractmethod
    def evaluate(self, weights: np.ndarray, mu: np.ndarray, sigma: np.ndarray) -> float:
        pass

    @abstractmethod
    def gradient(self, weights: np.ndarray, mu: np.ndarray, sigma: np.ndarray) -> np.ndarray:
        pass
        
    @abstractmethod
    def get_qp_matrices(self, mu: np.ndarray, sigma: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
        pass

class MinVarianceObjective(PortfolioObjective):
    def evaluate(self, weights: np.ndarray, mu: np.ndarray, sigma: np.ndarray) -> float:
        return 0.5 * weights.T @ sigma @ weights

    def gradient(self, weights: np.ndarray, mu: np.ndarray, sigma: np.ndarray) -> np.ndarray:
        return sigma @ weights

    def get_qp_matrices(self, mu: np.ndarray, sigma: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
        P = sigma
        q = np.zeros_like(mu)
        return P, q
    
class MaxUtilityObjective(PortfolioObjective):
    def __init__(self, risk_aversion: float = 1.0):
        if risk_aversion < 0:
            raise ValueError("Коэффициент неприятия риска (lambda) должен быть >= 0")
        self.lmbda: float = risk_aversion

    def evaluate(self, weights: np.ndarray, mu: np.ndarray, sigma: np.ndarray) -> float:
        risk_penalty = (self.lmbda / 2.0) * (weights.T @ sigma @ weights)
        expected_return = weights.T @ mu
        return risk_penalty - expected_return

    def gradient(self, weights: np.ndarray, mu: np.ndarray, sigma: np.ndarray) -> np.ndarray:
        return self.lmbda * (sigma @ weights) - mu

    def get_qp_matrices(self, mu: np.ndarray, sigma: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
        P = self.lmbda * sigma
        q = -mu
        return P, q
    
class MaxSharpeObjective(PortfolioObjective):
    def __init__(self, risk_free_rate: float = 0.0):
        self.rf: float = risk_free_rate

    def evaluate(self, weights: np.ndarray, mu: np.ndarray, sigma: np.ndarray) -> float:
        expected_return: float = weights.T @ mu - self.rf
        portfolio_volatility: float = np.sqrt(weights.T @ sigma @ weights)
        
        if portfolio_volatility < 1e-8:
            return 0.0
            
        sharpe_ratio: float = expected_return / portfolio_volatility
        return -sharpe_ratio

    def gradient(self, weights: np.ndarray, mu: np.ndarray, sigma: np.ndarray) -> np.ndarray:
        expected_return: float = weights.T @ mu - self.rf
        portfolio_volatility: float = np.sqrt(weights.T @ sigma @ weights)
        
        if portfolio_volatility < 1e-8:
            return np.zeros_like(weights)
        grad_return: np.ndarray = mu
        grad_vol: np.ndarray = (sigma @ weights) / portfolio_volatility
        
        grad_sharpe: np.ndarray = (grad_return * portfolio_volatility - expected_return * grad_vol) / (portfolio_volatility ** 2)

        return -grad_sharpe

    def get_qp_matrices(self, mu: np.ndarray, sigma: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
        P: np.ndarray = sigma
        q: np.ndarray = np.zeros_like(mu)
        return P, q