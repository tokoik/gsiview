#pragma once

//
// 各種設定
//

// 補助プログラム
#include "gg.h"
using namespace gg;

// 立体視の設定
#define NONE          0                                 // 単眼視
#define LINEBYLINE    1                                 // インターレース（未サポート）
#define TOPANDBOTTOM  2                                 // 上下
#define SIDEBYSIDE    3                                 // 左右
#define QUADBUFFER    4                                 // クワッドバッファステレオ
#define OCULUS        5                                 // Oculus Rift (HMD)

// 立体視の方式
#define STEREO        NONE

// 立体視特有のパラメータ
#if STEREO != NONE
const int useSecondary(1);                              // 1 ならセカンダリモニタに表示
const GLfloat initialParallax(0.032f);                  // 視差の初期値 (単位 m)
const GLfloat parallaxStep(0.001f);                     // 視差の変更ステップ (単位 m)
const GLfloat screenDistance(2.0f);                     // 実際のスクリーンまでの距離 (単位 m)

// Oculus Rift 固有のパラメータ
#  if STEREO == OCULUS
const GLfloat lensScaleStep(0.001f);                    // レンズの拡大率の補正係数の調整ステップ
const GLfloat projectionCenterOffsetStep(0.001f);       // レンズの中心位置の調整ステップ
const GLuint fboWidth(1024), fboHeight(1024);           // 補正に使う FBO のサイズ
#  endif
#endif

// 地形データ
const char demfile[] = "dem.csv";                       // デジタル標高地図データ
const GLfloat demscale[] = { 5.0f, 5.0f, 0.1f, 1.0f };  // 地図データのスケール
const char texfile[] = "texture.png";                   // 地図のテクスチャ
const GLsizei texWidth(2048), texHeight(2048);          // テクスチャメモリのサイズ
#define USE_ANISOTROPIC_FILTERING 1                     // 1 なら非対称フィルタリング拡張機能を使う

// カメラの初期状態
const GLfloat startPosition[] = { 0.0f, 0.0f, 20.0f };  // カメラの初期位置
const GLfloat displayCenter(0.5f);                      // ディスプレイの中心位置 (高さの半分
const GLfloat displayDepth(1.5f);                       // 観測者とディスプレイ面との距離
const GLfloat zNear(0.1f);                              // 前方面までの距離
const GLfloat zFar(50.0f);                              // 後方面までの距離

// ナビゲーションの速度調整
const GLfloat speedScale(0.00005f);                     // フレームあたりの移動速度係数
const GLfloat angleScale(0.00001f);                     // フレームあたりの回転速度係数
const GLfloat heightStep(0.005f);                       // カメラの高さの調整係数
const GLfloat axesSpeedScale(0.020f);                   // ゲームパッドのスティックの速度の係数
const GLfloat axesAngleScale(0.005f);                   // ゲームパッドのスティックの角速度の係数
const GLfloat btnsScale(0.005f);                        // ゲームパッドのボタンの係数

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
