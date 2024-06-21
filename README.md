# Auto Runhalo

## 构建和安装

进入工程根目录

```shell
$> mkdir build
$> cd build
$> cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=/usr/local
$> make
$> make install
```

### 说明

+ `CMAKE_BUILD_TYPE` 为标准 cmake 参数，支持 Debug, Release 等，例子为具有调试信息的 Release 版本，其他具体的请参见 cmake 官方说明
+ `CMAKE_INSTALL_PREFIX` 为标准 cmake 参数，默认支持 GNU 规范，含义为编译后程序安装的根目录

## 功能说明

工程编译分为两部分，具体如下

```shell
 .
└── bin
    ├── auto_runhalo
    └── find_mods
```

+ `find_mods` 支持查找指定配置文件并输出所有匹配的模块名
+ `auto_runhalo` 基于 `find_mods` 的自动化启动脚本，依次启动所有匹配到的模块，并最终输出详细结果信息

## 快速使用

```shell
$> auto_runhalo -i samples/conf-equipment.xml
```

具体的使用可参见两部分的 `-h`

### 细节

+ 网口名可通过 `-n` 修改，`auto_runhalo` 额外支持 `ETH_DEV` 环境变量修改，环境变量会覆盖选项 `-n`
+ `auto_runhalo` 默认禁用 "IO,LOG,ALARM,HISTORY,RECIPE,CTC,GUI,ERRTOOL"，可通过 `-d` 关闭禁用。此外，`find_mods` 默认不禁用。两者均支持 `-b` 自行设置禁用模块名
+ `auto_runhalo` 默认会检测 `ctf_io_server` 的运行状态，如果未执行会导致失败；可通过 `PPROC` 环境变量设置任意活跃进程以跳过检测
