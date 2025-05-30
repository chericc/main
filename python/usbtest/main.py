""" Module main """

import logging
import sys
import tkinter as tk
from tkinter import messagebox

import main_win
from myconfig import g_config
import myexception

if __name__ == "__main__":

    try:
        log_file_handle = logging.FileHandler(g_config.log_file)
        log_stream_handler = logging.StreamHandler(sys.stdout)

        log_file_handle.setLevel(g_config.map_log_level(g_config.log_level_file))
        log_stream_handler.setLevel(g_config.map_log_level(g_config.log_level_console))
        LOG_LEVEL_ROOT = g_config.map_log_level(g_config.log_level_all)

        for handler in logging.root.handlers[:]:
            logging.root.removeHandler(handler)

        logging.basicConfig(
            level=LOG_LEVEL_ROOT,
            # format='%(asctime)s - %(levelname)s - %(funcName)s - %(filename)s - %(message)s',
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
