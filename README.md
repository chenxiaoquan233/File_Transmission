# File Transmission

![](https://img.shields.io/badge/build-passing-brightgreen)

厦门大学信息学院计算机网络课程项目

作者： [chenxiaoquan233](https://github.com/chenxiaoquan233),[fester](https://github.com/ferster),[jrsmith12138](https://github.com/jrsmith12138),[firstday1](https://github.com/firstday1),[luckydog691](https://github.com/luckydog691),[Morstr](https://github.com/Morstr)


<!-- TOC -->
## 目录

- [Video Code](#video-code)
- [简介](#简介)
- [编译环境](#编译环境)
- [运行环境](#运行环境)
- [使用方法](#使用方法)
- [Change log](#change-log)
- [TODO](#TODO)
- [交流反馈](#交流反馈)
<!-- /TOC -->

## 简介
将指定文件夹的文件发送到另一个文件夹

## 编译环境
## 运行环境
## 使用方法
## Change log
## TODO
### Client:
#### 1.
成员sock改名为cmd_sock  
添加一个socket指针，命名为data_sock，指向多个传输数据的sock  
#### 2.  
把set_up_connection()改为sock_init()函数，分平台定义  
windows:
```
sock_init(SOCKET sock, const char* ip_addr, int port)
```
linux:
```
sock_init(int sock, const char* ip_addr, int port)
```
其中添加端口是否可用的检查
#### 3.
添加parse_arg()函数
```
parse_arg(int argc, char** argv)
```
首先判断参数是否合法  
再判断路径是否存在  
再判断是否有读权限
#### 4.
添加一个读取文件路径的函数
```
void read_path(char* path, char* buffer)---->完成 int read_path(const char* path,char *buffer[]) 
变更：传入变为二维数组，返回值为读取到的文件夹路径数目
```
递归读取path路径下的信息  
每一个文件夹按照(双引号内格式)进行存储:
```
文件夹: "d 文件夹名\n"
```
文件夹名应该包括从path开始的相对路径
#### 5.
添加一个发送路径信息的函数
```
bool send_path_info(char* buffer)
```
从命令sock发送命令:
```
INFO
```
服务端返回一个端口  
根据这个端口创建一个临时数据sock，从这个sock发送buffer内容  
若长度过长需要切片  
命令sock收到服务端的"INFO"作为回传则表示成功，否则重传  
超过次数后告诉用户失败  
#### 6.
添加一个常量 SEND_FREQ 表示最大重传次数
#### 7.完成
添加压缩函数(或调用库)
```
int zip(char* src, char* dest, int src_len)
```
将src中长度为src_len的数据压缩到dest中  
返回压缩后的数据长度
#### 8.
添加发送指令的函数
```
void send_cmd(char* cmd)
```
#### 9.
修改send_file函数
先向服务器发送命令
```
SEND 文件名
```
等待服务端返回
```
OFFS 偏移量
```
然后向服务器发送
```
PORT
```
服务器返回
```
PORT 端口号
```
建立临时数据sock，向对应端口发送数据
### Server:
#### 1.完成
添加参数检查函数
```
parse_arg(int argc, char** argv)
```
检查参数是否合法  
再检查路径是否存在并具有读写权限
#### 2.完成
添加解析命令函数
```
INFO 调用parse_path
SEND 调用check_file
PORT 调用check_port
```
#### 3.完成
添加parse_path函数
```
parse_path(char* path)
```
从数据sock读入路径信息，回传INFO，解析后放入path中
之后调用set_dir函数
#### 4.完成
添加set_dir函数
```
set_dir(char* path)
```
对path中每一个路径进行检查，若不存在该文件夹则建立文件夹
#### 5.
添加文件检查函数
```
check_file(char* file_name)
```
```
if 文件存在  
    if 日志存在
        if 文件长度和日志记录长度相同
            断点续传
其他情况皆从头开始传输
```
回传偏移量 断点续传则是文件长度，重传则为0
#### 6.完成
添加检查端口函数
开始端口为
```
start_port = cmd_port > 1024 ? cmd_port + 1 : 1024
```
从start_port开始检查端口是否被占用  
若是则依次加1  
找到空闲端口后建立数据sock  
从命令端口返回:
```
PORT 端口号
```
#### 7.完成
添加解压函数
```
int unzip(char* src, char* dest, int src_len)
```
将src中长度为src_len的数据解压到dest，返回解压后数据长度
#### 8.
修改接收文件函数
每一个文件若未接收完全，则先将已传输的合并，再生成一个日志文件
格式为"文件名.FTlog"
## 交流反馈
(排名不分先后)  
chenxiaoquan233:  
邮箱: chenxiaoquan233@gmail.com  
QQ: 770355275

fester:  
邮箱: 1014483832@qq.com  
QQ: 1014483832  
  
jrsmith12138:  
邮箱: 849070287@qq.com  
QQ: 849070287  
  
firstday1:  
邮箱: 2602164122@qq.com  
QQ: 2602164122  
  
luckydog691:  
邮箱: 1256161580@qq.com  
QQ: 1256161580  
  
Morstr:  
邮箱: 260603965@qq.com  
QQ: 260603965
