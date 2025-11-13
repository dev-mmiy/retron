# ReTron OS - ライセンスと参考実装

> **ドキュメント種別**: ライセンス・参考実装ドキュメント  
> **対象読者**: すべての開発者  
> **関連ドキュメント**: [ARCHITECTURE.md](ARCHITECTURE.md), [README.md](README.md)

## 概要

ReTron OSは商用利用を想定しているため、独自実装を開発します。
このドキュメントでは、ファイルシステムやネットワークスタックなどの実装において参考にできる既存実装とそのライセンスについて説明します。

## ライセンス要件

ReTron OSは商用利用可能なライセンスを採用する予定です。そのため、以下の要件を満たす実装を選択します：

- ✅ **商用利用可能**: プロプライエタリ製品に組み込み可能
- ✅ **改変・再配布自由**: ソースコードの改変と再配布が可能
- ✅ **派生物のライセンス制約なし**: 派生物を独自ライセンスで配布可能
- ❌ **GPL系は避ける**: GPL/LGPLは派生物にもGPL適用が必要なため不適切

## 参考実装

### 1. ファイルシステム実装

#### **FreeBSD ファイルシステム実装**（参考）

**ライセンス**: BSD 2-Clause License（修正BSDライセンス）

**特徴**:
- 商用利用・改変・再配布が完全に自由
- 高品質な実装（UFS、ZFS、FAT32など）
- 組み込みシステムでも実績あり
- 設計の参考として活用可能

**注意**: ReTron OSでは統合は行わず、設計の参考として活用します。

**参考実装**:
- UFS/UFS2（Unix File System）- 設計の参考として
- FAT32（MS-DOS互換）- 設計の参考として
- ext2/ext3 - 設計の参考として
- 仮想ファイルシステム（VFS）フレームワーク

**ライセンス表記例**:
```
Copyright (c) [year] [copyright holder]
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
```

#### ✅ **OpenBSD ファイルシステム実装**

**ライセンス**: BSD License（ISC Licenseに近い）

**特徴**:
- セキュリティ重視の実装
- 商用利用可能
- シンプルで理解しやすいコード

#### ✅ **NetBSD ファイルシステム実装**

**ライセンス**: BSD 2-Clause License

**特徴**:
- 移植性が高い
- 組み込みシステム向けの最適化
- 商用利用可能

#### ❌ **Linux ファイルシステム実装**（非推奨）

**ライセンス**: GPLv2

**問題点**:
- 派生物にもGPLv2を適用する必要がある
- 商用プロプライエタリ製品への組み込みが困難
- ソースコード公開が義務付けられる可能性

### 2. ネットワークスタック

#### **FreeBSD ネットワークスタック**（参考）

**ライセンス**: BSD 2-Clause License

**特徴**:
- 高品質なTCP/IP実装
- IPv4/IPv6完全サポート
- 各種プロトコル実装（TCP、UDP、ICMP、ARPなど）
- パフォーマンス最適化済み
- 設計の参考として活用可能

**注意**: ReTron OSでは統合は行わず、設計の参考として活用します。

**利用可能な実装**:
- TCP/IPスタック
- ソケットAPI実装
- ネットワークドライバフレームワーク
- ファイアウォール機能

#### ✅ **lwIP (Lightweight IP)**（組み込み向け）

**ライセンス**: BSD 3-Clause License

**特徴**:
- 軽量で組み込みシステム向け
- TCP/IPスタックの最小実装
- メモリフットプリントが小さい
- 商用利用可能

**注意点**:
- BSD 3-Clause Licenseは広告条項を含む（3条項BSD）
- ライセンス表記が必要

#### ✅ **OpenBSD ネットワークスタック**

**ライセンス**: BSD License

**特徴**:
- セキュリティ重視
- 商用利用可能

#### ❌ **Linux ネットワークスタック**（非推奨）

**ライセンス**: GPLv2

**問題点**:
- 商用利用に制約あり

### 3. C標準ライブラリ（libc）

#### ✅ **musl libc**（推奨）

**ライセンス**: MIT License

**特徴**:
- 軽量で組み込み向け
- 標準Cライブラリ
- 静的リンクに最適
- 商用利用可能（制約なし）

**利用可能な機能**:
- 標準Cライブラリ関数
- ファイルシステム操作
- ネットワークソケット

#### ✅ **newlib**

**ライセンス**: 複数ライセンス（BSD、MIT、LGPLなど）

**特徴**:
- 組み込みシステム向け
- 多くの実装がBSD/MITライセンス
- 商用利用可能（実装による）

**注意点**:
- 一部の実装はLGPLの可能性があるため、確認が必要

#### ⚠️ **glibc**（注意が必要）

**ライセンス**: LGPL 2.1+

**問題点**:
- 動的リンク時は比較的自由
- 静的リンク時は制約あり（オブジェクトファイルの提供が必要）
- 商用利用は可能だが、条件を満たす必要がある

### 4. その他のコンポーネント

#### **FreeBSD システムライブラリ**（参考）

**ライセンス**: BSD 2-Clause License

**参考実装**:
- プロセス管理
- メモリ管理ユーティリティ
- デバイス管理
- システムコール実装

**注意**: ReTron OSでは統合は行わず、設計の参考として活用します。

## 実装アプローチ

### ファイルシステム層

1. **VFSフレームワーク**: 独自実装（既存実装の設計を参考に）
2. **ファイルシステム実装**: ReTron専用ファイルシステムを独自実装
3. **互換ファイルシステム**: 必要に応じてFAT32、ext2などを実装

### ネットワーク層

1. **TCP/IPスタック**: 独自実装またはlwIPをベースに統合
2. **ソケットAPI**: ネットワークソケット実装

### libc実装

1. **musl libc**: 軽量で組み込み向け、MITライセンス
2. **必要に応じて拡張**: ReTron固有の機能を追加

## ライセンス表記の義務

### BSD 2-Clause License

以下の表記をソースコードに含める必要があります：

```
Copyright (c) [year] [copyright holder]
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
```

### MIT License（musl libcなど）

以下の表記をソースコードに含める必要があります：

```
Copyright (c) [year] [copyright holder]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## 実装戦略（参考）

### Phase 1: 基盤構築

1. **VFSフレームワーク**を独自実装（既存実装の設計を参考に）
2. 基本的なファイルシステム操作を実装

### Phase 2: ネットワーク実装

1. **TCP/IPスタック**を独自実装または**lwIP**をベースに統合
2. ソケットAPIを実装

### Phase 3: 最適化

1. ReTronの要件に合わせて最適化
2. 不要な機能の削除
3. パフォーマンスチューニング

## 注意事項

1. **ライセンス表記**: 使用する実装のライセンス表記を必ず含める
2. **著作権表示**: 元の著作権表示を保持する
3. **変更履歴**: 大幅な変更を行った場合は記録を残す
4. **法的確認**: 商用利用前に法的な確認を推奨

## 参考リンク

- [FreeBSD License](https://www.freebsd.org/copyright/freebsd-license.html)
- [OpenBSD License](https://www.openbsd.org/policy.html)
- [NetBSD License](https://www.netbsd.org/about/redistribution.html)
- [musl libc License](https://git.musl-libc.org/cgit/musl/tree/COPYRIGHT)
- [lwIP License](https://savannah.nongnu.org/projects/lwip/)

