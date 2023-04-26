# -*- coding: utf-8 -*-

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