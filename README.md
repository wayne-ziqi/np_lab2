# word game

## Bug
### 3.28
* 三个client A, B, C，A 可以向 B C都发送挑战请求，但应当规定只可以发给一个（使用CONNECTING状态）。并设置超时机制，对方在10s内决定，否则拒绝
  * fix: oppo state information shouldn't change user state unless oppo is in game or offline

## TODO
### 3.28
* server在rcv失败（连接断开时）应当广播该用户下线，对手在client端应当能够从BUSY状态切换到VACANCY状态

## 项目简介

剪刀石头布文字小游戏的客户端和服务器实现

## 项目结构

```
.
├── Cmakelists.txt
├── README.md
├── client
│   ├── Client.cpp
│   ├── Cmakelists.txt
│   ├── GTCPController.cpp
│   ├── MainScene.cpp
│   ├── TCPController.cpp
│   ├── include
│   │   ├── Client.hpp
│   │   ├── ClientException.h
│   │   ├── GTCPController.h
│   │   ├── MainScene.hpp
│   │   ├── TCPController.h
│   │   └── ToMsgBox.hpp
│   └── main.cpp
├── lib
│   ├── CMakeLists.txt
│   ├── common.h
│   ├── error.c
│   └── read.c
└── server
    ├── CmakeLists.txt
    ├── Competition.cpp
    ├── Player.cpp
    ├── SNetController.cpp
    ├── Server.cpp
    ├── include
    │   ├── Competition.h
    │   ├── Player.h
    │   ├── SNetController.h
    │   ├── Server.h
    │   └── ServerException.h
    └── main.cpp

```

## 项目构建

确保安装`qt5`依赖库，可以通过`pip install PyQt5`或者官网安装获取，在项目根目录下执行以下命令完成构建，项目文件位于target/bin目录下

```shell
mkdir build && cd build
cmake .. && make -j 6
```

