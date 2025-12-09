//! PVH boot プロトコル
//!
//! QEMU の 64-bit カーネル直接起動に必要な ELF Note

/// PVH ELF Note 構造体
#[repr(C)]
struct PvhNote {
    name_size: u32,
    desc_size: u32,
    note_type: u32,
    name: [u8; 4],
    entry: u64,
}

/// PVH ELF Note（.note.PVH セクションに配置）
#[used]
#[link_section = ".note.PVH"]
static PVH_NOTE: PvhNote = PvhNote {
    name_size: 4,              // "Xen" のサイズ
    desc_size: 8,              // entry point のサイズ (64-bit)
    note_type: 18,             // XEN_ELFNOTE_PHYS32_ENTRY (0x12)
    name: *b"Xen\0",           // 名前（null終端）
    entry: 0x0000000000100493, // エントリーポイント（kernel_main）
};
