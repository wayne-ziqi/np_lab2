# word game

## Bug
### 3.28
* 三个client A, B, C，A 可以向 B C都发送挑战请求，但应当规定只可以发给一个（使用CONNECTING状态）。并设置超时机制，对方在10s内决定，否则拒绝

## TODO
### 3.28
* server在rcv失败（连接断开时）应当广播该用户下线，对手在client端应当能够从BUSY状态切换到VACANCY状态