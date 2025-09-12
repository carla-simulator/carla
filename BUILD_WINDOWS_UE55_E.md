Windows 下构建 CARLA UE5.5（引擎放在 E 盘）

目标
- 在 Windows 11 上从源码构建 CARLA（UE5.5 分支），并将 UE5.5 引擎与缓存放置在 E 盘，减少 C 盘占用。
- 全程记录日志，适合“启动后去休息”的无人值守场景（仍需在拉取 UE 源码时完成 GitHub 授权）。

环境前提
- Windows 11（已开启“开发人员模式”）
- Visual Studio 2022（缺少组件即可由脚本自动补齐）
- Git、CMake、Ninja（仓库脚本会检测并安装缺失项）
- Epic ↔ GitHub 账号已关联，可访问 CarlaUnreal/UnrealEngine 私有仓库
- E 盘可用空间尽量充足（建议 ≥ 180–225GB；仓库、UE 源码/构建、内容资源与中间文件都较大）

目录与缓存规划
- UE 引擎目录（默认）：`E:\UnrealEngine5_carla`
- UE 缓存（DDC）：
  - `E:\UE_DDC`
  - `E:\UE_DDC_Local`
- 仓库路径：`E:\Project\AI\python\project\carla`（当前已在 E 盘，无需迁移）

一、一次性准备（仅首次）
1) 以管理员方式打开“x64 Native Tools Command Prompt for VS 2022”。
2) 创建缓存目录并设置环境变量（避免占用 C 盘）：
   - `mkdir E:\UE_DDC 2>nul`
   - `mkdir E:\UE_DDC_Local 2>nul`
   - `setx UE-SharedDataCachePath E:\UE_DDC`
   - `setx UE-LocalDataCachePath  E:\UE_DDC_Local`
3) 可选：若希望 UE 固定放在 `E:\UnrealEngine5_carla`，提前设置并持久化：
   - `setx CARLA_UNREAL_ENGINE_PATH E:\UnrealEngine5_carla`
   - 若该目录事先不存在，CarlaSetup 会在仓库同级创建 `..\UnrealEngine5_carla`（本仓库位于 E 盘，仍满足“放在 E 盘”要求）。

二、启动构建（可无人值守）
在仓库根目录执行新脚本：

```
Start-CarlaUE55-Build-E.bat
```

该脚本会：
- 激活 VS 构建环境；
- 设置 UE 缓存到 E 盘；
- 自动选择 Python 3.8（若存在 `D:\Project\soft\Python38`）；
- 运行 `CarlaSetup.bat --interactive` 完整构建流程，包括：
  - （如必要）安装先决条件（VS 组件、Python 包、Ninja 等）；
  - 下载 CARLA 内容；
  - 克隆并构建 CARLA 专用 UE5.5 fork；
  - 配置并构建 CARLA；
  - 安装 Python API；
- 将输出追加写入 `Build_Start_E.log` 便于次日查看。

注意：
- Windows 端克隆 UE 仓库需要在 Git 提示时登录 GitHub（具备 Epic 授权）。脚本会在该步骤等待你的凭据。
- 初次构建时间较长（数小时）。建议保证 E 盘空间充足与网络稳定。

三、构建完成后的常用命令
- 重新配置（如修改代码后）：
  - `cmake -G Ninja -S . -B Build --toolchain=%cd%\CMake\Toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DPython_ROOT_DIR=D:\Project\soft\Python38 -DPython3_ROOT_DIR=D:\Project\soft\Python38`
- 构建：
  - `cmake --build Build`
- 安装 Python API：
  - `cmake --build Build --target carla-python-api-install`
- 启动编辑器：
  - `cmake --build Build --target launch`

四、验证与示例
- 安装示例依赖：
  - `py -3.8 -m pip install -r PythonAPI\examples\requirements.txt`
- 启动 CARLA 后运行手动驾驶示例：
  - `py -3.8 PythonAPI\examples\manual_control.py`

五、常见问题
- 找不到 cl.exe / SDK：先运行 `Util\SetupUtils\InstallPrerequisites.bat` 或用 VS Installer 安装“使用 C++ 的桌面开发”、MSVC v143、Windows 11 SDK、CMake for VS。
- 拉取 UE 失败：确认 Epic 账号已授权 GitHub，按提示登录；也可预先手动在 `E:\UnrealEngine5_carla` 克隆好仓库后再运行脚本。
- 空间不足：清理 E 盘或迁移到更大磁盘；缓存与构建工件都可指向 E 盘以减少 C 盘压力。

六、日志位置
- 本脚本日志：`Build_Start_E.log`
- `RunCarlaSetup.bat` 日志：`RunCarlaSetup.log`
- 构建日志：`BuildCarlaUE5.log`（如果使用仓库自带的构建脚本）

附：手动一步到位（不使用脚本）
1) 管理员 VS x64 工具命令行，进入仓库根：
   - `cd E:\Project\AI\python\project\carla`
2) 设置缓存路径（一次性）：
   - 见“一次性准备”第 2 步。
3) 直接运行：
   - `CarlaSetup.bat --python-root=D:\Project\soft\Python38 --interactive`
4) 首次完成后，按“构建完成后的常用命令”进行增量构建与运行。

