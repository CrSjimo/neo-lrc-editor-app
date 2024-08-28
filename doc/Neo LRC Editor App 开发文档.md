# Neo LRC Editor App 开发文档

## 创意来源

LRC 是一种可以使歌词与音频同步的文件格式，被广泛应用于各种音乐播放器软件中。因此，需要一个方便的实用软件来进行编辑。目前已有的可视化编辑器大多只支持实时制作歌词，无法实现对歌词时间可视化的精确调整。本软件在实现了基本的制作功能的基础上，还提供的一些方便的时间调整功能。

## 实现思路

文档编辑部分采用 MVC 架构，在 Controller 层实现撤销重做功能。 音频播放模块可以读取音频文件并将其播放到音频设备，并且通过音频数据绘制波形图。

## 运行环境

这个软件基于 Qt 6.5.3，并使用 vcpkg 管理依赖库 libsndfile 和 SDL2。配置 Qt6_DIR 环境变量和 vcpkg 相关的 CMake 参数即可编译。

## 技术细节

1. 使用 QRegularExpression，通过正则表达式解析 LRC 文件格式。

2. MVC 架构采用了 Qt 自带的 QStandardItemModel，使用 QTreeView 作为基本编辑视图，QGraphicsView 作为可视化编辑视图，并采用 QSortFilterModel 实现按歌词时间码排序。在 Controller  层接入 QUndoStack 实现撤销重做功能。

3. 实现了可变分辨率的波形图绘制，对音频数据储存了 16 倍，256 倍，4096 倍三个缩放档次的 mipmap，在绘图时进行计算。

4. 利用 QJSEngine 提供可编程接口，用户可以编写 JavaScript 脚本，实现自动化编辑（类似 Microsoft Office 中的宏）

5. 通过 libsndfile 读取音频文件，然后通过 SDL2 将音频输出至音频设备。

6. 本软件已使用 InnoSetup 打包，用户安装即可运行。并且还通过 GitHub Actions 执行自动构建。

## 收获

1. 掌握了 Qt 内置的 MVC 框架的基本功能。

2. 学习了 QGraphicsView 的用法，并学会了诸如移动，缩放，就地编辑等操作的实现方式。

3. 实现了一套音频波形图绘制算法，可以在其它程序的开发中复用。


