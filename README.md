地理院地図 3D を Oculus Rift DK1 で見てみる
===========================================

国土地理院が提供くださっている地理院地図 3D 上を
Oculus Rift でフライスルーしながら眺めるプログラムです

プログラムのビルド
------------------
プログラムのパッケージには，Windows 用の Visual Studio 2012 のプロジェクト，
Mac OS X 用の Xcode 5 のプロジェクト，および Linux 用の Makefile を含めています．
ビルドするには Oculus SDK 0.2.5c，OpenCV 2.4.9，および GLFW 3.0.4 が必要です．
ただし Windows と Mac OS X については，パッケージに GLFW を含めています．
OpenCV は PNG のテクスチャファイルを読むためだけに使っています．
libpng や glpng を使うことも考えたのですが，めんどくさくなりました．

###Windows
「プロジェクトのプロパティ」の「構成プロパティ」で，
「VC++ ディレクトリ」の「インクルードディレクトリ」に
OpenCV と Oculus SDK を展開した場所にある include ディレクトリ，
「ライブラリディレクトリ」に同じくライブラリファイルのあるディレクトリを追加してください．

*インクルードディレクトリ
    +<OpenCV を展開したディレクトリ>\opencv\build\include;<Oculus SDK を展開したディレクトリ>\OculusSDK\LibOVR\Include;
*ライブラリディレクトリ
    +<OpenCV を展開したディレクトリ>\opencv\build\x86\vc12\lib;<Oculus SDK を展開したディレクトリ>\OculusSDK\LibOVR\Lib\Win32;

立体視表示はフルスクリーンで行う必要がありますが，
Debug ビルドの時はフルスクリーンにしないようにしてあります．
実際に使う時は Release でビルドしてください．
あと，当たり前ですけど，OpenCV の DLL を置いている場所を環境変数 PATH に含めておいてください．

###Mac OS X
gsiview というターゲットの Build Settings の Search Paths という項目にある
Header Search Paths に OpenCV と Oculus SDK のヘッダファイルを置いたディレクトリを指定してください．
また，Library Search Paths に OpenCV と Oculus SDK のライブラリファイルを置いたディレクトリを指定してください．

デフォルトでは Debug ビルドになり，フルスクリーン表示を行いません．
Xcode で Release Build に切り替えるには，
Edit Scheme の Run gsiview.app の Info タブで Build Configuration を Release に切り替えてください．
デバッグビルドでフルスクリーン表示をしたい時は
Build Setting の Other C Flags の Debug のところに入っている -D_DEBUG を消してください．
-fno-rtti は Oculus SDK をリンクするときには必要です．

なお，Oculus SDK を使わない場合は，
Linking の項目にある Other Linker Flags に設定されている -lovr を削除してください．

余談ですが， OpenCV の 2.4.9 をHomeBrew でインストールする時は，
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

Oculus SDK の利用
-----------------
このプログラムで Oculus Rift に対応するために追加した部分には，
OCULUS という記号定数を検索すれば見つかると思います．
main.cpp では，OVR:: System::Init() を呼び出して初期化だけを行っています．
この引数に Log::ConfigureDefaultLog(LogMask_All) を指定すれば，標準出力にログが出力されます．

    #if STEREO == OCULUS
      // Oculus Rift (LibOVR) を初期化する
      System::Init(Log::ConfigureDefaultLog(LogMask_All));
    #endif

すべての処理は Window という安直な名前のクラスに実装しています．

Oculus Rift の情報の取得
------------------------
Oculus Rfit から情報を得るには，まず DeviceManager を作成し，
一台の Oculus Rift ごとに HMDDevice を作成します．
Oculus Rift を管理するための変数は Window クラスのメンバで定義しています．

    // Oculus Rift のヘッドトラッキングセンサ
    static Ptr<DeviceManager> pManager;
    Ptr<HMDDevice> pHmd;
    Ptr<SensorDevice> pSensor;
    HMDInfo hmdInfo;
    SensorFusion sensorFusion;
    
この Ptr というテンプレートは，どうやら std::shared_ptr みたいなものらしく，
この変数にポインタを代入すると変数がスコープを外れたときに，
自動的にデストラクタが呼ばれるみたいです (呼ばれて慌てました)．
DeviceManager はすべてのデバイスに対して一つだけ動かせばいいようなので，
static メンバ (クラス変数) にしています．

Window クラスは, 立体視を行う場合だけリファレンスカウントを使って, 
シェーダプログラムやテクスチャなどのリソースを不用意に解放しないようにしています．
DeviceManager の作成もこれを使って一度だけ行います．
このため，このクラスはコピー禁止にしています．

    // プログラムオブジェクト, VAO / VBO, Oculus Rift のデバイスマネージャーの作成は最初一度だけ行う
    if (!count++)
    {
      // Oculus Rift のレンズの歪みを補正するシェーダプログラム
      ocuProgram = ggLoadShader("oculus.vert", "oculus.frag");
      ocuFboColorLoc = glGetUniformLocation(ocuProgram, "ocuFboColor");
      ocuAspectLoc = glGetUniformLocation(ocuProgram, "ocuAspect");
      lensOffsetLoc = glGetUniformLocation(ocuProgram, "lensOffset");
      lensDistortionLoc = glGetUniformLocation(ocuProgram, "lensDistortion");
      lensScaleLoc = glGetUniformLocation(ocuProgram, "lensScale");
      
      // Oculus Rift 表示に使う矩形
      glGenVertexArrays(1, &ocuVao);
      glBindVertexArray(ocuVao);
      glGenBuffers(1, &ocuVbo);
      glBindBuffer(GL_ARRAY_BUFFER, ocuVbo);
      static const GLfloat rect[] = { -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f };
      glBufferData(GL_ARRAY_BUFFER, sizeof rect, rect, GL_STATIC_DRAW);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
      glEnableVertexAttribArray(0);
      
      // Oculus Rift のデバイスマネージャーの作成
      pManager = *DeviceManager::Create();
    }

視差の設定
----------
HMD (Head Mounted Display) における立体視は，単一ディスプレイによる立体視とは異なり，
スクリーン (シーンの投影面のディスプレイの表示領域 (ビューポート) に表示する範囲*1) をずらす必要はありません．

ただ，Oculus Rift の場合は左右の目に対する映像表示を一枚のディスプレイで行っていることや，
レンズを使って視野角を広げていることもあって，表示方法が若干複雑になっています．

Oculus Rift では一枚の液晶パネルを縦に半分に分けて表示します．
したがって，左目に対する表示を行う領域の中心の横位置は，駅書パネルの横幅 HScreenSize の 4 分の 1 になります．
LensSeparationDistance は Oculus Rift の左右のレンズ中心の間隔ですから，
液晶パネルの左側の中心位置 (本来の投影中心) と左側のレンズの中心位置との差は
HScreenSize / 4 - LensSeparationDistance / 2 になります．

一方，シーンのレンダリングの対象になるクリッピング空間の xy 平面の座標系である正規化デバイス座標系の横幅は 2 であり，
液晶パネルの片側の横幅いっぱいにこれを表示するなら，
正規化デバイス座標系上のこの差は液晶パネル上の差を HScreenSize / 4 で割ったものになります．
したがって，正規化デバイス座標系上のこの差 projectionCenterOffset は 1 - 2 LensSeparationDistance / HScreenSize になります．

シーンのスクリーンへの投影像の画角 (Fov, Field of View) は，
視点とスクリーンまでの距離とスクリーンの高さとの関係で決まります．
ただしこのプログラムでは，これを画角に置き換えることはせず，
液晶パネルの中心位置の高さ VScreenCenter と視点とスクリーンとの距離 EyeToScreenDistance との比を
スクリーンの高さと幅の大きさ scrH, scrW として保持します．
これらを zNear 倍して，シーンの投影面である前方面上のスクリーンの大きさを求めます．

視差 parallax には，人間の目と目の間隔 InterpupillaryDistance の 2 分の 1 を設定します．
この InterpupillaryDistance は左右の目の間隔の実測値というより，
シーンのスケールに対する人間のスケールみたいなものだと考えた方がいいかも知れません．
たとえば，この値が 0.64 のとき，シーンの座標値の単位がメートルなら，
これは 6.4cm となって，だいたい大人の左右の目の間隔になります．
これを，仮に 1000 分の 1 の 0.00064 に設定したとすれば，シーンの座標値の単位は km になり，
両眼視差 (輻輳角) による立体感が少なくなります．

これらの処理に必要な情報は，HMDInfo クラスの変数に格納されます．

    // Oculus Rift のデバイスマネージャーが作成できたら情報を取得する
    if (pManager
      && (pHmd = *pManager->EnumerateDevices().CreateDevice())
      && pHmd->GetDeviceInfo(&hmdInfo)
    )
    {
      // レンズの中心の画面の中心からのずれ
      projectionCenterOffset = 1.0f - 2.0f * hmdInfo.LensSeparationDistance / hmdInfo.HScreenSize;
      
      // スクリーンの幅と高さ
      scrW = scrH = zNear * hmdInfo.VScreenCenter / hmdInfo.EyeToScreenDistance;
      
      // 視差
      parallax = hmdInfo.InterpupillaryDistance * 0.5f;
      
      // レンズの歪みの補正係数
      lensDistortion[0] = hmdInfo.DistortionK[0];
      lensDistortion[1] = hmdInfo.DistortionK[1];
      lensDistortion[2] = hmdInfo.DistortionK[2];
      lensDistortion[3] = hmdInfo.DistortionK[3];
      
      // 片目の表示領域のアスペクト比
      ocuAspect = hmdInfo.HScreenSize * 0.5f / hmdInfo.VScreenSize;
      
      // Oculus Rift のセンサの取得
      pSensor = *pHmd->GetSensor();
      
      // センサーを登録する
      if (pSensor) sensorFusion.AttachToSensor(pSensor);
    }

透視投影変換行列の設定
----------------------
この scrW と scrH を使って，透視投影変換行列を求めます．
単一ディスプレイの場合は左右の目でスクリーンをずらす必要がありますが，
Oculus Rift の場合はその必要はありません．
なお，このメソッドは Windows.h で定義しています．

    //
    // 立体視用の透視投影変換行列を求める
    //
    //   ・ウィンドウのサイズ変更時やカメラパラメータの変更時に呼び出す
    //
    void updateStereoProjectionMatrix()
    {
      // 視差によるスクリーンのオフセット量
    #if STEREO == OCULUS
      const GLfloat shift(0.0f);
    #else
      const GLfloat shift(parallax * zNear / screenDistance);
    #endif
      
      // 立体視用の透視投影変換行列
      mpL.loadFrustum(-scrW + shift, scrW + shift, -scrH, scrH, zNear, zFar);
      mpR.loadFrustum(-scrW - shift, scrW - shift, -scrH, scrH, zNear, zFar);
    }

そのかわり，視点の位置は左右の目でずらして投影する必要があります．
これはモデルビュー変換行列に設定します．
なお，このときに左右の目ごとに異なる処理も行っています．

    //
    // 左目用のモデルビュー変換行列を得る
    //
    //   ・左目の描画特有の処理を行う
    //
    GgMatrix Window::getMvL() const
    {
    #  if STEREO == LINEBYLINE
      
      ... (中略) ...
      
    #  elif STEREO == OCULUS
      // 左目用の FBO に描画する
      glBindFramebuffer(GL_FRAMEBUFFER, ocuFbo[0]);
      glDrawBuffers(1, ocuFboDrawBuffers);
      
      // 左目用の FBO を消去する
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    #  endif
      
      // 左目を左に動かす代わりにシーンを右に動かす
      return mv.translate(parallax, 0.0f, 0.0f);
    }
     
    //
    // 右目用のモデルビュー変換行列を得る
    //
    //   ・右目の描画特有の処理を行う
    //
    GgMatrix Window::getMvR() const
    {
    #  if STEREO == LINEBYLINE
      
      ... (中略) ...
      
    #  elif STEREO == OCULUS
      // 右目用の FBO に描画する
      glBindFramebuffer(GL_FRAMEBUFFER, ocuFbo[1]);
      glDrawBuffers(1, ocuFboDrawBuffers);
      
      // 右目用の FBO を消去する
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    #  endif
      
      // 右目を左に動かす代わりにシーンを左に動かす
      return mv.translate(-parallax, 0.0f, 0.0f);
    }

レンズの歪みの補正
------------------
前節のプログラムでは，Oculus Rift への表示では直接フレームバッファに描かず，
一旦フレームバッファオブジェクト (FBO) に描画しています．
FBO に描画された内容をテクスチャとして参照して，レンズの歪みの補正処理を行います．
これには遅延レンダリングと同じテクニックで，補正処理は 2 パス目のフラグメントシェーダで行います．

縦横の大きさが [-1, 1]，すなわちクリッピング空間の xy 平面と同じ正方形のポリゴンを準備し*2，
それに FBO のカラーバッファに使ったテクスチャをマッピングします．

###FBO のテクスチャのマッピング

バーテックスシェーダで頂点座標をそのまま gl_Position に代入すれば，
ビューポートいっぱいにポリゴンを描きます．
また，Oculus Rift の片目の表示領域のアスペクト比を乗じ，
さらに先ほど求めた projectionCenterOffset 加えたものを，
varying (out) 変数 t を介してフラグメントシェーダに送ります．

    #version 150 core
    #extension GL_ARB_explicit_attrib_location : enable
     
    // Oculus Rift のアスペクト比
    uniform float ocuAspect;
     
    // Oculus Rift のレンズの中心の画面の中心からのずれ
    uniform float projectionCenterOffset;
     
    // 頂点属性
    layout (location = 0) in vec4 pv;
     
    // ラスタライザに送る頂点属性
    out vec2 t;
     
    void main()
    {
      t = vec2(pv.x * ocuAspect + projectionCenterOffset, pv.y);
      gl_Position = pv;
    }

また，HMDInfo クラスのメンバ変数 DistortionK の内容を uniform 変数 lensDistortion に格納しておき，
フラグメントシェーダではこれを用いて頂点座標の補間値である varing (in) 変数 t に補正を加えます．

これに DistortionK の総和の逆数 lensScale をかけて，レンズの歪み補正にともなう図形の縮小を補います．
そして，その結果を 0.5 倍し 0.5 を足して，[0, 1] のテクスチャ座標に直します．

    #version 150 core
    #extension GL_ARB_explicit_attrib_location : enable
     
    // テクスチャ
    uniform sampler2D ocuFboColor;
     
    // レンズの歪みの補正係数
    uniform vec4 lensDistortion;
     
    // レンズの拡大縮の補正係数
    uniform float lensScale;
     
    // ラスタライザから受け取る頂点属性の補間値
    in vec2 t;                                          // テクスチャ座標
     
    // フレームバッファに出力するデータ
    layout (location = 0) out vec4 fc;                  // フラグメントの色
     
    void main()
    {
      vec4 r;
      r.x = 1.0;
      r.y = dot(t, t);	// r^2
      r.z = r.y * r.y;	// r^4
      r.w = r.y * r.z;	// r^6
      vec2 tc = t * dot(r, lensDistortion);
      fc = texture(ocuFboColor, tc * lensScale * 0.5 + 0.5);
    }
    

ヘッドトラッキング
------------------
Oculus Rift を使う時は視線の方向をヘッドトラッキングで決定しています．
Oculus Rift の向きは SensorFusion クラスの GetOrientation() メソッドで取り出すことができます．
取り出した方向はクォータニオン (四元数) なので，
自分の授業の宿題用に使っている補助プログラムのクォータニオンを扱う関数にそのまま突っ込んでみたら
ちゃんと動いたので，結構嬉しかったです．
でもこのメソッドは，Oculus SDK のバージョン 0.3 以降の API では使えなくなったみたいなので悲しい…

このクォータニオンによる回転の変換行列をモデルビュー変換行列に反映します．
x 軸中心に -π/2 回転しているのは，このプログラムでは地形が xy 平面上にあるから (z が高さ) です．

    //
    // 画面クリア
    //
    //   ・図形の描画開始前に呼び出す
    //   ・画面の消去などを行う
    //
    void Window::clear()
    {
    #if STEREO == OCULUS
      // 隠面消去処理を有効にする
      glEnable(GL_DEPTH_TEST);
      
      // FBO 全体をビューポートにする
      glViewport(0, 0, fboWidth, fboHeight);
      
      // センサー有効時の処理
      if (pSensor)
      {
        // Oculus Rift の向きを取得する
        const Quatf o(sensorFusion.GetOrientation());
        
        // Oculus Rift の向きの回転の変換行列を求める
        const GgMatrix mo(ggQuaternionTransposeMatrix(GgQuaternion(o.x, o.y, o.z, o.w)));
        
        // Oculus Rift の向きをモデルビュー変換行列に反映する
        mv = mo.rotateX(-1.5707963f).rotateZ(direction).translate(-ex, -ey, -ez);
      }
      else
      {
        // モデルビュー変換行列を設定する
        mv = ggRotateX(pitch).rotateZ(heading + direction).translate(-ex, -ey, -ez);
      }
    #else
      // モデルビュー変換行列を設定する
      mv = ggRotateX(pitch).rotateZ(heading + direction).translate(-ex, -ey, -ez);
      
      // カラーバッファとデプスバッファを消去
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    #endif
    }

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

###マウスによる操作

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

###ゲームコントローラによる操作

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
