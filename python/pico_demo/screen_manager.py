from machine_i2c_lcd import I2cLcd
from machine import Timer
from machine import RTC
from machine import I2C
from machine import Pin
from machine import ADC

class ScreenManager:

    ViewDefault = 0
    ViewMenu = 1
    ViewTime = 2
    ViewTemp = 3
    ViewButt = 4
    
    UPDATE_INTERVAL = 1000

    def __init__(self, i2c_index, i2c_sda, i2c_scl, width, height):
        self.__i2c_index = i2c_index
        self.__i2c_scl = i2c_scl
        self.__i2c_sda = i2c_sda
        self.__width = width
        self.__height = height
        self.__enum_view_type = ScreenManager.ViewDefault
        self.__i2c = I2C(i2c_index, sda = Pin(i2c_sda), scl = Pin(i2c_scl))
        self.__lcd = I2cLcd(i2c=self.__i2c, i2c_addr=0x27, num_lines=self.__height, num_columns=self.__width)
        self.__sensor_adc = ADC(4)
        self.__EMPTY_FULL_STR = ''
        self.__EMPTY_LINE_STR = ''
        for i in range (width * height):
            self.__EMPTY_FULL_STR += ' '
        for i in range (width):
            self.__EMPTY_LINE_STR += ' '

    def __get_datetime_str(self):
        datetime = RTC().datetime()
        strTime = ('%04d/%02d/%02d' %
            (datetime[0],datetime[1],datetime[2]))
        return strTime
    
    def __get_time_str(self):
        datetime = RTC().datetime()
        strTime = ('%02d:%02d:%02d' %
            (datetime[4],datetime[5],datetime[6]))
        return strTime

    def __make_head_str(self, head_name):
        head_str = head_name.center(self.__width)
        #print ('headlen=' + str(len(head_str)))
        return head_str
    
    def __make_content_str(self, content):
        content_str = content.center(self.__width)
        #print ('contentlen=' + str(len(content_str)))
        return content_str

    def __make_screen_string_default (self):
        timestr = self.__get_time_str()
        finalstr = self.__make_head_str('Welcome') + self.__make_content_str(timestr)
        return finalstr

    def __make_screen_string_menu (self):
        finalstr = self.__make_head_str('Menu') + self.__make_content_str('none')
        return finalstr

    def __make_screen_string_datetime(self):
        datetimestr = self.__get_datetime_str()
        finalstr = self.__make_head_str('Date') + self.__make_content_str(datetimestr)
        return finalstr

    def __make_screen_string_temp(self):
        adc_read = self.__sensor_adc.read_u16()
        volte = adc_read * 3.3 / 65535
        temperature = 27 - (volte - 0.706) / 0.001721
        finalstr = self.__make_head_str('Temperature') + self.__make_content_str('%.2f' % temperature)
        return finalstr

    def __update_screen(self):
        #print ('update screen')
        if self.__enum_view_type == ScreenManager.ViewDefault:
            content = self.__make_screen_string_default()
            self.__lcd.move_to(0,0)
            self.__lcd.putstr (content)
        elif self.__enum_view_type == ScreenManager.ViewMenu:
            content = self.__make_screen_string_menu()
            self.__lcd.move_to(0,0)
            self.__lcd.putstr(content)
        elif self.__enum_view_type == ScreenManager.ViewTime:
            content = self.__make_screen_string_datetime()
            self.__lcd.move_to(0,0)
            self.__lcd.putstr (content)
        elif self.__enum_view_type == ScreenManager.ViewTemp:
            content = self.__make_screen_string_temp()
            self.__lcd.move_to(0,0)
            self.__lcd.putstr(content)
        else:
            print ('undefined type=%d' % self.__enum_view_type)


    def __update_screen_callback(self, arg):
        self.__update_screen()

    def __reset_screen(self):
        self.__lcd.display_off()
        self.__lcd.hide_cursor()
        self.__lcd.clear()
        self.__lcd.move_to(0,0)
        self.__lcd.display_on()

    def start(self):
        self.__update_timer = Timer(period=ScreenManager.UPDATE_INTERVAL,mode=Timer.PERIODIC,callback=self.__update_screen_callback)

    def stop(self):
        self.__update_timer.deinit()
    
    def nextview(self):
        print ('next view')

        if self.__enum_view_type + 1 >= ScreenManager.ViewButt:
            self.__enum_view_type = ScreenManager.ViewDefault
        else:
            self.__enum_view_type += 1
        
        self.__reset_screen()
        self.__update_screen()
    
    def restore_default_view(self):
        print ('restore default view')

        self.__enum_view_type = ScreenManager.ViewDefault

        self.__reset_screen()
        self.__update_screen()