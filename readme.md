# ビーコン受信機
MbedのBLE用サンプルプログラム「BLE_EddystoneObserverの拡張版」でアドバタイズのうちビーコンの信号
と思しきものだけを処理(出力)するプログラム

元のサンプルプログラムのURL
https://github.com/ARMmbed/mbed-os-example-ble/tree/master/BLE_EddystoneObserver

## 対応しているビーコン
- EDDYSTONE
- IBEACON
- オムロン環境センサ(ビーコンモード)
- AltBeacon
- UCODE

ただし，ほとんどのビーコンは現物を持っていないので，規格書等を見てコードを書いている都合上，正しく動作しないものも
あるかもしれない(特に，EDDYSTONEのURL以外，AltBeacon, UCODE)．


## 動作確認済みプラットフォーム
mbedで動作するnordicのnRF5xシリーズなら動作すると思うが，手元で確認したもの以下のもののみ．
- スイッチサイエンスmbed TY51822

## 使い方
オンラインIDEの「URL指定インポート」機能を使って，取り込んでください．
