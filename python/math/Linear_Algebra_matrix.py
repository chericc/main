import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, Button

# 初始几何元素（一个三角形）
original_shape = np.array([[0, 0], [1, 0], [0.5, 1], [0, 0]])

# 创建图形和轴
fig, ax = plt.subplots(figsize=(10, 8))
plt.subplots_adjust(bottom=0.3)

# 绘制原始形状
original_line, = ax.plot(original_shape[:, 0], original_shape[:, 1], 'b-', label='Original')
transformed_line, = ax.plot(original_shape[:, 0], original_shape[:, 1], 'r-', label='Transformed')

# 设置坐标轴
ax.set_xlim(-5, 5)
ax.set_ylim(-5, 5)
ax.grid(True)
ax.axhline(0, color='black', linewidth=0.5)
ax.axvline(0, color='black', linewidth=0.5)
ax.set_aspect('equal')
ax.legend()

# 创建滑块控件
ax_scale = plt.axes([0.2, 0.2, 0.6, 0.03])
ax_rotate = plt.axes([0.2, 0.15, 0.6, 0.03])
ax_translate_x = plt.axes([0.2, 0.1, 0.6, 0.03])
ax_translate_y = plt.axes([0.2, 0.05, 0.6, 0.03])

scale_slider = Slider(ax_scale, 'Scale', 0.1, 3.0, valinit=1.0)
rotate_slider = Slider(ax_rotate, 'Rotate (deg)', 0, 360, valinit=0)
translate_x_slider = Slider(ax_translate_x, 'Translate X', -3, 3, valinit=0)
translate_y_slider = Slider(ax_translate_y, 'Translate Y', -3, 3, valinit=0)

# 更新函数
def update(val):
    # 获取当前滑块值
    scale = scale_slider.val
    angle = np.radians(rotate_slider.val)
    tx = translate_x_slider.val
    ty = translate_y_slider.val
    
    # 创建变换矩阵
    # 缩放矩阵
    scale_matrix = np.array([[scale, 0], [0, scale]])
    # 旋转矩阵
    rotation_matrix = np.array([
        [np.cos(angle), -np.sin(angle)],
        [np.sin(angle), np.cos(angle)]
    ])
    # 组合变换（先缩放，再旋转，最后平移）
    transformed = original_shape @ scale_matrix @ rotation_matrix + np.array([tx, ty])
    
    # 更新变换后的形状
    transformed_line.set_data(transformed[:, 0], transformed[:, 1])
    fig.canvas.draw_idle()

# 注册更新函数
scale_slider.on_changed(update)
rotate_slider.on_changed(update)
translate_x_slider.on_changed(update)
translate_y_slider.on_changed(update)

# 重置按钮
resetax = plt.axes([0.8, 0.025, 0.1, 0.04])
button = Button(resetax, 'Reset', color='lightgoldenrodyellow', hovercolor='0.975')

def reset(event):
    scale_slider.reset()
    rotate_slider.reset()
    translate_x_slider.reset()
    translate_y_slider.reset()
button.on_clicked(reset)

plt.show()
