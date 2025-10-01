# PS/2 X68 Keyboard Converter
## 概要
PS/2キーボードをX68000に接続する変換器です。
2008年ごろに作りました。動作無保証です。

## Files
・keyb.c : ソースコード。

・default/ps2x68k.hex : ファームウェアです。

・ps2x68k.aps : AVR Studio 4 + WinAVRプロジェクトファイルです。

・ps2x68k_schematics.png : 回路図です。

・batファイル : AVRマイコンに書き込むためのコマンドです。

## パーツ
・AVRマイコン ATtiny26L

・mini-DIN7pinコネクタ（オス）

・mini-DIN6pinコネクタ（メス）

・抵抗とLED

## ピンアサイン
![Image](https://github.com/user-attachments/assets/18d37756-170f-4d2e-a72b-399d8f2d6552)

この例ではX68000側のコネクタはmini-DIN7pinではなく、mini-DIN8pinを使用しています。
