# T-Kernel ビルド設定ガイド

> **ドキュメント種別**: T-Kernelビルド設定ガイド  
> **対象読者**: 開発者  
> **関連ドキュメント**: [TKERNEL_ACQUISITION.md](TKERNEL_ACQUISITION.md), [DEVELOPMENT_ENVIRONMENT.md](DEVELOPMENT_ENVIRONMENT.md), [TKERNEL_ARCHITECTURE.md](TKERNEL_ARCHITECTURE.md)

## ドキュメント情報

| 項目 | 内容 |
|------|------|
| **ドキュメント名** | T-Kernel ビルド設定ガイド |
| **バージョン** | 1.0 |
| **作成日** | 2024年 |
| **最終更新日** | 2024年 |
| **作成者** | - |
| **承認者** | - |
| **ステータス** | 草案 |

---

## 概要

このドキュメントでは、ReTron OSで使用するT-Kernelのビルド設定について説明します。

**現在の状況**: T-Kernel 2.02.00には`tef_em1d`（ARM11コア）向けの設定のみが含まれています。ARM Generic AArch64向けの設定を新規に作成する必要があります。

**注意**: T-Kernelの仕様自体はアーキテクチャ非依存で、ARMv8-A（AArch64）への移植は技術的に可能です。ただし、公式の実装例は提供されていないため、独自に実装する必要があります。詳細は [TKERNEL_ARMV8_SUPPORT.md](TKERNEL_ARMV8_SUPPORT.md) を参照してください。

---

## 1. T-Kernelのビルドシステム構造

### 1.1 ビルドシステムの概要

T-Kernelは以下の構造でビルドシステムが構成されています：

```
tkernel_2/
├── etc/
│   ├── makerules                    # 共通ビルドルール
│   └── sysdepend/
│       └── $(TETYPE)_$(MACHINE)/
│           └── makerules.sysdepend  # プラットフォーム依存ビルドルール
├── config/
│   ├── src/
│   │   └── sysdepend/
│   │       └── $(TETYPE)_$(MACHINE)/
│   │           ├── SYSCONF          # システム設定
│   │           └── DEVCONF          # デバイス設定
│   └── build/
│       └── $(TETYPE)_$(MACHINE)/
│           └── Makefile
└── kernel/
    └── sysdepend/
        ├── cpu/
        │   └── $(CPU名)/            # CPU依存部
        └── device/
            └── $(機種名)/           # デバイス依存部
```

### 1.2 主要な環境変数

| 変数名 | 説明 | 例 |
|--------|------|-----|
| **BD** | 開発環境ベースディレクトリ | `/usr/local/te` |
| **MACHINE** | ターゲットCPU名 | `em1d`, `aarch64` |
| **TETYPE** | T-Engineタイプ | `tef`, `retron` |
| **GNU_BD** | GNUツールチェーンベースディレクトリ | `/usr/local/te` |
| **GNUARM_2** | ARM用GNUツールチェーン | `/usr/local/te/gnuarm_2` |
| **mode** | ビルドモード | `debug`（デバッグ）または空（リリース） |

### 1.3 機種名の命名規則

T-Kernelでは、機種名を以下の形式で表現します：

```
<機種名> = <ボード名>_<CPU名>
```

**例**:
- `tef_em1d`: T-Engine Forum Reference Board (ARM11コア)
- `retron_aarch64`: ReTron OS (ARM AArch64) ← 新規作成が必要

---

## 2. 現在のT-Kernel設定（tef_em1d）

### 2.1 既存設定の確認

現在のT-Kernelには以下の設定が含まれています：

- **機種名**: `tef_em1d`
- **CPU名**: `em1d` (EMMA Mobile 1-D, ARM11コア)
- **ボード名**: `tef` (T-Engine Forum Reference Board)
- **アーキテクチャ**: ARMv6 (ARM1176JZF-S)

### 2.2 ビルド設定ファイル

#### 2.2.1 プラットフォーム依存ビルドルール

**ファイル**: `etc/sysdepend/tef_em1d/makerules.sysdepend`

**主要な設定**:
```makefile
# CPUタイプフラグ
_CPUTYPE_FLAGS = -mcpu=arm1176jzf-s -msoft-float -mfpu=vfp
_CPUTYPE_FLAGS_AS = -mcpu=arm1176jzf-s -mfpu=softvfp

# コンパイラ
CC := $(GNUARM_2)/bin/gcc4arm
```

#### 2.2.2 システム設定

**ファイル**: `config/src/sysdepend/tef_em1d/SYSCONF`

**主要な設定**:
- タスク数、セマフォ数などのリソース上限
- システムスタックサイズ
- タイマー間隔

#### 2.2.3 デバイス設定

**ファイル**: `config/src/sysdepend/tef_em1d/DEVCONF`

**主要な設定**:
- デバッグモードの有効/無効

---

## 3. ARM AArch64向け設定の作成方針

### 3.1 必要な作業

ARM Generic AArch64向けの設定を作成するには、以下の作業が必要です：

1. **プラットフォーム依存ビルドルールの作成**
   - `etc/sysdepend/retron_aarch64/makerules.sysdepend` の作成

2. **システム設定の作成**
   - `config/src/sysdepend/retron_aarch64/SYSCONF` の作成
   - `config/src/sysdepend/retron_aarch64/DEVCONF` の作成

3. **CPU依存部の作成**
   - `kernel/sysdepend/cpu/aarch64/` ディレクトリの作成
   - CPU依存のソースコードの実装

4. **デバイス依存部の作成**
   - `kernel/sysdepend/device/retron_aarch64/` ディレクトリの作成
   - QEMU virt machine向けのデバイス設定

5. **ビルドディレクトリの作成**
   - `config/build/retron_aarch64/Makefile` の作成
   - 各コンポーネントの`build/retron_aarch64/`ディレクトリの作成

### 3.2 機種名の決定

ReTron OS向けの機種名を以下のように定義します：

- **機種名**: `retron_aarch64`
- **CPU名**: `aarch64`
- **ボード名**: `retron`
- **TETYPE**: `retron`

### 3.3 ビルドツールチェーンの設定

ARM AArch64向けには、標準的なGCCクロスコンパイルツールチェーンを使用します：

```makefile
# コンパイラ設定
CC := aarch64-linux-gnu-gcc
AS := aarch64-linux-gnu-as
LD := aarch64-linux-gnu-ld
AR := aarch64-linux-gnu-ar

# CPUタイプフラグ
_CPUTYPE_FLAGS = -march=armv8-a -mtune=cortex-a53
_CPUTYPE_FLAGS_AS = -march=armv8-a
```

---

## 4. ビルド設定ファイルの作成

### 4.1 プラットフォーム依存ビルドルール

**ファイル**: `etc/sysdepend/retron_aarch64/makerules.sysdepend`

```makefile
#
# ----------------------------------------------------------------------
#     T-Kernel 2.0 Software Package
#
#     Copyright 2011 by Ken Sakamura.
#     This software is distributed under the latest version of T-License 2.x.
# ----------------------------------------------------------------------
#
#     Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
#     Modified by TRON Forum(http://www.tron.org/) at 2015/06/01.
#     Modified for ReTron OS (ARM AArch64) at 2024/XX/XX.
#
# ----------------------------------------------------------------------
#

#
#	makerules
#		for GNU make
#
#	default rules - system-dependent (ReTron OS ARM AArch64)
#
#	MACHINE	target CPU
#		aarch64	: ARM AArch64 (ARMv8-A)
#
#	TETYPE	target T-Engine type
#		retron	: ReTron OS
#
#	GNU_BD	GNU development tool base directory
#

# check environmental variables
ifndef GNU_BD
  $(error 'GNU_BD' is not defined)
endif

# command path
PATH = .
ifneq ($(filter "$(origin CROSS_COMPILE)", $(TOOL_ORIGIN)), )
  PATH := $(PATH):$(dir $(CROSS_COMPILE))
endif
ifneq ($(filter Linux-%, $(CROSS_ARCH)), )
  PATH := $(PATH):/usr/local/bin:/bin:/usr/bin
endif

# ----- ReTron OS (ARM AArch64) ----------------------------

# GCC environment
TARGET_ARCH =

# target type
_CPUTYPE_FLAGS    = -march=armv8-a -mtune=cortex-a53
_CPUTYPE_FLAGS_AS = -march=armv8-a
_TE_SYSTEM_NAME_ = _RETRON_AARCH64_

# code set
_CODESET_FLAGS =

### build option ###
CFLAGS +=
CPPFLAGS +=
ASFLAGS +=
LDFLAGS +=

### C  ###
ifdef CROSS_COMPILE
  CC := $(CROSS_COMPILE)gcc
else
  CC := aarch64-linux-gnu-gcc
endif

OUTPUT_OPTION = -o $@
ifeq ($(mode), debug)
  CFLAGS += -g
  CPPFLAGS += $(HEADER:%=-I%) -D$(_TE_SYSTEM_NAME_) -DDEBUG
else
  CFLAGS += -O2
  CPPFLAGS += $(HEADER:%=-I%) -D$(_TE_SYSTEM_NAME_)
endif

CFLAGS += $(_CPUTYPE_FLAGS) $(_CODESET_FLAGS) -ffreestanding -fno-common

CFLAGS_WARNING      = -Wall -Wno-pointer-sign
CFLAGS_WARNING_FULL = -pedantic -W -Wall

COMPILE.c = $(CC) $(TARGET_ARCH) $(CFLAGS) $(CPPFLAGS) -c
LINK.c = $(CC) $(TARGET_ARCH) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)

%.o: %.c
	$(COMPILE.c) $< $(OUTPUT_OPTION)

%.s: %.c
	$(CC) $(TARGET_ARCH) $(CFLAGS) $(CPPFLAGS) -S $<

%.i: %.c
	$(CC) $(TARGET_ARCH) $(CFLAGS) $(CPPFLAGS) -E $< $(OUTPUT_OPTION)

### C++ ###
ifdef CROSS_COMPILE
  CXX := $(CROSS_COMPILE)g++
else
  CXX := aarch64-linux-gnu-g++
endif

COMPILE.cc = $(CXX) $(TARGET_ARCH) $(CXXFLAGS) $(CPPFLAGS) -c
LINK.cc = $(CXX) $(TARGET_ARCH) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS)

%.o: %.cpp
	$(COMPILE.cc) $< $(OUTPUT_OPTION)

%.s: %.cpp
	$(CXX) $(TARGET_ARCH) $(CXXFLAGS) $(CPPFLAGS) -S $<

### Assembler ###
ifdef CROSS_COMPILE
  AS := $(CROSS_COMPILE)as
else
  AS := aarch64-linux-gnu-as
endif

ASFLAGS += $(_CPUTYPE_FLAGS_AS)

COMPILE.S = $(CC) $(TARGET_ARCH) $(ASFLAGS) $(CPPFLAGS) -c

%.o: %.S
	$(COMPILE.S) $< $(OUTPUT_OPTION)

%.s: %.S
	$(CC) $(TARGET_ARCH) $(ASFLAGS) $(CPPFLAGS) -E $< $(OUTPUT_OPTION)

### Linker ###
ifdef CROSS_COMPILE
  LD := $(CROSS_COMPILE)ld
else
  LD := aarch64-linux-gnu-ld
endif

LINK.o = $(CC) $(TARGET_ARCH) $(LDFLAGS) $(LDFLAGS2) $(START_ADR)
LINK_R.o = $(CC) $(TARGET_ARCH) $(LDFLAGS) -r -nostdlib
LINK_A.o = $(CC) $(TARGET_ARCH) $(LDFLAGS) -r
LOCATE.o = $(CC) $(TARGET_ARCH) $(LDFLAGS) -nostdlib $(LDFLAGS3) $(START_ADR)

### Archive ###
ifdef CROSS_COMPILE
  AR := $(CROSS_COMPILE)ar
  RANLIB := $(CROSS_COMPILE)ranlib
else
  AR := aarch64-linux-gnu-ar
  RANLIB := aarch64-linux-gnu-ranlib
endif

NM = $(dir $(AR))nm

### Others ###
ifdef CROSS_COMPILE
  STRIP := $(CROSS_COMPILE)strip --strip-unneeded
  OBJCOPY := $(CROSS_COMPILE)objcopy
else
  STRIP := aarch64-linux-gnu-strip --strip-unneeded
  OBJCOPY := aarch64-linux-gnu-objcopy
endif
```

### 4.2 システム設定

**ファイル**: `config/src/sysdepend/retron_aarch64/SYSCONF`

```makefile
#
# ----------------------------------------------------------------------
#     T-Kernel 2.0 Software Package
#
#     Copyright 2011 by Ken Sakamura.
#     This software is distributed under the latest version of T-License 2.x.
# ----------------------------------------------------------------------
#
#     Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
#     Modified by TRON Forum(http://www.tron.org/) at 2015/06/01.
#     Modified for ReTron OS (ARM AArch64) at 2024/XX/XX.
#
# ----------------------------------------------------------------------
#

#
#	SYSCONF (ReTron OS ARM AArch64)
#
#	System configuration
#

#
#	Product information
#
TSysName	ReTron OS	# System name

#
#	Kernel version information for tk_ref_ver(T_RVER*)
#
Maker		0x0000		# = "T-Engine Forum"
ProductID	0x0000		# Kernel Identifier
SpecVer		0x7201		# = "T-Kernel" + "Ver 2.01"
ProductVer	0x0202		# Product Version "Ver 2.02"
ProductNo	0x0000 0x0000 0x0000 0x0000
				# Product Number [0]-[3]

#
#	T-Kernel/OS
#
TMaxTskId	150	# Maximum task ID
TMaxSemId	100	# Maximum semaphore ID
TMaxFlgId	100	# Maximum event flag ID
TMaxMbxId	20	# Maximum mail box ID
TMaxMtxId	100	# Maximum mutex ID
TMaxMbfId	20	# Maximum message buffer ID
TMaxPorId	50	# Maximum rendezvous port ID
TMaxMpfId	10	# Maximum fixed size memory pool ID
TMaxMplId	10	# Maximum variable size memory pool ID
TMaxCycId	20	# Maximum cyclic handler ID
TMaxAlmId	40	# Maximum alarm handler ID
TMaxResId	60	# Maximum resource group ID
TMaxSsyId	50	# Maximum sub system ID
TMaxSsyPri	16	# Maximum sub system priority

TSysStkSz	2048	# Default system stack size (byte)
TSVCLimit	1	# SVC protection level
TTimPeriod	10	# Timer interval (msec)

#
#	T-Kernel/SM
#
TMaxRegDev	32	# Maximum number of devices registration
TMaxOpnDev	64	# Maximum number of devices open
TMaxReqDev	64	# Maximum number of device requests
TDEvtMbfSz	1024 64	# Event notification message buffer size (byte),
			# Maximum length of message (byte)

#
#	Task Event(1-8)
#
TEV_MsgEvt	1	# Message management : Receive message
TEV_MsgBrk	2	# Message management : Release of an message waiting state
TEV_GDI		3	# GDI interface
TEV_FFLock	4	# Release of an FIFO lock waiting state

#
#	Segment manager
#
RealMemEnd	0x40000000	# RAM bottom address (logical address)
				# 注意: QEMU virt machineのメモリマップに合わせて調整が必要

#
#	Exception/Interrupt stack
#
AbtStkSz	64		# Abort(MMU)
UndStkSz	64		# Undefined instruction
IrqStkSz	512		# IRQ interrupt
FiqStkSz	128		# FIQ interrupt (ARM AArch64では使用しない可能性あり)
```

### 4.3 デバイス設定

**ファイル**: `config/src/sysdepend/retron_aarch64/DEVCONF`

```makefile
#
# ----------------------------------------------------------------------
#     T-Kernel 2.0 Software Package
#
#     Copyright 2011 by Ken Sakamura.
#     This software is distributed under the latest version of T-License 2.x.
# ----------------------------------------------------------------------
#
#     Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
#     Modified by TRON Forum(http://www.tron.org/) at 2015/06/01.
#     Modified for ReTron OS (ARM AArch64) at 2024/XX/XX.
#
# ----------------------------------------------------------------------
#

#
#	DEVCONF (ReTron OS ARM AArch64)
#
#	Device configuration
#

# Debug mode (1:debug mode, 0:normal mode)
DEBUGMODE	1
```

### 4.4 ビルドディレクトリのMakefile

**ファイル**: `config/build/retron_aarch64/Makefile`

```makefile
#
# ----------------------------------------------------------------------
#     T-Kernel 2.0 Software Package
#
#     Copyright 2011 by Ken Sakamura.
#     This software is distributed under the latest version of T-License 2.x.
# ----------------------------------------------------------------------
#
#     Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
#     Modified by TRON Forum(http://www.tron.org/) at 2015/06/01.
#     Modified for ReTron OS (ARM AArch64) at 2024/XX/XX.
#
# ----------------------------------------------------------------------
#

#
#	Makefile (config/ReTron OS ARM AArch64)
#
#	RomInfo / SYSCONF / DEVCONF
#

# T-Engine type
MACHINE = aarch64
TETYPE = retron

# default rules
include ../../../etc/makerules

# include main makefile (common description)
include ../../src/Makefile.common
```

---

## 5. CPU依存部の実装

### 5.1 必要な実装

ARM AArch64向けには、以下のCPU依存部を実装する必要があります：

1. **CPU初期化**
   - `kernel/sysdepend/cpu/aarch64/cpu_startup.S` - CPU起動コード
   - `kernel/sysdepend/cpu/aarch64/cpu_task.c` - タスク管理
   - `kernel/sysdepend/cpu/aarch64/cpu_status.c` - CPU状態管理

2. **割り込み処理**
   - `kernel/sysdepend/cpu/aarch64/cpu_int.c` - 割り込み処理

3. **メモリ管理**
   - `kernel/sysdepend/cpu/aarch64/cpu_mem.c` - メモリ管理

4. **その他**
   - `kernel/sysdepend/cpu/aarch64/cpu_conf.h` - CPU設定
   - `kernel/sysdepend/cpu/aarch64/cpu_insn.h` - 命令定義

**注意**: これらの実装は、ARM AArch64の仕様に基づいて新規に作成する必要があります。既存の`em1d`（ARM11）の実装を参考にできますが、ARMv8-Aの仕様に合わせて大幅な変更が必要です。

---

## 6. デバイス依存部の実装

### 6.1 QEMU virt machine向けの設定

QEMU `virt` machine向けには、以下のデバイス依存部を実装する必要があります：

1. **タイマー**
   - `kernel/sysdepend/device/retron_aarch64/tkdev_timer.c` - タイマー処理

2. **UART**
   - `kernel/sysdepend/device/retron_aarch64/tkdev_uart.c` - UART通信（Hello World出力用）

3. **その他**
   - `kernel/sysdepend/device/retron_aarch64/tkdev_conf.h` - デバイス設定

**注意**: QEMU `virt` machineのデバイスマップに合わせて実装する必要があります。

---

## 7. ビルド手順

### 7.1 環境変数の設定

```bash
# 開発環境ベースディレクトリ
export BD=/home/miyasaka/source/retron/third_party/tkernel_2

# GNUツールチェーンベースディレクトリ
export GNU_BD=$BD

# クロスコンパイルツールチェーン
export CROSS_COMPILE=aarch64-linux-gnu-

# ターゲット設定
export MACHINE=aarch64
export TETYPE=retron
```

### 7.2 ビルドの実行

```bash
# configのビルド
cd $BD/config/build/retron_aarch64
make

# kernelのビルド
cd $BD/kernel/sysinit/build/retron_aarch64
make

cd $BD/kernel/tkernel/build/retron_aarch64
make

cd $BD/kernel/sysmgr/build/retron_aarch64
make
```

---

## 8. 実装の難易度と課題

### 8.1 実装の難易度

ARM AArch64向けの設定作成は、**中〜高難易度**です。

**理由**:
- CPU依存部の実装が必要（ARMv8-Aの仕様理解が必要）
- デバイス依存部の実装が必要（QEMU virt machineのデバイスマップ理解が必要）
- 既存の`em1d`実装を参考にできるが、大幅な変更が必要

### 8.2 主な課題

1. **CPU依存部の実装**
   - ARMv8-Aのアーキテクチャ理解
   - 例外処理、割り込み処理の実装
   - MMU設定の実装

2. **デバイス依存部の実装**
   - QEMU virt machineのデバイスマップ理解
   - UART、タイマーなどのデバイスドライバ実装

3. **ビルドシステムの理解**
   - T-Kernelのビルドシステムの理解
   - 各コンポーネントのビルド順序の理解

---

## 9. 推奨アプローチ

### 9.1 段階的アプローチ

1. **Phase 1: ビルド設定ファイルの作成**
   - プラットフォーム依存ビルドルールの作成
   - システム設定、デバイス設定の作成
   - ビルドディレクトリの作成

2. **Phase 2: 最小限のCPU依存部の実装**
   - CPU起動コードの実装
   - 基本的なタスク管理の実装

3. **Phase 3: デバイス依存部の実装**
   - UARTドライバの実装（Hello World出力用）
   - タイマードライバの実装

4. **Phase 4: 統合とテスト**
   - T-Kernelのビルド確認
   - QEMU上での動作確認

### 9.2 参考実装

既存の`tef_em1d`実装を参考にします：

- `kernel/sysdepend/cpu/em1d/` - CPU依存部の参考
- `kernel/sysdepend/device/tef_em1d/` - デバイス依存部の参考
- `etc/sysdepend/tef_em1d/makerules.sysdepend` - ビルドルールの参考

---

## 10. 次のステップ

1. **ビルド設定ファイルの作成**
   - プラットフォーム依存ビルドルールの作成
   - システム設定、デバイス設定の作成

2. **CPU依存部の実装**
   - ARM AArch64向けのCPU依存部の実装

3. **デバイス依存部の実装**
   - QEMU virt machine向けのデバイス依存部の実装

4. **ビルドとテスト**
   - T-Kernelのビルド確認
   - QEMU上での動作確認

---

## 11. 参照ドキュメント

- [TKERNEL_ACQUISITION.md](TKERNEL_ACQUISITION.md) - T-Kernel取得ガイド
- [DEVELOPMENT_ENVIRONMENT.md](DEVELOPMENT_ENVIRONMENT.md) - 開発環境構築ガイド
- [TKERNEL_ARCHITECTURE.md](TKERNEL_ARCHITECTURE.md) - T-Kernelベースアーキテクチャ設計
- T-Kernel 2.0 仕様書: [TEF020-S001-02.01.00_ja.pdf](https://www.tron.org/ja/wp-content/themes/dp-magjam/pdf/specifications/TEF020-S001-02.01.00_ja.pdf)
- T-Kernel 2.0 実装仕様書: `third_party/tkernel_2/doc/ja/impl-tef_em1d.txt`

---

## 変更履歴

| バージョン | 日付 | 変更内容 | 変更者 |
|-----------|------|----------|--------|
| 1.0 | 2024年 | 初版作成 | - |

