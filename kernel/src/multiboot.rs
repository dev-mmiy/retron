//! Multiboot ヘッダー
//!
//! QEMU/GRUBでブート可能にするためのMultiboot仕様準拠ヘッダー

/// Multiboot マジックナンバー
const MULTIBOOT_MAGIC: u32 = 0x1BADB002;

/// Multiboot フラグ
/// - bit 0: アラインメント（4KBページ境界にモジュールをアライン）
/// - bit 1: メモリ情報要求
const MULTIBOOT_FLAGS: u32 = 0x0;

/// Multiboot チェックサム
const MULTIBOOT_CHECKSUM: u32 = -(MULTIBOOT_MAGIC as i32 + MULTIBOOT_FLAGS as i32) as u32;

/// Multiboot ヘッダー構造体
#[repr(C, align(4))]
struct MultibootHeader {
    magic: u32,
    flags: u32,
    checksum: u32,
}

/// Multiboot ヘッダー（.multiboot セクションに配置）
#[used]
#[link_section = ".multiboot"]
static MULTIBOOT_HEADER: MultibootHeader = MultibootHeader {
    magic: MULTIBOOT_MAGIC,
    flags: MULTIBOOT_FLAGS,
    checksum: MULTIBOOT_CHECKSUM,
};
