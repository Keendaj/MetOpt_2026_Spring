import numpy as np
import pandas as pd
import yfinance as yf
import matplotlib.pyplot as plt
from tabulate import tabulate
from objective import *
from estimators import *
from solvers import *

class PortfolioEngine:
    def __init__(self, estimator: 'RiskEstimator', objective: 'PortfolioObjective', solver: 'QPSolver'):
        self.estimator = estimator
        self.objective = objective
        self.solver = solver
        
    def fit(self, train_returns: pd.DataFrame) -> np.ndarray:
        mu: np.ndarray = train_returns.mean().to_numpy()
        sigma: np.ndarray = self.estimator.estimate(train_returns)

        weights = self.solver.solve(self.objective, mu, sigma)
        return weights

    def backtest(self, train_returns: pd.DataFrame, test_returns: pd.DataFrame) -> dict:
        """Проверка портфеля на будущих (Out-of-Sample) данных."""
        # Получаем веса на основе прошлого (солвер обучается на масштабированных данных * 100)
        weights = self.fit(train_returns)
        
        # ВАЖНО: Возвращаем тестовые данные к долям (от 5.0 обратно к 0.05)
        # Это необходимо для корректного расчета сложного процента
        true_test_returns = test_returns / 100.0 
        
        # Умножаем веса на реальные доходности
        portfolio_returns = true_test_returns.dot(weights)
        
        # Считаем итоговые метрики на тестовом периоде (геометрическая доходность)
        total_return = (1 + portfolio_returns).prod() - 1
        
        # Годовые метрики (252 торговых дня)
        annualized_return = portfolio_returns.mean() * 252 
        annualized_volatility = portfolio_returns.std() * np.sqrt(252)
        
        # Доходность бенчмарка (поровну во все активы)
        equal_weights = np.ones(len(weights)) / len(weights)
        benchmark_returns = true_test_returns.dot(equal_weights)
        benchmark_total_return = (1 + benchmark_returns).prod() - 1
        
        return {
            "weights": weights,
            "portfolio_returns": portfolio_returns,
            "total_return": total_return,
            "annualized_return": annualized_return,
            "annualized_vol": annualized_volatility,
            "benchmark_return": benchmark_total_return
        }


if __name__ == "__main__":
    tickers = [
        "AAPL", "MSFT", "GOOGL", "AMZN", "META", 
        "NVDA", "AMD", "INTC", "TSM", "QCOM", "AVGO", "TXN", "ASML", "CSCO",
        "EA", "SONY", "NFLX", "DIS", "TTWO",
        "ADBE", "CRM", "ORCL", "IBM", "INTU", "V", "MA"
    ]
    
    print(f"Загрузка данных для {len(tickers)} компаний...")
    
    data = yf.download(tickers, start="2020-01-01", end="2024-01-01")
    
    try:
        prices = data['Adj Close']
    except KeyError:
        try:
            prices = data['Close']
        except KeyError:
            prices = data.xs('Close', level=1, axis=1)
            
    threshold = int(prices.shape[0] * 0.9)
    prices = prices.dropna(axis=1, thresh=threshold)
    
    dropped_tickers = set(tickers) - set(prices.columns)
    if dropped_tickers:
        print(f"Внимание! Исключены из-за нехватки данных: {dropped_tickers}")
    
    returns = prices.pct_change(fill_method=None) * 100.0
    
    returns = returns.dropna()

    if returns.empty:
        raise ValueError("Данные пусты! Проверьте список тикеров.")

    train_returns = returns.loc["2020-01-01":"2022-12-31"]
    test_returns = returns.loc["2023-01-01":"2023-12-31"]
    
    print(f"Размер Train: {train_returns.shape[0]} дней, Активов: {train_returns.shape[1]}")
    print(f"Размер Test: {test_returns.shape[0]} дней, Активов: {test_returns.shape[1]}")
    
    import itertools

    print("\nНачинаем масштабный стресс-тест (Grid Search)...")
    
    estimators = {
        "Sample (Сырая)": SampleCovariance(),
        "Shrinkage (d=0.2)": ShrinkageCovariance(shrinkage_intensity=0.2),
        "Shrinkage (d=0.5)": ShrinkageCovariance(shrinkage_intensity=0.5),
        "Shrinkage (d=0.8)": ShrinkageCovariance(shrinkage_intensity=0.8),
        "EWMA (span=60)": EWMACovariance(span=60),
        "EWMA (span=120)": EWMACovariance(span=120),
        "EWMA (span=180)": EWMACovariance(span=180),
        "EWMA (span=240)": EWMACovariance(span=240)
    }
    
    objectives = {
        "Min Variance": MinVarianceObjective(),
        "Max Sharpe": MaxSharpeObjective(risk_free_rate=0.0),
        "Max Utility (L=2)": MaxUtilityObjective(risk_aversion=2.0),
        "Max Utility (L=10)": MaxUtilityObjective(risk_aversion=10.0),
        "Max Utility (L=100)": MaxUtilityObjective(risk_aversion=100.0)
    }
    
    solver = SciPyReferenceSolver()
    
    experiment_results = []
    
    for est_name, est_obj in estimators.items():
        for obj_name, obj_func in objectives.items():
            
            engine = PortfolioEngine(est_obj, obj_func, solver)
            
            try:
                res = engine.backtest(train_returns, test_returns)
                oos_sharpe = res['annualized_return'] / res['annualized_vol'] if res['annualized_vol'] > 0 else 0
                
                experiment_results.append({
                    "Матрица рисков": est_name,
                    "Целевая функция": obj_name,
                    "Доходность (OOS)": res['total_return'] * 100,
                    "Бенчмарк (1/N)": res['benchmark_return'] * 100,
                    "Волатильность": res['annualized_vol'] * 100,
                    "Шарп (OOS)": oos_sharpe,
                    "Статус": "OK"
                })
            except Exception as e:
                experiment_results.append({
                    "Матрица рисков": est_name,
                    "Целевая функция": obj_name,
                    "Доходность (OOS)": None,
                    "Бенчмарк (1/N)": None,
                    "Волатильность": None,
                    "Шарп (OOS)": None,
                    "Статус": "Ошибка солвера"
                })

    df_results = pd.DataFrame(experiment_results)
    
    df_results = df_results.sort_values(by="Шарп (OOS)", ascending=False).reset_index(drop=True)

    df_results["Доходность (OOS)"] = df_results["Доходность (OOS)"].apply(lambda x: f"{x:.2f}%" if pd.notnull(x) else "Ошибка")
    df_results["Бенчмарк (1/N)"] = df_results["Бенчмарк (1/N)"].apply(lambda x: f"{x:.2f}%" if pd.notnull(x) else "Ошибка")
    df_results["Волатильность"] = df_results["Волатильность"].apply(lambda x: f"{x:.2f}%" if pd.notnull(x) else "Ошибка")
    df_results["Шарп (OOS)"] = df_results["Шарп (OOS)"].apply(lambda x: f"{x:.3f}" if pd.notnull(x) else "Ошибка")

    print("\n" + "="*95)
    print(" ИТОГИ ЭКСПЕРИМЕНТА (OUT-OF-SAMPLE БЭКТЕСТ ЗА 2023 ГОД) ".center(95))
    print("="*95)
    
    print(tabulate(df_results, headers='keys', tablefmt='psql', showindex=False))
    print("="*95 + "\n")