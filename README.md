# layeredFS Toolkit

layeredFS 游戏翻译数据包生成工具

# 简介

layeredFS 是 NTR CFW 所使用的正版卡带加载外置翻译数据的解决方案，只要将翻译数据安装至SD卡即可在正版卡带运行时动态加载，达到正版卡享受汉化游戏的目的。

layeredFS 支持 NTR 1.0 和 NTR 2.0 及未来的版本。

# 工作原理

layeredFS      通过动态替换游戏的资源文件达到在不对正版卡带进行硬件上的修改的前提下修改部分语言文件的目的。当游戏请求访问资源文件时，将检测SD卡上的翻译数据目录下是否有同名的翻译目录，如果有则进行重定向（如果没有的话仍然访问原始的文件）。只要提供修改过的资源文件的翻译数据，即可实现汉化。


# 使用方法

需要安装下列软件：

devkitARM: http://devkitpro.org/

python 2.7: https://www.python.org/

 1. 将解密后的 exefs.bin 放入 workdir 目录
 2. 运行 build_layeredfs.bat，中途会提示：Enter the folder of the layeredFS file: 此时输入翻译数据包的目录名，例如 layered （建议使用游戏的简称，不要和已知的游戏重复）。
 3. 将会在 workdir 里生成 layeredfs.plg 文件，拷贝到 SD 卡，路径为：plugin/游戏ID/layeredfs.plg。
 4. 准备翻译数据， 将所有翻译和修改过的 romfs 文件，拷贝到 SD 卡中步骤 2 所输入的目录下，例如 layered 目录。
 5. 运行正版游戏后，屏幕会闪绿色，并开始应用翻译数据，重定向文件。
 
 
