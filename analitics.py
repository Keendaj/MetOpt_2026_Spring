import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Button
import plotly.graph_objects as go
import itertools

class InteractiveVisualizer2D:
    def __init__(self, func, trajectory, title="Оптимизация", epsilon=None, A=None, b=None, signs=None, is_free=None):
        self.func = func
        self.traj = trajectory
        self.title = title
        self.epsilon = epsilon
        self.A = A
        self.b = b
        self.signs = signs if signs is not None else (['<='] * len(b) if b is not None else None)
        self.is_free = is_free if is_free is not None else ([False] * A.shape[1] if A is not None else None)
        self.current_step = 0
        self.max_steps = len(trajectory) - 1

        self.fig, self.ax = plt.subplots(figsize=(9, 7))
        plt.subplots_adjust(bottom=0.2)
        
        self._draw_contours_and_constraints()

        self.line, = self.ax.plot([], [], 'r-', lw=2, alpha=0.7)
        self.points, = self.ax.plot([], [], 'ro', markersize=4)
        self.current_point, = self.ax.plot([], [], 'y*', markersize=12, markeredgecolor='k')
        
        self.ax.plot(self.traj[0, 0], self.traj[0, 1], 'go', markersize=8, label='Старт')
        self.ax.plot(self.traj[-1, 0], self.traj[-1, 1], 'g*', markersize=12, label='Минимум')
        self.ax.legend(loc='upper right')
        
        self._setup_buttons()
        self._update_plot()
        
    def _draw_contours_and_constraints(self):
        x_min = min(0.0, np.min(self.traj[:, 0]))
        x_max = np.max(self.traj[:, 0])
        y_min = min(0.0, np.min(self.traj[:, 1]))
        y_max = np.max(self.traj[:, 1])
        
        pad_x = max(abs(x_max - x_min) * 0.5, 5.0)
        pad_y = max(abs(y_max - y_min) * 0.5, 5.0)
        
        if self.b is not None:
            max_b = np.max(self.b)
            pad_x = max(pad_x, max_b * 0.3)
            pad_y = max(pad_y, max_b * 0.3)
        
        x_bound = [x_min - pad_x, x_max + pad_x]
        y_bound = [y_min - pad_y, y_max + pad_y]
        
        X = np.linspace(x_bound[0], x_bound[1], 300)
        Y = np.linspace(y_bound[0], y_bound[1], 300)
        X, Y = np.meshgrid(X, Y)
        
        Z = np.zeros_like(X)
        for i in range(X.shape[0]):
            for j in range(X.shape[1]):
                Z[i, j] = self.func(np.array([X[i, j], Y[i, j]]))
                
        traj_vals = [self.func(pt) for pt in self.traj]
        step = max(1, len(traj_vals) // 20)
        levels_traj = np.sort(np.unique(traj_vals[::step]))
        
        bg_levels = np.linspace(np.min(Z), np.max(Z), 20)
        levels = np.sort(np.unique(np.concatenate([levels_traj, bg_levels])))
            
        contour = self.ax.contour(X, Y, Z, levels=levels, cmap='viridis', alpha=0.6)
        self.fig.colorbar(contour, ax=self.ax, label='Значение функции')

        if self.A is not None and self.b is not None:
            grid_points = np.c_[X.ravel(), Y.ravel()]
            
            mask_Ab = np.ones(grid_points.shape[0], dtype=bool)
            for i in range(len(self.b)):
                vals = grid_points @ self.A[i]
                if self.signs[i] == '<=':
                    mask_Ab &= (vals <= self.b[i] + 1e-6)
                elif self.signs[i] == '>=':
                    mask_Ab &= (vals >= self.b[i] - 1e-6)
                elif self.signs[i] == '==':
                    mask_Ab &= (np.abs(vals - self.b[i]) <= 1e-6)
                    
            mask_pos = np.ones(grid_points.shape[0], dtype=bool)
            for j in range(2):
                if not self.is_free[j]:
                    mask_pos &= (grid_points[:, j] >= -1e-6)
            
            invalid_mask = (~(mask_Ab & mask_pos)).astype(float).reshape(X.shape)
            
            if np.any(invalid_mask > 0):
                self.ax.contourf(X, Y, invalid_mask, levels=[0.5, 1.5], colors=['black'], alpha=0.3)
        
        self.ax.set_xlim(x_bound)
        self.ax.set_ylim(y_bound)

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

class InteractiveVisualizer3D:
    def __init__(self, func, trajectory, title="Оптимизация 3D", epsilon=None, A=None, b=None, signs=None, is_free=None):
        self.func = func
        self.traj = np.array(trajectory)
        self.title = title
        self.epsilon = epsilon
        self.A = A
        self.b = b
        self.signs = signs if signs is not None else (['<='] * len(b) if b is not None else None)
        self.is_free = is_free if is_free is not None else ([False] * (A.shape[1] if A is not None else 3))

    def _get_polyhedron_faces(self):
        """ Находит реальные грани многогранника для точной отрисовки """
        A_ext, b_ext = [], []
        # Сбор всех ограничений в формат A*x <= b
        for i in range(len(self.b)):
            if self.signs[i] == '>=':
                A_ext.append(-self.A[i]); b_ext.append(-self.b[i])
            elif self.signs[i] == '<=':
                A_ext.append(self.A[i]); b_ext.append(self.b[i])
        
        for j in range(3):
            if not self.is_free[j]:
                row = np.zeros(3); row[j] = -1.0
                A_ext.append(row); b_ext.append(0.0)

        # Ограничивающий бокс для визуализации (от -20 до 20)
        span = 20.0
        for j in range(3):
            row = np.zeros(3); row[j] = 1.0; A_ext.append(row); b_ext.append(span)
            row = np.zeros(3); row[j] = -1.0; A_ext.append(row); b_ext.append(span)

        A_ext, b_ext = np.array(A_ext), np.array(b_ext)
        faces_data = []

        # Для каждой плоскости находим все вершины, которые лежат на ней
        for i in range(len(A_ext)):
            plane_vertices = []
            # Ищем пересечения этой плоскости (i) с парами других (j, k)
            for j, k in itertools.combinations([idx for idx in range(len(A_ext)) if idx != i], 2):
                A_sub = A_ext[[i, j, k]]
                b_sub = b_ext[[i, j, k]]
                if np.abs(np.linalg.det(A_sub)) < 1e-6: continue
                try:
                    x = np.linalg.solve(A_sub, b_sub)
                    if np.all(A_ext @ x <= b_ext + 1e-4):
                        plane_vertices.append(x)
                except: continue
            
            if len(plane_vertices) >= 3:
                # Убираем дубликаты и сортируем точки по кругу, чтобы полигон не был "самопересекающимся"
                pts = np.unique(np.round(plane_vertices, 5), axis=0)
                if len(pts) < 3: continue
                center = np.mean(pts, axis=0)
                # Проекция на 2D для сортировки (выбираем оси в зависимости от нормали плоскости)
                normal = A_ext[i]
                ax_idx = np.argmin(np.abs(normal))
                pts_2d = np.delete(pts - center, ax_idx, axis=1)
                angles = np.arctan2(pts_2d[:, 1], pts_2d[:, 0])
                sorted_pts = pts[np.argsort(angles)]
                faces_data.append(sorted_pts)
        return faces_data

    def show(self):
        fig = go.Figure()
        
        # 1. ОТРИСОВКА ГРАНЕЙ (КАЖДАЯ ГРАНЬ - ОТДЕЛЬНЫЙ ПОЛИГОН)
        if self.A is not None:
            faces = self._get_polyhedron_faces()
            for i, face in enumerate(faces):
                fig.add_trace(go.Mesh3d(
                    x=face[:, 0], y=face[:, 1], z=face[:, 2],
                    color='deepskyblue', opacity=0.2, 
                    showlegend=(i == 0), name='Грани ограничений'
                ))
                # Рисуем четкие ребра граней
                f_edge = np.vstack([face, face[0]])
                fig.add_trace(go.Scatter3d(
                    x=f_edge[:, 0], y=f_edge[:, 1], z=f_edge[:, 2],
                    mode='lines', line=dict(color='blue', width=2), showlegend=False
                ))

        # 2. ТРАЕКТОРИЯ
        fig.add_trace(go.Scatter3d(
            x=self.traj[:, 0], y=self.traj[:, 1], z=self.traj[:, 2],
            mode='lines+markers', line=dict(color='red', width=6),
            marker=dict(size=4, color='darkred'), name='Траектория'
        ))

        # ТОЧКИ СТАРТА И МИНИМУМА
        fig.add_trace(go.Scatter3d(x=[self.traj[0,0]], y=[self.traj[0,1]], z=[self.traj[0,2]], 
                                   mode='markers', marker=dict(size=8, color='green'), name='Старт'))
        fig.add_trace(go.Scatter3d(x=[self.traj[-1,0]], y=[self.traj[-1,1]], z=[self.traj[-1,2]], 
                                   mode='markers', marker=dict(size=10, color='gold', symbol='diamond'), name='Минимум'))

        # 3. НАСТРОЙКИ ЗОНЫ [-20, 20]
        ax_lim = dict(range=[-20, 20], autorange=False, backgroundcolor="rgb(230, 230,230)", gridcolor="white")
        fig.update_layout(
            title=f"<b>3D Оптимизация</b><br>f_min = {self.func(self.traj[-1]):.4f}",
            scene=dict(xaxis=ax_lim, yaxis=ax_lim, zaxis=ax_lim, xaxis_title='X1', yaxis_title='X2', zaxis_title='X3'),
            margin=dict(l=0, r=0, b=0, t=50), legend=dict(x=0.1, y=0.9)
        )
        fig.show()

class Logger:
    @staticmethod
    def print_log(func, trajectory, method_name=""):
        n_vars = len(trajectory[0])
        
        x_headers = " | ".join([f"x{j+1:<8}" for j in range(n_vars)])
        grad_headers = " | ".join([f"∇f_{j+1:<7}" for j in range(n_vars)])
        
        header = f"{'Итер':<5} | {x_headers} | {'f(x)':<12} | {'||Δx||':<10} | {'|Δf|':<10} | {grad_headers} | {'||∇f||':<10}"
        line_len = len(header)
        
        print(f"\n{'='*line_len}")
        print(f"ЛОГ ОПТИМИЗАЦИИ (n={n_vars}) | МЕТОД: {method_name}")
        print(f"{'='*line_len}")
        print(header)
        print("-" * line_len)
        
        prev_x = None
        prev_f = None
        
        for i, x in enumerate(trajectory):
            val = func(x)
            grad = func.gradient(x)
            grad_norm = np.linalg.norm(grad)
            
            delta_x = np.linalg.norm(x - prev_x) if prev_x is not None else 0.0
            delta_f = abs(val - prev_f) if prev_f is not None else 0.0
            
            x_vals = " | ".join([f"{xi:>8.4f}" for xi in x])
            grad_vals = " | ".join([f"{gi:>8.4f}" for gi in grad])
            
            row = f"{i:<5} | {x_vals} | {val:>12.6f} | {delta_x:>10.6f} | {delta_f:>10.6f} | {grad_vals} | {grad_norm:>10.6e}"
            print(row)
            
            prev_x = x
            prev_f = val
            
        print(f"{'='*line_len}\n")