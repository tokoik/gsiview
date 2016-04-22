#pragma once

//
// 各種設定
//

// 補助プログラム
#include "gg.h"
using namespace gg;

// 立体視の設定
#define NONE          0                                 // 単眼視
#define LINEBYLINE    1                                 // インターレース（未実装）
#define TOPANDBOTTOM  2                                 // 上下
#define SIDEBYSIDE    3                                 // 左右
#define QUADBUFFER    4                                 // クワッドバッファステレオ
#define OCULUS        5                                 // Oculus Rift (HMD)

// 立体視の方式
#define STEREO        OCULUS

// セカンダリモニタの使用
const int useSecondary(1);                              // 1 ならセカンダリモニタに表示

// 地形データ
const char demfile[] = "dem.csv";                       // デジタル標高地図データ
const GLfloat demscale[] = { 5.0f, 5.0f, 0.1f, 1.0f };  // 地図データのスケール
const char texfile[] = "texture.png";                   // 地図のテクスチャ
#define USE_ANISOTROPIC_FILTERING 1                     // 1 なら非対称フィルタリング拡張機能を使う

// カメラの初期状態 (単位 m)
const GLfloat startPosition[] = { 0.0f, 0.0f, 5.0f };   // カメラの初期位置
const GLfloat startPitch(-1.5707963f);                  // 最初は正面を向いている
const GLfloat displayCenter(0.5f);                      // ディスプレイの中心位置 (高さの半分)
const GLfloat displayDistance(1.5f);                    // 観測者とディスプレイ面との距離
const GLfloat zNear(0.1f);                              // 前方面までの距離
const GLfloat zFar(500.0f);                             // 後方面までの距離

// 視差
const GLfloat initialParallax(0.032f);                  // 視差の初期値 (単位 m)
const GLfloat parallaxStep(0.001f);                     // 視差の変更ステップ (単位 m)

// ナビゲーションの速度調整
const GLfloat initialZoom(1.0f);                        // ズーム率の初期値
const double zoomStep(0.01);                            // ズーム率調整のステップ
const GLfloat speedScale(0.00002f);                     // フレームあたりの移動速度係数
const GLfloat angleScale(0.00002f);                     // フレームあたりの回転速度係数
#if defined(__APPLE__)
const GLfloat wheelXStep(0.001f);                       // Magic Mouse の X 方向の係数
const GLfloat wheelYStep(0.001f);                       // Magic Mouse の Y 方向の係数
#else
const GLfloat wheelXStep(0.005f);                       // マウスホイールの X 方向の係数
const GLfloat wheelYStep(0.005f);                       // マウスホイールの Y 方向の係数
#endif
const GLfloat axesSpeedScale(0.01f);                    // ゲームパッドのスティックの速度の係数
const GLfloat axesAngleScale(0.01f);                    // ゲームパッドのスティックの角速度の係数
const GLfloat btnsScale(0.01f);                         // ゲームパッドのボタンの係数

// 光源
const GgSimpleShader::Light light =
{
  { 0.4f, 0.4f, 0.4f, 1.0f },                           // 環境光成分
  { 0.8f, 0.8f, 0.8f, 1.0f },                           // 拡散反射光成分
  { 0.8f, 0.8f, 0.8f, 1.0f },                           // 鏡面光成分
  { 0.0f, 0.5f, 1.0f, 0.0f }                            // 位置
};

// 材質
const GgSimpleShader::Material material =
{
  { 0.8f, 0.8f, 0.8f, 1.0f },                           // 環境光の反射係数
  { 0.8f, 0.8f, 0.8f, 1.0f },                           // 拡散反射係数
  { 0.2f, 0.2f, 0.2f, 1.0f },                           // 鏡面反射係数
  50.0f                                                 // 輝き係数
};

// 境界色 (Oculus Rift 表示時の表示範囲外の色)
const GLfloat border[] = { 0.0, 0.0, 0.0, 0.0 };

// 背景色
const GLfloat back[] = { 0.2f, 0.3f, 0.4f, 0.0f };

// デバッグモード
#if defined(_DEBUG)
const bool debug(true);
#else
const bool debug(false);
#endif
