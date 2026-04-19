import numpy as np
import pandas as pd
from abc import ABC, abstractmethod
from typing import Optional, Union

class RiskEstimator(ABC):
    @abstractmethod
    def estimate(self, returns: pd.DataFrame) -> np.ndarray:
        pass


class SampleCovariance(RiskEstimator):
    def estimate(self, returns: pd.DataFrame) -> np.ndarray:
        covariance_matrix: np.ndarray = returns.cov().to_numpy() #Просто считаем ковариацию как обычно как разницу коэфа от среднего
        return covariance_matrix


class EWMACovariance(RiskEstimator):
    def __init__(self, span: int = 180):
        self.span: int = span
        
    def estimate(self, returns: pd.DataFrame) -> np.ndarray:
        num_assets: int = returns.shape[1]
        
        ewma_cov_df: pd.DataFrame = returns.ewm(span=self.span).cov().iloc[-num_assets:]
        
        covariance_matrix: np.ndarray = ewma_cov_df.to_numpy()
        return covariance_matrix

class ShrinkageCovariance(RiskEstimator):
    def __init__(self, shrinkage_intensity: float = 0.5):
        if not (0.0 <= shrinkage_intensity <= 1.0):
            raise ValueError("Интенсивность сжатия должна быть в диапазоне [0, 1]")
            
        self.shrinkage_intensity: float = shrinkage_intensity
        
    def estimate(self, returns: pd.DataFrame) -> np.ndarray:
        sample_cov: np.ndarray = returns.cov().to_numpy()
        
        prior_matrix: np.ndarray = np.diag(np.diag(sample_cov))
        
        shrunk_cov: np.ndarray = (
            self.shrinkage_intensity * prior_matrix + 
            (1.0 - self.shrinkage_intensity) * sample_cov
        )
        return shrunk_cov