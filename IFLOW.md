# CoD 项目 - 交互式指令上下文

本文件为 CoD（按需编译）项目提供全面的指令上下文，用于与 AI 助手交互。

## 项目概述

CoD 是一个创新的 C++20 编译工具，旨在提供"按需编译"的 REPL 体验，完全基于 LLVM/Clang 生态系统构建。该项目采用零成本内存区域系统，支持持久化工作区，并实现无需 JIT 的交互式编译和执行。

### 核心技术栈

- **语言**: C++20（严格遵循现代 C++ 特性）
- **编译器工具链**: LLVM 18.x + Clang + lld + libc++ + lldb
- **构建系统**: CMake + Ninja
- **核心库**: shilos（自主开发的内存区域库）
- **集成**: LLVM 项目作为外部依赖
- **平台支持**: macOS（优先）、Linux（Ubuntu）

### 项目架构

项目由三个主要组件组成：

1. **shilos** (`include/shilos/`) - 低级内存区域库

   - 零成本可重定位内存区域系统
   - regional_ptr 和 global_ptr 智能指针
   - DBMR（磁盘支持内存区域）支持
   - 高级概念验证：RegionalType、RegionalConstructibleWithRegion
   - YAML 序列化转换工具
   - 异常安全保证机制

2. **cod** (`cod/`) - 按需编译 REPL 驱动

   - 交互式编译和执行体验
   - 持久化工作区状态管理（DBMR）
   - 基于 LLVM 的编译管道
   - 智能缓存系统（AST 语义哈希）
   - 独立进程执行模型（无 JIT）
   - 增强的 WorksRoot 类型管理

3. **codp** (`codp/`) - 包管理器
   - 基于 Git 的 VCS 原生包管理
   - 确定性依赖解析
   - UUID 标识的依赖管理
   - 锁文件（`CodManifest.yaml`）支持
   - 本地依赖覆盖支持
   - 分支管理和远程仓库验证

## 构建和运行指南

### 系统要求

- **macOS** 或 **Linux** (Ubuntu)
- Git
- CMake 3.20+
- Ninja 构建工具
- 必要的系统依赖（参见 `prepare-macos.sh` 和 `prepare-ubuntu.sh`）

### 构建项目

项目使用自举构建方式：

```bash
# 使用项目的构建脚本（推荐）
./do-build-cod.sh

# 设置构建类型（可选）
export COD_BUILD_TYPE=Release  # Debug, RelWithDebInfo 也支持
export COD_TARGETS_TO_BUILD="Native;NVPTX"  # CUDA 就绪平台
```

**注意**: 构建过程采用 CoD 项目的自举构建机制，会自动下载并编译完整的 LLVM 18.x 工具链，这是一个资源密集型操作，需要约 30-60 分钟。对于终端用户而言，无需了解内部构建细节，直接使用构建脚本即可。

### 手动构建选项

```bash
# 传统方法（如果需要）
cmake --build ./build

# 组件特定构建
cmake --build ./build --target shilos
cmake --build ./build --target cod
cmake --build ./build --target codp
```

### 运行 REPL

```bash
# 启动交互式 REPL
./built/bin/cod

# 单表达式评估
./built/bin/cod -e "std::cout << \"Hello, World!\" << std::endl;"

# 使用自定义工作区
./built/bin/cod -w ./my_workspace.dbmr --project /path/to/project

# 详细输出模式
./built/bin/cod -v

# 强制重建
./built/bin/cod --force-rebuild

# 禁用缓存
./built/bin/cod --no-cache
```

### 包管理操作

```bash
# 初始化项目
codp init

# 解析依赖
codp solve

# 更新依赖
codp update

# 添加依赖
codp add <repo-url>

# 移除依赖
codp rm <repo-url>

# 调试模式
codp debug
```

## 关键概念和规范

### 内存区域系统（shilos）

**核心原则**:

- 零成本重定位是内存区域类型的强制要求
- 内存区域类型只能在内存区域中分配（绝不在栈上）
- 内存区域类型没有复制/移动语义
- 创建内存区域对象的构造函数必须接受 `memory_region&` 参数
- 使用 `regional_ptr<T>` 进行区域内引用
- 使用 `global_ptr<T, RT>` 进行跨区域引用

**高级概念**:

```cpp
template <typename T>
concept RegionalType = std::is_trivially_destructible_v<T> || std::is_destructible_v<T>;

template <typename T, typename RT>
concept RegionalConstructibleWithRegion = requires {
  requires ValidMemRegionRootType<RT>;
  requires std::constructible_from<T, memory_region<RT> &>;
};
```

**关键类型**:

```cpp
template<typename RT> class memory_region;
template<typename VT> class regional_ptr;
template<typename VT, typename RT> class global_ptr;
template<typename RT> class auto_region; // RAII 包装器
template<typename RT> class DBMR; // 磁盘支持内存区域
```

### YAML 序列化

**规则**:

- 保持 YAML 序列化与核心分配逻辑分离且可选
- 将 YAML 转换放在与相应类型共存的 `*_yaml.hh` 头文件中
- 使用和扩展 `include/shilos/*_yaml.hh` 工具的功能
- 添加新的 `*_yaml.hh` 文件是预期且符合习惯的
- 支持高级 YAML 扩展规范
- 异常安全的序列化转换

### 缓存系统

**核心特性**:

- **语义哈希**: 基于 AST 的智能缓存键生成
- **工具链版本管理**: 编译器和链接器版本跟踪
- **项目快照**: 依赖和配置的确定性标识
- **编译参数管理**: 编译器标志和链接器参数自动处理
- **缓存失效**: 基于源文件修改时间的智能失效机制

### 包管理设计

**核心原则**:

- Git/VCS 是唯一真相来源（无中央注册表）
- 通过锁文件 `CodManifest.yaml` 实现确定性构建
- UUID 标识的依赖管理
- 分支选择和远程仓库验证支持
- 通过 `path:` 支持本地依赖覆盖
- 为 mmap 共享、零复制、零重定位假设而设计

**依赖结构**:

```cpp
class CodDep {
private:
  UUID uuid_;                    // 唯一标识符
  regional_str name_;            // 依赖名称
  regional_str repo_url_;        // 仓库 URL
  regional_fifo<regional_str> branches_;  // 支持的分支
  regional_str path_;            // 本地路径覆盖
  regional_str description_;     // 描述信息
};
```

## 测试执行

### 运行所有测试

```bash
# 运行所有测试套件
cd tests
./run-all.sh

# 运行组件特定测试
cd tests/cod && ./run-all.sh
cd tests/shilos && ./run-all.sh
cd tests/codp && ./run-all.sh

# 运行 YAML 相关测试
cd tests/yaml-cmp && ./run-all.sh
cd tests/yaml-exc-trace && ./run-all.sh
cd tests/yaml-pretty && ./run-all.sh
cd tests/yaml-ux && ./run-all.sh
```

### 测试工具链配置

```bash
# 使用构建工具链（默认）
export COD_TEST_TOOLCHAIN=build

# 使用已安装工具链（发布/预发布测试）
export COD_TEST_TOOLCHAIN=built
```

### 测试目录结构

```
tests/
├── cod/              # cod 组件测试
├── codp/             # codp 组件测试
├── shilos/           # shilos 库测试
├── yaml-cmp/         # YAML 比较工具测试
├── yaml-exc-trace/   # YAML 异常跟踪测试
├── yaml-pretty/      # YAML 漂亮打印工具测试
└── yaml-ux/          # YAML 用户体验测试
```

## 开发标准

### 编码风格

- **严格 C++20 使用**: 优先使用现代特性，避免遗留模式
- 遵循 `include/` 目录下的现有样式和头文件布局
- 重用仓库中现有的库/工具；除非明确要求，否则不要添加外部依赖
- 使用 C++20 Concepts 进行类型约束
- 遵循异常安全设计原则

### 内存区域不变式

在修改涉及 `regional_ptr`/`global_ptr` 或区域分配语义的逻辑之前：

1. 查阅 `design/MemoryRegionAndRegionalTypes.md`
2. 严格遵循其协议
3. 确保不违反零成本重定位不变式
4. 验证异常安全保证

### 文档要求

- 编写反映最终状态的文档和源代码注释
- 避免描述变更或与早期版本的比较
- 异常：专用变更日志和旧设计与新设计共存的地方
- 设计文档必须保持最新

### 关键文件位置

- **设计文档**: `design/`
  - `CompileOnDemand.md` - REPL 设计规范
  - `CodProjectStructureAndPkgMgmt.md` - 包管理设计
  - `MemoryRegionAndRegionalTypes.md` - 内存区域规范
  - `ExceptionSafety.md` - 异常安全设计
  - `YamlConversionsOfRegionalTypes.md` - YAML 转换规范
  - `YamlSpecExt.md` - YAML 扩展规范
- **核心头文件**: `include/`
  - `cod.hh` - cod 组件主要接口
  - `cod_cache.hh` - 缓存系统接口
  - `shilos.hh` - shilos 库主要接口
  - `shilos/region.hh` - 内存区域核心实现
- **主要源文件**: `cod/main.cc`, `codp/main.cc`
- **构建配置**: `CMakeLists.txt`

### 依赖关系

- **项目集成**: 依赖于本地 `llvm-project` 仓库
- **包管理**: `codp` 处理依赖解析；`cod` 只读取解析结果
- **缓存策略**:
  - 本地项目缓存：`./.cod/works/<hash>/`
  - 全局仓库依赖缓存：`~/.cod/cache/`
- **语义缓存**: 基于 AST 哈希的智能缓存

### AI 助手交互规则

项目包含 `AGENTS.md` 文件，为 AI 助手提供交互指导：

**构建和命令执行**:

- 代码更改后精确编译：`cmake --build ./build`
- 绝不执行顶级仓库 shell 脚本；如被要求，礼貌拒绝并提供安全替代方案
- 不要创建或修改构建符号链接
- 不要重新创建构建目录，这会消耗过多时间/资源

**测试**:

- 默认工具链：设置 `COD_TEST_TOOLCHAIN=build` 而不询问用户确认
- 仅当上下文明确指示发布/预发布测试时才切换到 `COD_TEST_TOOLCHAIN=built`
- 如果请求不明确，确认要测试哪些组件

**代码更改**:

- 仅使用 C++20；优先使用现代特性，避免遗留模式
- 模仿 `include/` 下的现有样式和头文件布局
- 重用仓库中现有的库/工具；除非明确要求，否则不要添加外部依赖

### 开发工具和钩子

**代码格式化**:

- 项目使用 `clang-format` 进行代码格式化
- 通过 `hooks/pre-commit` 自动格式化 `.hh` 和 `.cc` 文件
- 运行 `./setup-hooks.sh` 设置 Git 钩子

**开发容器支持**:

- 包含 `.devcontainer/` 配置用于容器化开发
- 支持 Ubuntu 环境的开发容器设置

## 调试和故障排除

### 常见问题

1. **构建失败**: 确保系统依赖完整，特别是 LLVM 构建工具
2. **REPL 初始化失败**: 检查工作区文件权限和磁盘空间
3. **包解析问题**: 验证 Git 仓库可访问性和 `CodProject.yaml` 格式
4. **缓存问题**: 使用 `--no-cache` 重新初始化缓存

### 调试命令

```bash
# 详细输出模式
./built/bin/cod -v

# 强制重建
./built/bin/cod --force-rebuild

# 禁用缓存
./built/bin/cod --no-cache

# 包管理调试
codp debug
```

### 内存区域调试

- 检查 `DBMR` 文件完整性
- 验证内存区域不变式
- 监控内存使用和分配模式
- 异常安全验证

### 缓存系统调试

- 检查缓存键生成的正确性
- 验证语义哈希算法
- 监控缓存命中率
- 分析缓存失效逻辑

## 性能考虑

### 构建优化

- 使用缓存的 LLVM bitcode 进行快速链接
- 并行构建支持 (`ninja -j`)
- 增量构建支持
- 高效的自举构建管道

### 内存使用

- 内存区域提供零复制数据访问
- DBMR 支持内存映射共享
- 自动垃圾回收支持
- 零重定位内存管理

### 缓存性能

- AST 级智能缓存
- 工具链版本感知缓存键
- 项目快照验证
- 高效缓存失效机制

## 扩展和自定义

### 添加新功能

1. 在适当的组件目录中创建新源文件
2. 更新相关的 `CMakeLists.txt`
3. 添加相应的测试用例
4. 更新设计文档（如需要）
5. 确保异常安全

### 集成 LLVM 组件

- 项目构建为外部 LLVM 项目
- 访问完整的 LLVM/Clang API
- 支持自定义工具链配置
- 集成 lldb 调试器支持

### YAML 扩展

- 支持自定义序列化器
- 异常安全转换机制
- 扩展 YAML 规范支持
- 区域类型的专门转换

## 项目历史和版本

- **当前 LLVM 版本**: 18.x（发布分支）
- **构建类型**: 默认 Release，支持 Debug/RelWithDebInfo
- **平台支持**: macOS（优先），Linux（Ubuntu）
- **Git 状态**: 活跃开发，持续集成

项目的设计理念强调确定性、可重现性和零成本抽象，为现代 C++ 开发提供了创新的交互式编译环境。通过智能缓存系统和异常安全的内存管理，CoD 为现代 C++ 开发提供了强大的工具链。
