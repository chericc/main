# -*- coding: utf-8 -*-

from email import header
from urllib import parse
import requests
import logging
import json

from route import getsafecode

host = '10.0.0.1'
password = 'woaini'

url_host = 'http://' + host
header_pwd = {
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

header_pwd['Host'] = host
header_pwd['Origin'] = url_host
header_pwd['Referer'] = url_host

logging.basicConfig(level=logging.DEBUG,format='%(filename)s:%(lineno)d %(levelname)s %(message)s')

code = getsafecode(password)
logging.debug(code)

dict_post_pwd = {}
dict_post_pwd['method'] = 'do'
dict_post_pwd['login'] = {}
dict_post_pwd['login']['password'] = code

print(dict_post_pwd)

post_str = json.dumps(dict_post_pwd).encode()

header_pwd['Content-Length'] = str(len(post_str))

session = requests.Session()
response = session.post(url_host, data=post_str, headers=header_pwd)

json_login_info = json.loads (response.text)
print(json_login_info)

stok = json_login_info['stok']

print(stok)

stok_decode = parse.unquote(stok)

print(stok_decode)

dict_get_hosts_info = {}
dict_get_hosts_info['hosts_info'] = {}
dict_get_hosts_info['hosts_info']['table'] = 'host_info'
dict_get_hosts_info['hosts_info']['name'] = 'cap_host_num'
dict_get_hosts_info['network'] = {}
dict_get_hosts_info['network']['name'] = 'iface_mac'
dict_get_hosts_info['hyfi'] = {}
dict_get_hosts_info['hyfi']['table'] = 'connected_ext'
dict_get_hosts_info['method'] = 'get'

text_get_hosts_info = json.dumps (dict_get_hosts_info)
data_get_hosts_info = text_get_hosts_info.encode()

url_getonlinedev = url_host + '/stok=' + stok_decode + '/ds'

header_gethost = header_pwd # re-ref
header_gethost['Content-Length'] = str(len(data_get_hosts_info))

response_gethost = session.post (url_getonlinedev, data=data_get_hosts_info, headers=header_gethost) # post

json_gethost_info = json.loads (response_gethost.text)
print(json_gethost_info)