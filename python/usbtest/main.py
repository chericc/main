import main_win

import logging
import sys
import main_win
import tkinter as tk

if __name__ == "__main__":
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(levelname)s - %(message)s',
        handlers=[
            logging.FileHandler('app.log'),  # 写入文件
            logging.StreamHandler(sys.stdout)  # 输出到当前终端
        ]
    )

    # set log level here ?

    logging.info("Starting")
    root = tk.Tk()
    root.geometry("800x600")
    app = main_win.StatusMonitorApp(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()