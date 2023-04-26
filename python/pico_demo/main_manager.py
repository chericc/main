
from machine import Timer

from screen_manager import ScreenManager
from input_manager import InputManager

class MainManager:

    INPUT_PIN = 13

    RESTORE_DEFAULT_VIEW_INTERVAL = 1000
    RESTORE_DEFAULT_VIEW_SECS = 300

    def __init__(self):
        self.__screen_manager = ScreenManager(i2c_index=1, i2c_sda=2, i2c_scl=3, width=16, height=2)
        self.__input_manager = InputManager(MainManager.INPUT_PIN)
        self.__restore_default_view_count_num = 0
        self.__restore_default_view_timer = (Timer(period=MainManager.RESTORE_DEFAULT_VIEW_INTERVAL,
            mode=Timer.PERIODIC,callback=self.__callback_restore_default_view_counter))
    
    def __restore_default_view_count(self):
        self.__restore_default_view_count_num += 1
        if self.__restore_default_view_count_num > MainManager.RESTORE_DEFAULT_VIEW_SECS:
            self.__restore_default_view_count_num = 0
            self.__screen_manager.restore_default_view()

    def __callback_restore_default_view_counter(self, arg):
        self.__restore_default_view_count()

    def callback_nextview(self, pin):
        if pin.value() > 0:
            self.__restore_default_view_count_num = 0
            self.__screen_manager.nextview()

    def start(self):
        self.__screen_manager.start()
        self.__input_manager.set_callback_menu (self.callback_nextview)

    def stop(self):
        self.__input_manager.disable_callback()
        self.__screen_manager.stop()    
