#

gcc test.c -Wl,-Map,link.map 
objdump -d -S a.out > a.S
