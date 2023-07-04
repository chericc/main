import json
from state_query import StateQuety

def callback(response: str):
    dict_response = json.loads(response)

    host_info : dict = dict_response["hosts_info"]["host_info"]

    for i in host_info:
        i : dict
        items : dict = list(i.values())[0]
        for (k,v) in items.items():
            print(k,v)


def main():

    query = StateQuety("192.168.128.1", "woaini", callback)
    query.run()

if __name__ == '__main__': 
    main()