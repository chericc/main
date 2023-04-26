# router

import logging
from public import HostInfo
from public import ListenerInterface
import json
import requests

class Header:
    def __init__(self):
        self.__content = {
            'Host': '0.0.0.0',
            'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:81.0) Gecko/20100101 Firefox/81.0',
            'Accept': 'application/json, text/javascript, */*; q=0.01',
            'Accept-Language': 'zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2',
            'Accept-Encoding': 'gzip, deflate',
            'Content-Type': 'application/json; charset=UTF-8',
            'X-Requested-With': 'XMLHttpRequest',
            'Content-Length': '0', # need update. eg. '236'
            'Origin': 'http://0.0.0.0',
            'Connection': 'keep-alive',
            'Referer': 'http://0.0.0.0',
        }
    
    def get(self,host:str,len:int):
        host_url = 'http://' + host
        content = self.__content.copy()
        content['Host'] = host
        content['Content-Length'] = str(len)
        content['Origin'] = host_url
        content['Referer'] = host_url

class Router:
    def __init__(self,host:str,password:str):

        self.__host = host
        self.__password_ = password
        self.__url_host = "http://" + self.__host
        self.__stok = ''
    
    def try_connect(self):
        stok = self.__get_stok()

        logging.info("got a stok: [%s]" % (stok))

    def start_refreshing(self,listener:ListenerInterface):
        pass

    def __get_stok(self):

        header_generator = Header()
        post_session = requests.Session()
        password_encoded = Router.__getsafecode(self.__password_)
        
        dict_post_pwd = {}
        dict_post_pwd['method'] = 'do'
        dict_post_pwd['login'] = {}
        dict_post_pwd['login']['password'] = password_encoded

        password_data = json.dumps(dict_post_pwd).encode()
        dict_header = header_generator.get(self.__host,str(len(password_data)))
        post_response = post_session.post(url=self.__url_host,data=password_data,headers=dict_header)

        json_response = json.loads(post_response.text)
        stok = json_response['stok']
        return stok

    def __securityEncode(a, b, d):
        p = 187; q = 187
        g = len(a); h = len(b); m = len(d)
        e = max(g, h)
        c = ""
        for k in range(e):
            q=p=187
            if k >= g:
                q = b[k]
            else:
                if k >= h:
                    p = a[k]
                else:
                    p = a[k]
                    q = b[k]

            #print (p,q,m)

            if isinstance(p, str):
                p = ord(p)
            if (isinstance(q, str)):
                q = ord(q)
            if (isinstance(m, str)):
                m = ord(m)

            #print (p,q,m)

            c = c + d[ (p ^ q) % m ]

        return c

    def __getsafecode(password):
        return Router.__securityEncode(Router.__string_a, 
            password, Router.__string_d)

    __string_a = "RDpbLfCPsJZ7fiv"
    __string_d = "yLwVl0zKqws7LgKPRQ84Mdt708T1qQ3Ha7xv3H7NyU84p21BriUWBU43odz3iP4rBL3cD02KZciXTysVXiV8ngg6vL48rPJyAUw0HurW20xqxv9aYb4M9wK1Ae0wlro510qXeU07kV57fQMc8L6aLgMLwygtc0F10a0Dg70TOoouyFhdysuRMO51yY5ZlOZZLEal1h0t9YQW0Ko7oBwmCAHoic4HYbUyVeU3sfQ1xtXcPcf1aT303wAQhv66qzW"


logging.basicConfig(level=logging.INFO,format='%(filename)s:%(lineno)d %(levelname)s %(message)s')
route = Router('10.0.0.1','woaini')
route.try_connect()