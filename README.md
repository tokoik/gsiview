地理院地図 3D を Oculus Rift DK1/DK2 で見てみる
===============================================

はじめに
--------
国土地理院が提供くださっている地理院地図 3D 上を，
Oculus Rift でフライスルーしながら眺めるプログラムです．
たいしたことないプログラムですが，
GLFW で Oculus を使うサンプルとして見ていただけると幸いです．

	Copyright (c) 2014, 2015, 2016 Kohe Tokoi
	
	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files (the
	"Software"), to deal in the Software without restriction, including
	without limitation the rights to use, copy, modify, merge, publish,
	distribute, sublicense, and/or sell copies of the Software, and to
	permit persons to whom the Software is furnished to do so, subject to
	the following conditions:
	
	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.
	
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
	LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
	OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
	WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

プログラムのビルド
------------------
プログラムのパッケージには，Windows 用の Visual Studio 2013 のプロジェクト，
Mac OS X 用の Xcode 5 のプロジェクト，および Linux 用の Makefile を含めています．
ビルドするには Oculus SDK 0.8beta，OpenCV 2.4.12，および GLFW 3.1.2 が必要です．
ただし Windows と Mac OS X については，パッケージに GLFW を含めています．
OpenCV は PNG のテクスチャファイルを読むためだけに使っています．
libpng や glpng を使うことも考えたのですが，めんどくさくなりました．

    Oculus SDK 0.8 に対応して DK2 も使えるようにしました．

### Windows
「プロジェクトのプロパティ」の「構成プロパティ」で，
「VC++ ディレクトリ」の「インクルードディレクトリ」に
OpenCV と Oculus SDK を展開した場所にある include ディレクトリ，
「ライブラリディレクトリ」に同じくライブラリファイルのあるディレクトリを追加してください．

* インクルードディレクトリ
    * <OpenCV を展開したディレクトリ>\opencv\build\include;<Oculus SDK を展開したディレクトリ>\OculusSDK\LibOVR\Include;
* ライブラリディレクトリ
    * <OpenCV を展開したディレクトリ>\opencv\build\x86\vc12\lib;<Oculus SDK を展開したディレクトリ>\OculusSDK\LibOVR\Lib\Win32;

立体視表示はフルスクリーンで行う必要がありますが，
Debug ビルドの時はフルスクリーンにしないようにしてあります．
実際に使う時は Release でビルドしてください．
あと，当たり前ですけど，OpenCV の DLL を置いている場所を環境変数 PATH に含めておいてください．

### Mac OS X
gsiview というターゲットの Build Settings の Search Paths という項目にある
Header Search Paths に OpenCV と Oculus SDK のヘッダファイルを置いたディレクトリを指定してください．
また，Library Search Paths に OpenCV と Oculus SDK のライブラリファイルを置いたディレクトリを指定してください．

    現在は Oculus の runtime / SDK が Mac をサポートしていないため，Oculus は使用できません．

デフォルトでは Debug ビルドになり，フルスクリーン表示を行いません．
Xcode で Release Build に切り替えるには，
Edit Scheme の Run gsiview.app の Info タブで Build Configuration を Release に切り替えてください．
デバッグビルドでフルスクリーン表示をしたい時は
Build Setting の Other C Flags の Debug のところに入っている -D_DEBUG を消してください．
-fno-rtti は Oculus SDK をリンクするときには必要です．

なお，Oculus SDK を使わない場合は，
Linking の項目にある Other Linker Flags に設定されている -lovr を削除してください．

余談ですが， OpenCV の 2.4.12 をHomeBrew でインストールする時は，
brew tap コマンドで homebrew/science を指定してください．
これ，ついこのあいだ知りました．

    $ brew tap homebrew/science
    $ brew install opencv

また，このパッケージをそのまま使う場合は必要がありませんが，
やはり HomeBrew で GLFW 3 をインストールする場合は，
brew tap コマンドで homebrew/versions を指定してください．GLFW 2 もインストールできます．

    $ brew tap homebrew/versions
    $ brew install glfw3
    $ brew install glfw2

Linux
-----
cmake とか使ってないので，Makefile 編集してください．

設定ファイル
------------
プログラムの細かな設定値は，config.h にまとめています．立体視の方式も，ここで選択できます．

    // 立体視の設定
    #define NONE          0                                 // 単眼視
    #define LINEBYLINE    1                                 // インターレース（未サポート）
    #define TOPANDBOTTOM  2                                 // 上下
    #define SIDEBYSIDE    3                                 // 左右
    #define QUADBUFFER    4                                 // クワッドバッファステレオ
    #define OCULUS        5                                 // Oculus Rift (HMD)
    
    // 立体視の方式
    #define STEREO        NONE

ここで OCURUS 以外を選んだ時は，Oculus SDK は必要ありません．
また LINEBYLINE は当初実装しようと思っていましたけど，結局していません．
これは自分の授業 で使っていた GL_POLYGON_STIPPLE が OpenGL の Core Profile では使えないことや，
ラインバイライン方式の液晶パネルを使ったモニタでも，
現在はサイドバイサイドやトップアンドボトム方式の映像信号に対応しているからです．
あと，QUADBUFFER は，そういえばテストしていません．

データの入手
------------
データは国土地理院の地理院地図 3D からダウンロードしてください．
こういうものを公開して頂いて本当に感謝しています．
自分で見たいところの地図を表示して，「WebGL 用ファイル」をダウンロードしてください．

ファイルは ZIP でまとめられています．
展開して，中に入っている dem.csv と texture.png というファイルを，
このプログラムのソースファイルと同じところに置いてください．

Xcode の場合は，さらにこれらを Build Phase の Copy Bundle Resources に追加してください．
これはドラッグアンドドロップでできます．

操作方法
--------
プログラムを実行すると，立体視表示 (フルスクリーン表示) の時は，
セカンダリディスプレイがあれば，そちらに表示しようとします．
視点 (カメラ) の移動は，マウスかゲームコントローラで行うことができます．

### マウスによる操作

マウスの左ボタンドラッグで水平方向の前後の速度と進行方向を制御します．
この制御の仕方は宿題の解答例そのまんまです．
マウスを前に進めると前進，後ろに進めると交代，左右で旋回します．

ホイールは上下移動で，横スクロールのできるホイールなら横移動 (カニ歩き) もできます．

右ボタンドラッグでカメラの向きを変えます．
起動直後，カメラは地形の上空にあり，真下を向いています．
右ボタンでマウスを前にドラッグすると，カメラが前に向きます．
h キーでカメラの向きを正面に戻します．

プログラムを起動すると正面に地形が見えます．
ホイールを回して地面に近づいたら，右ボタンで前にドラッグして，
カメラを前に向けてください．左ボタンドラッグでフライスルーできます．

### ゲームコントローラによる操作

ゲームコントローラは ELECOM のを使いました．他
のに対応する時は，ソース変更してください．アナログモードでないとうまく動きません．

左スティックで水平方向の前後の速度と進行方向を制御します．
右スティックで上下移動と横移動 (カニ歩き) ができます．
カメラの向きは 1，2，3，4 のボタンで変えられます．

プログラムを起動すると正面に地形が見えます．
左スティックを手前に引いて地面に近づいたら，
2 ボタンを押してカメラを前に向けてください．左スティックでフライスルーできます．

Oculus Rift を使うとき
Oculus Rift を使う時は，ヘッドトラッキングによりカメラの向きを変えます．
プログラムを起動すると地面が下にありますから下を向いてください．
降下して地面に近づいたら前を向いてください．

視差の調整
左右の画像が一致しなかったり，逆に頭が痛かったりした場合は，
左右の矢印キー←→で視差を調整してください．
また Oculus Rift では，上下の矢印キー↑↓で画像の拡大率を変えることができます．
