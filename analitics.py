import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Button

class InteractiveVisualizer2D:
    def __init__(self, func, trajectory, title="Оптимизация", epsilon=None):
        self.func = func
        self.traj = trajectory
        self.title = title
        self.epsilon = epsilon
        self.current_step = 0
        self.max_steps = len(trajectory) - 1

        self.fig, self.ax = plt.subplots(figsize=(9, 7))
        plt.subplots_adjust(bottom=0.2)
        
        self._draw_contours()

        self.line, = self.ax.plot([], [], 'r-', lw=2, alpha=0.7)
        self.points, = self.ax.plot([], [], 'ro', markersize=4)
        self.current_point, = self.ax.plot([], [], 'y*', markersize=12, markeredgecolor='k')
        
        self.ax.plot(self.traj[0, 0], self.traj[0, 1], 'go', markersize=8, label='Старт')
        self.ax.plot(self.traj[-1, 0], self.traj[-1, 1], 'g*', markersize=12, label='Минимум')
        self.ax.legend()
        
        self._setup_buttons()
        self._update_plot()
        
    def _draw_contours(self):
        x_min, x_max = np.min(self.traj[:, 0]), np.max(self.traj[:, 0])
        y_min, y_max = np.min(self.traj[:, 1]), np.max(self.traj[:, 1])
        
        pad_x = max(abs(x_max - x_min) * 0.2, 1.0)
        pad_y = max(abs(y_max - y_min) * 0.2, 1.0)
        
        X = np.linspace(x_min - pad_x, x_max + pad_x, 100)
        Y = np.linspace(y_min - pad_y, y_max + pad_y, 100)
        X, Y = np.meshgrid(X, Y)
        
        Z = np.zeros_like(X)
        for i in range(X.shape[0]):
            for j in range(X.shape[1]):
                Z[i, j] = self.func(np.array([X[i, j], Y[i, j]]))
                
        contour = self.ax.contour(X, Y, Z, levels=50, cmap='viridis', alpha=0.6)
        self.fig.colorbar(contour, ax=self.ax, label='Значение функции')

    def _setup_buttons(self):
        ax_prev = plt.axes([0.3, 0.05, 0.15, 0.075])
        ax_next = plt.axes([0.55, 0.05, 0.15, 0.075])
        
        self.btn_prev = Button(ax_prev, 'Назад')
        self.btn_next = Button(ax_next, 'Вперед')
        
        self.btn_prev.on_clicked(self.prev_step)
        self.btn_next.on_clicked(self.next_step)

    def next_step(self, event):
        if self.current_step < self.max_steps:
            self.current_step += 1
            self._update_plot()

    def prev_step(self, event):
        if self.current_step > 0:
            self.current_step -= 1
            self._update_plot()

    def _update_plot(self):
        current_traj = self.traj[:self.current_step + 1]
        
        self.line.set_data(current_traj[:, 0], current_traj[:, 1])
        self.points.set_data(current_traj[:, 0], current_traj[:, 1])

        self.current_point.set_data([current_traj[-1, 0]], [current_traj[-1, 1]])

        val = self.func(current_traj[-1])
        grad_norm = np.linalg.norm(self.func.gradient(current_traj[-1]))
        
        eps_str = f" | ε = {self.epsilon:.1e}" if self.epsilon is not None else ""
        
        self.ax.set_title(f"{self.title}\nШаг {self.current_step}/{self.max_steps} | f(x) = {val:.4f} | ||∇f|| = {grad_norm:.2e}{eps_str}")
        self.fig.canvas.draw_idle()

    def show(self):
        plt.show()


class Logger:
    @staticmethod
    def print_log(func, trajectory, method_name=""):
        n_vars = len(trajectory[0])
        
        print(f"\n{'='*80}")
        print(f"ЛОГ ОПТИМИЗАЦИИ (n={n_vars}) | МЕТОД: {method_name}")
        print(f"{'='*80}")
        
        x_headers = " | ".join([f"x{j+1:<7}" for j in range(n_vars)])
        print(f"{'Итер':<5} | {x_headers} | {'f(x)':<12} | {'||∇f||':<12}")
        print("-" * 80)
        
        for i, x in enumerate(trajectory):
            val = func(x)
            grad_norm = np.linalg.norm(func.gradient(x))
            
            x_vals = " | ".join([f"{xi:>8.4f}" for xi in x])
    
            print(f"{i:<5} | {x_vals} | {val:>12.6f} | {grad_norm:>12.6e}")
            
        print(f"{'='*80}\n")