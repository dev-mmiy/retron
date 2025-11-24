#!/bin/bash
echo "Building test single sector Stage2 loader..."

# セクタ数1テスト用第2段階ELFローダーのビルド
echo "Building test single sector Stage2 ELF loader..."
nasm -f bin boot/test_single_sector_stage2_elf_loader.asm -o retron-test-single-sector-stage2-elf-loader.bin
if [ $? -eq 0 ]; then
    echo "Test single sector Stage2 ELF loader built successfully: retron-test-single-sector-stage2-elf-loader.bin"
    ls -l retron-test-single-sector-stage2-elf-loader.bin
else
    echo "Error building test single sector Stage2 ELF loader"
    exit 1
fi

