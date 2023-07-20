# mymuduo



### 简介

- 仿照muduo库实现了EventLoop、EventLoopThread、EventLoopThreadPool、TcpConnection、TcpServer等muduo库中的网络IO核心类
- 实现日志系统，实现了muduo日志库中的核心类（Logging、AsyncLogging等）
- 基于重写的muduo库实现了http服务器，提供了http报文解析功能，并支持POST请求
- 基于重写的muduo库实现了简单的跳表服务器以及简单的跳表服务器客户端，可以实现增删改查



### 安装

```shell
sudo ./build.sh
```



### 运行

- HttpServer

  ```shell
  cd bin
  sudo ./HttpServer
  ```

- SkipList

  ```shell
  cd bin
  sudo ./SkipListServer
  
  sudo ./SkipListClient
  ```



### Reference

- https://github.com/chenshuo/muduo
- https://github.com/linyacool/WebServer
- https://github.com/youngyangyang04/Skiplist-CPP