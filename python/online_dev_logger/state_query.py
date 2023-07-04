
import requests
import logging
import json
from urllib import parse
import time


string_a = "RDpbLfCPsJZ7fiv"
string_d = "yLwVl0zKqws7LgKPRQ84Mdt708T1qQ3Ha7xv3H7NyU84p21BriUWBU43odz3iP4rBL3cD02KZciXTysVXiV8ngg6vL48rPJyAUw0HurW20xqxv9aYb4M9wK1Ae0wlro510qXeU07kV57fQMc8L6aLgMLwygtc0F10a0Dg70TOoouyFhdysuRMO51yY5ZlOZZLEal1h0t9YQW0Ko7oBwmCAHoic4HYbUyVeU3sfQ1xtXcPcf1aT303wAQhv66qzW"

def securityEncode(a, b, d):
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

def getsafecode(password):
    return securityEncode(string_a, password, string_d)

class StateQuety:
    def __init__(self, ip_address, password, callback):
        self.__ip = ip_address
        self.__pwd = password
        self.__session = requests.Session()
        self.__url_host = 'http://' + self.__ip
        self.__callback = callback

    def gen_header(self):
        url_header = {
            'Host': '10.0.0.1',
            'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:81.0) Gecko/20100101 Firefox/81.0',
            'Accept': 'application/json, text/javascript, */*; q=0.01',
            'Accept-Language': 'zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2',
            'Accept-Encoding': 'gzip, deflate',
            'Content-Type': 'application/json; charset=UTF-8',
            'X-Requested-With': 'XMLHttpRequest',
            'Content-Length': '0', # need update. eg. '236'
            'Origin': 'http://10.0.0.1',
            'Connection': 'keep-alive',
            'Referer': 'http://10.0.0.1/',
        }
        
        url_header['Host'] = self.__ip[:]
        url_header['Origin'] = self.__url_host[:]
        url_header['Referer'] = self.__url_host[:]
        return url_header
    
    def gen_quest(self):
        dict_get_hosts_info = {}
        dict_get_hosts_info['hosts_info'] = {}
        dict_get_hosts_info['hosts_info']['table'] = 'host_info'
        dict_get_hosts_info['hosts_info']['name'] = 'cap_host_num'
        dict_get_hosts_info['network'] = {}
        dict_get_hosts_info['network']['name'] = 'iface_mac'
        dict_get_hosts_info['hyfi'] = {}
        dict_get_hosts_info['hyfi']['table'] = 'connected_ext'
        dict_get_hosts_info['method'] = 'get'
        return dict_get_hosts_info

    def run(self):
        code = getsafecode(self.__pwd)
        dict_post_pwd = {}
        dict_post_pwd['method'] = 'do'
        dict_post_pwd['login'] = {}
        dict_post_pwd['login']['password'] = code

        bytes_login = json.dumps(dict_post_pwd).encode()

        header_login = self.gen_header()
        header_login['Content-Length'] = str(len(bytes_login))

        response = self.__session.post(self.__url_host, data=bytes_login, headers=header_login)

        json_login_info = json.loads (response.text)
        stok = json_login_info['stok']
        stok_decode = parse.unquote(stok)

        bytes_gethostinfo = json.dumps(self.gen_quest()).encode()
        url_getonlinedev = self.__url_host + '/stok=' + stok_decode + '/ds'

        header_info = self.gen_header()
        header_info['Content-Length'] = str(len(bytes_gethostinfo))

        while True:
            response_info = self.__session.post (url_getonlinedev, data=bytes_gethostinfo, headers=header_info)
            self.__callback(response_info.text)
            time.sleep(2)
