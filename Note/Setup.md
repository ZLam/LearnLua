# Lua 源码编译配置
![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/Xnip2023-03-28_01-04-01.jpg)
## 步骤
* 在 https://www.lua.org/ 下载源码
* 像上面目录图那样创建 Lua , src , proj , win , mac 对应层级的目录
* 将源码目录 src 里的所有文件复制到我们新建的 src 文件夹里
* Windows
  1. 在我们新建的 win 目录下 , 新建一个名为 Lua 的 empty solution
  2. 添加一个名为 LuaLib 的 proj 进 solution
  3. LuaLib 的 proj 要设成 编译成 static lib , 使用 c++14 或以上编译 , Character Set 使用 Unicode
  4. 添加源码进行编译(除了 lua.c 和 luac.c)
  5. 编完应该能得到 LuaLib.lib
  6. 添加一个名为 Luac 的 proj 进 solution
  7. Luac 的 proj 要设成 编译成 exe , 使用 c++14 或以上编译 , Character Set 使用 Unicode , 加添库目录链接上 LuaLib.lib
  8. 添加源码 luac.c 进行编译
  9. 编完应该能得到 Luac.exe
  10. 添加一个名为 Lua 的 proj 进 solution
  11. Lua 的 proj 要设成 编译成 exe , 使用 c++14 或以上编译 , Character Set 使用 Unicode , 加添库目录链接上 LuaLib.lib
  12. 添加源码 lua.c 进行编译
  13. 编完应该能得到 Lua.exe
* Mac OS
  1. 在我们新建的 mac 目录下 , 新建一个名为 Lua 的 empty project
  2. 添加一个名为 LuaLib 的 target(library类型) 进 project
  3. LuaLib 的 Build Settings 设置 , 使用 c++17 或以上编译
  4. LuaLib 的 Build Phrases 设置 , 把源码所有头文件拖进 Headers , 把所有源码c文件拖进 Compile Sources
  5. 编译 , 最后应该能得到 libLuaLib.a
  6. 添加一个名为 Luac 的 target(command line tool类型) 进 project
  7. Luac 的 Build Settings 设置 , 使用 c++17 或以上编译
  8. Luac 的 Build Phrases 设置 , 把 luac.c 文件拖进 Compile Sources , 引用 libLuaLib.a 库
  9. 编译 , 最后应该能得到 Luac.exec
  10. 添加一个名为 Lua 的 target(command line tool类型) 进 project
  11. Lua 的 Build Settings 设置 , 使用 c++17 或以上编译
  12. Lua 的 Build Phrases 设置 , 把 lua.c 文件拖进 Compile Sources , 引用 libLuaLib.a 库
  13. 编译 , 最后应该能得到 Lua.exec
* Misc
  * ```XCOPY "$(ProjectDir)..\..\..\..\res" "$(OutDir)\res\" /S /Y```
  * ```cp -f -R "${PROJECT_DIR}/../../../res/." "${BUILT_PRODUCTS_DIR}/res"```