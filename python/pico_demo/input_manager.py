
from machine import Pin
from machine import Timer

class InputManager:

    INPUT_DELAY_INTERVAL = 10
    INPUT_DELAY = 400

    def __init__(self, pin_menu):
        self.__pin_menu = Pin(pin_menu, Pin.IN, Pin.PULL_DOWN)
        self.__input_count = 0
        self.__callback_menu = self.__callback_none
        self.__pin_menu.irq (self.__callback_menu_default)
        self.__timer_delay = (Timer(period=InputManager.INPUT_DELAY_INTERVAL,
            mode=Timer.PERIODIC,callback=self.__callback_input_delay))
    
    def __callback_input_delay(self, arg):
        if self.__input_count > 0:
            self.__input_count -= InputManager.INPUT_DELAY_INTERVAL
        if self.__input_count < 0:
            self.__input_count = 0

    def __callback_none(self):
        pass

    def __callback_menu_default(self, arg):
        if self.__input_count == 0:
            self.__input_count = InputManager.INPUT_DELAY
            self.__callback_menu(arg)

    def set_callback_menu(self, callback):
        self.__callback_menu = callback
    
    def disable_callback(self):
        self.__callback_menu = self.__callback_none