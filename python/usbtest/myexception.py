

class MyException(Exception):
    """自定义异常，仅用于显示提示信息"""

    def __init__(self, message):
        self.message = message

    def __str__(self):
        return self.message

