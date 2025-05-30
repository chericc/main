"""Main window"""

import sys
import logging
import threading
from queue import Queue
import tkinter as tk
from tkinter import ttk

import status_checker
import myexception
from myconfig import g_config

class StatusMonitorApp:
    def __init__(self, root):
        self.root = root
        self.root.title("升级程序")
        self.buttons = []
        self.status_queue = Queue()
        self.running = True
        self.status_checker = status_checker.StatusChecker(g_config.com_port)
        self.cond_wait = threading.Condition()

        self.color_font_bright = 'white'
        self.color_font_dark = 'black'

        self.color_normal = 'Silver'
        self.color_font_normal = self.color_font_dark
        self.color_inserted = 'white'
        self.color_font_inserted = self.color_font_dark
        self.color_active = 'blue'
        self.color_font_active = self.color_font_bright
        self.color_ok = 'green'
        self.color_font_ok = self.color_font_bright
        self.color_error = 'red'
        self.color_font_error = self.color_font_bright

        self.error_in_other_thread: bool = False

        # 创建UI
        self.setup_ui()

        # 启动状态监控线程
        self.start_monitor_thread()

        # 启动UI更新循环
        self.update_ui()


    def setup_ui(self):
        # 主框架
        main_frame = ttk.Frame(self.root)
        main_frame.pack(expand=True, fill='both', padx=10, pady=10)

        # 按钮网格框架
        grid_frame = ttk.Frame(main_frame)
        grid_frame.pack(expand=True, fill='both')

        # 创建10个按钮
        for i in range(10):
            row = i % 10
            col = i // 10
            grid_frame.columnconfigure(col, weight=1)
            grid_frame.rowconfigure(row, weight=1)

            btn = tk.Button(
                grid_frame,
                text=f"端口{i + 1}",
                font=('Microsoft YaHei', 12),
                relief='raised',
                width=2,
                height=1,
                bg = self.color_normal,
                fg = self.color_font_normal,
            )
            btn.grid(row=row, column=col, sticky="nsew", padx=5, pady=5)
            self.buttons.append(btn)

        # 控制面板
        control_frame = ttk.Frame(main_frame)
        control_frame.pack(fill='x', pady=10)

        self.status_label = tk.Label(control_frame, text="状态: 运行中", fg="green")
        self.status_label.pack(side='left', padx=10)

        tk.Button(control_frame, text="停止监控", command=self.stop_monitoring).pack(side='right', padx=10)
        tk.Button(control_frame, text="手动刷新", command=self.force_check).pack(side='right', padx=10)

    def start_monitor_thread(self):
        """启动状态监控线程"""
        self.monitor_thread = threading.Thread(
            target=self.monitor_status,
            daemon=True
        )
        self.monitor_thread.start()

    def status_check(self):
        self.status_checker.run()
        ports_state = self.status_checker.get_port_state()
        self.status_queue.put(ports_state)

    def monitor_status(self):
        """模拟定期检查外部状态的线程"""
        try:

            while self.running:
                # 这里模拟从外部系统获取状态
                # 实际应用中可能是API调用、数据库查询等
                # time.sleep(2)  # 每2秒检查一次
                logging.debug('before wait')
                self.cond_wait.acquire()
                self.cond_wait.wait(g_config.update_interval_sec)
                self.cond_wait.release()
                logging.debug('after wait')

                self.status_check()
        except Exception as e:
            logging.error(f'error in programe{e}')
            self.error_in_other_thread = True
            

    def update_ui(self):
        """更新UI的主循环"""
        try:
            if self.error_in_other_thread:
                logging.error('error in other thread, terminate')
                raise myexception.MyException('error in other thread')

            # 检查队列中是否有新状态
            while not self.status_queue.empty():
                status_list = self.status_queue.get_nowait()
                self.apply_status(status_list)
        except Exception as e:
            logging.error(f'exit: {e}')
            sys.exit(1)

        # 继续定期检查
        if self.running:
            self.root.after(500, self.update_ui)  # 每500ms检查一次更新

    def apply_status(self, status_list: list[status_checker.PortState]):
        """根据状态数据更新按钮颜色"""
        color_map = {
            0: "green",  # 正常
            1: "orange",  # 警告
            2: "red"  # 错误
        }

        for i, status in enumerate(status_list):
            if i < len(self.buttons):
                bg_color = self.color_normal
                fg_color = self.color_font_normal
                text = f'端口{i + 1}'
                port_info = status.get_port_info()
                if status.get_port_state() == status_checker.PortStateType.Init:
                    bg_color = self.color_normal
                    fg_color = self.color_font_normal
                    text += '/初始化'
                elif status.get_port_state() == status_checker.PortStateType.WaitInsert:
                    bg_color = self.color_normal
                    fg_color = self.color_font_normal
                    text += '/等待设备插入'
                elif status.get_port_state() == status_checker.PortStateType.WaitVolumeMount:
                    bg_color = self.color_inserted
                    fg_color = self.color_font_inserted
                    text += '/等待设备挂载'
                elif status.get_port_state() == status_checker.PortStateType.UpgradePrepare:
                    bg_color = self.color_active
                    fg_color = self.color_font_active
                    text += '/升级准备'
                elif status.get_port_state() == status_checker.PortStateType.Upgrading:
                    bg_color = self.color_active
                    fg_color = self.color_font_active
                    text += '/升级中...'
                elif status.get_port_state() == status_checker.PortStateType.Upgraded:
                    bg_color = self.color_ok
                    fg_color = self.color_font_ok
                    text += '/已是最新版本'
                elif status.get_port_state() == status_checker.PortStateType.Error:
                    bg_color = self.color_error
                    fg_color = self.color_font_error
                    text += '/错误'
                else:
                    bg_color = self.color_error
                    fg_color = self.color_font_error
                    text += '/未知错误'
                if len(port_info) > 0:
                    text += f'/{port_info}'

                self.buttons[i].config(
                    bg=bg_color,
                    fg=fg_color,
                    text=text
                )

    def force_check(self):
        """手动触发状态检查"""
        # 清空队列以避免处理旧数据
        while not self.status_queue.empty():
            self.status_queue.get_nowait()

        logging.debug('force check in')
        self.cond_wait.acquire()
        self.cond_wait.notify()
        self.cond_wait.release()
        logging.debug('force check out')
        pass

    def stop_monitoring(self):
        """停止监控线程"""
        # self.running = False
        # self.status_label.config(text="状态: 已停止", fg="red")
        pass

    def on_closing(self):
        """窗口关闭时的清理工作"""
        logging.info('closed')
        self.running = False
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    root.geometry("800x600")
    app = StatusMonitorApp(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()
