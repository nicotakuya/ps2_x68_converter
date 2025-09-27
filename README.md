# PS/2 X68 Keyboard Converter
## 概要
PS/2キーボードをX68000に接続する変換器です。
2008年ごろに作りました。動作無保証です。

## Files
・keyb.cがソースコード。

・default/ps2x68k.hexがファームウェアです。

・apsの拡張子がプロジェクトファイルです。2008年当時、AVR Studio 4 + WinAVRで開発していました。

・ps2x68k_schematics.pngが回路図です。

・batファイルはAVRマイコンに書き込むためのコマンドです。

## パーツ
・AVRマイコン ATtiny26L

・mini-DIN7pinコネクタ（オス）

・mini-DIN6pinコネクタ（メス）

・抵抗とLED

## ピンアサイン
![Image](https://github.com/user-attachments/assets/18d37756-170f-4d2e-a72b-399d8f2d6552)

この例ではX68000側のコネクタはmini-DIN7pinではなく、mini-DIN8pinを使用しています。
