//! no_std環境用のプリルード

// 基本的な型の再エクスポート
pub use core::option::Option;
pub use core::option::Option::{None, Some};
pub use core::result::Result;
pub use core::result::Result::{Err, Ok};

// 基本的なマクロ
pub use core::concat;
pub use core::write;

// 基本的なトレイト
pub use core::cmp::{PartialEq, PartialOrd};
pub use core::default::Default;
pub use core::iter::Iterator;
pub use core::marker::{Send, Sized, Sync};

// アロケーション関連
pub use core::alloc::{GlobalAlloc, Layout};

// 文字列関連
pub use core::str::FromStr;
