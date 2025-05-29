import main_win

import logging
import sys
import main_win
import tkinter as tk
from myconfig import g_config
import myexception
from tkinter import messagebox

if __name__ == "__main__":

    try:

        log_file_handle = logging.FileHandler(g_config.log_file)
        log_stream_handler = logging.StreamHandler(sys.stdout)

        # log_file_handle.setLevel(g_config.map_log_level(g_config.log_level_file))
        # log_stream_handler.setLevel(g_config.map_log_level(g_config.log_level_console))
        log_file_handle.setLevel(logging.DEBUG)
        log_stream_handler.setLevel(logging.DEBUG)

        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s',
            handlers=[
                log_file_handle,
                log_stream_handler
            ]
        )

        logging.info("Starting")
        root = tk.Tk()
        root.geometry("800x600")
        app = main_win.StatusMonitorApp(root)
        root.protocol("WM_DELETE_WINDOW", app.on_closing)
        root.mainloop()
    except myexception.MyException as e:
        messagebox.showinfo('Warning', e)
        sys.exit(1)
    except Exception as e:
        messagebox.showinfo('Error', e)
        sys.exit(1)