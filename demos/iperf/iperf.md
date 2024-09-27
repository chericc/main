# iperf.md

## 

./iperf3_x86 -s -p 10010 -f k --idle-timeout 1 --rcv-timeout 1000

./iperf3_arm -c 10.0.0.3%wlan0 -p 10010 -b 10M -t 10000
./iperf3_arm -c 10.0.0.3%wlan0 -p 10011 -b 10M -t 10000 -R

./iperf3_arm -c 10.0.0.3%wlan0 -p 10010 -b 10M -t 10000 -u
./iperf3_arm -c 10.0.0.3%wlan0 -p 10011 -b 10M -t 10000 -R -u

iperf3 -c iperf3.moji.fr%wlan0 -p 5200 -b 10M -t 10000
