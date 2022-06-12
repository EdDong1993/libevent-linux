# libevent-linux

## Introduction
Simplify project based on [libevent](https://github.com/libevent/libevent) with version release-2.1.12-stable.

features:
1. Only for Linux;
2. Cmake configurations and files are removed;
3. SSL and DEBUG functions are removed while THREAD functions are reserved;
4. TESTS are removed;
5. Only static lib and shared lib of libevent are compiled and no more core libs or extra libs;
6. Sample codes are reserved but not compiled.

## Compile
```
mkdir build
cd build
cmake ..
make -j4
```

## 简介
基于 [libevent](https://github.com/libevent/libevent) release-2.1.12-stable 版本进行简化，具体简化内容如下：
1. 删减代码只适配 Linux；
2. 移除了 cmake 动态配置，变为固定配置
3. 配置保留了多线程功能，删除了 ssl 和 debug 功能；
4. 删除了 tests；
5. 只编译了 libevent 的动态链接和静态链接库，对于其它的例如 core、extra 等库不再进行编译；
6. 保留了 sample 源码，但同样并不对其进行编译；

## 编译
```
mkdir build
cd build
cmake ..
make -j4
```