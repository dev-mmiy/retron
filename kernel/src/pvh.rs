//! PVH boot プロトコル
//!
//! QEMU の 64-bit カーネル直接起動に必要な ELF Note

/// PVH ELF Note 構造体
/// XEN_ELFNOTE_PHYS32_ENTRY (type 18) は 32-bit の物理アドレスを要求
#[repr(C)]
struct PvhNote {
    name_size: u32,
    desc_size: u32,
    note_type: u32,
    name: [u8; 4],
    entry: u32,      // 32-bit physical entry point
}

/// PVH ELF Note（.note.PVH セクションに配置）
#[used]
#[link_section = ".note.PVH"]
static PVH_NOTE: PvhNote = PvhNote {
    name_size: 4,              // "Xen" のサイズ
    desc_size: 4,              // entry point のサイズ (32-bit)
    note_type: 18,             // XEN_ELFNOTE_PHYS32_ENTRY (0x12)
    name: *b"Xen\0",           // 名前（null終端）
    entry: 0x00101000,         // エントリーポイント（kernel_main at .text start）
};
