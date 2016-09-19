### 关于 ###
这个是基于ESP8266的安防控制盒。
主要有4部分组成。ESP8266，CC1101，STM8L101和DKA120语音芯片。

#### ESP8266 ####
主要实现与服务器的交互，与STM8L的串口通信。支持无线设备的添加和删除，无线设备报警，远程控制，OTA在线升级等等。

#### CC1101 ####
无线射频芯片，1GHz Sub。我们这边使用433M频段。这部分主要用于接收各种设备的发送的信息。

#### STM8L101 ####
主要实现控制CC1101接收，DKA120语音播放，以及与ESP8266的串口通信。

#### DKA120 ####
国产语音芯片。需要什么语音，需要该厂烧录。

如有问题。下次补充。2016-9-19