# wifi6


## Support CHIP

|      CHIP        | Remark |
|:----------------:|:------:|
|BL616/BL618       |        |

## Compile

- BL616/BL618

```
make CHIP=bl616 BOARD=bl616dk
```

## Flash

```
make flash COMX=xxx ## xxx is your com name
```

## How use wifi udp test


```             
              ))                                 ____________
    \|/      )))         \|/  \|/               |            |
     |______           ___|____|___             |  Host pc   |
    |       |         |            |            |____________|
    | BL616 |         |   Router   |#<-------->#/            /#
    |_______|         |____________|           /____________/#   
   192.168.1.3         192.168.1.1              192.168.1.2    
                      SSID: SIPEED_TEST
                    Password:12345678
```

### udp echo test

On BL616 Board start udp server

```bash
bouffalolab />wifi_sta_connect SIPEED_TEST 12345678
bouffalolab />wifi_udp_server
bouffalolab />udp server task start ...
wifi_udp_server [port]
         port: local listen port, default port 5001
udp bind success!
Server ip Address : 0.0.0.0:5001
Press CTRL-C to exit.
recv[2/204800] from 172.49.14.160
recv[10240/204798] from 172.49.14.160
recv[10240/194558] from 172.49.14.160
recv[10240/184318] from 172.49.14.160
recv[10240/174078] from 172.49.14.160
recv[10240/163838] from 172.49.14.160

```

```bash
$ python up.py <image_file>

```

extra

```bash
while [ 1 ]; do for img in `ls *.png -m|tr -d ','`; do python up.py $img; sleep 1; done; done

```
