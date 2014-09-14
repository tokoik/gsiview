#pragma once

//
// ウィンドウ関連の処理
//

// 標準ライブラリ
#include <cmath>

// 各種設定
#include "config.h"

// Oculus Rift SDK ライブラリ (LibOVR) の組み込み
#if STEREO == OCULUS
#  ifdef _WIN32
#    ifdef _DEBUG
#      pragma comment(lib, "libovrd.lib")
#    else
#      pragma comment(lib, "libovr.lib")
#    endif
#    pragma comment(lib, "winmm.lib")
#  endif
#  ifdef APIENTRY
#    undef APIENTRY
#  endif
#  include <OVRVersion.h>
#  include <OVR.h>
using namespace OVR;
#endif

//
// ウィンドウ関連の処理を担当するクラス
//
class Window
{
  // ウィンドウの識別子
  GLFWwindow *const window;

  // ジョイスティックの番号
  int joy;

  // スティックの中立位置
  float origin[4];

  // ドラッグ開始位置
  double cx, cy;

  // カメラの位置
  GLfloat ex, ey, ez;

  // カメラの進行方向
  GLfloat direction;

  // カメラの向き
  GLfloat heading, pitch;

  // モデルビュー変換行列
  GgMatrix mv;

  // スクリーンの幅と高さ
  GLfloat scrW, scrH;

#if STEREO == NONE
  // 投影変換行列
  GgMatrix mp;

  //
  // 透視投影変換行列を求める
  //
  //   ・ウィンドウのサイズ変更時やカメラパラメータの変更時に呼び出す
  //
  void updateProjectionMatrix()
  {
    mp.loadFrustum(-scrW, scrW, -scrH, scrH, zNear, zFar);
  }
#else
  // ウィンドウの幅と高さ
  int winW, winH;

  // 視差
  GLfloat parallax;

  // 立体視用の投影変換行列
  GgMatrix mpL, mpR;

  //
  // 立体視用の透視投影変換行列を求める
  //
  //   ・ウィンドウのサイズ変更時やカメラパラメータの変更時に呼び出す
  //
  void updateStereoProjectionMatrix()
  {
    // 視差によるスクリーンのオフセット量
#  if STEREO == OCULUS
    const GLfloat shift(0.0f);
#  else
    const GLfloat shift(parallax * zNear / screenDistance);
#  endif

    // 立体視用の透視投影変換行列
    mpL.loadFrustum(-scrW + shift, scrW + shift, -scrH, scrH, zNear, zFar);
    mpR.loadFrustum(-scrW - shift, scrW - shift, -scrH, scrH, zNear, zFar);
  }
#endif

#if STEREO == OCULUS
  // Oculus Rift 表示用の FBO
  GLuint ocuFbo[2];

  // Oculus Rift 表示用の FBO のカラーバッファに使うテクスチャ
  GLuint ocuFboColor[2];

  // Oculus Rift 表示用の FBO のデプスバッファに使うレンダーバッファ
  GLuint ocuFboDepth;

  // Oculus Rift の画面のアスペクト比
  GLfloat ocuAspect;

  // Oculus Rift のレンズの中心の画面の中心からのずれ
  GLfloat projectionCenterOffset;

  // Oculus Rift のレンズの歪みの補正係数
  GLfloat lensDistortion[4];

  // Oculus Rift のレンズの拡大率の補正係数
  GLfloat lensScale;

  // Oculus Rift 表示に使う矩形
  static GLuint ocuVao, ocuVbo;

  // Oculus Rift 表示用のシェーダプログラム
  static GLuint ocuProgram;

  // Oculus Rift 表示用の FBO のテクスチャユニットの uniform 変数の場所
  static GLint ocuFboColorLoc;

  // Oculus Rift の画面のアスペクト比の uniform 変数の場所
  static GLint ocuAspectLoc;

  // Oculus Rift のレンズの中心の画面の中心からのずれの uniform 変数の場所
  static GLint projectionCenterOffsetLoc;

  // Oculus Rift のレンズの歪みの補正係数の uniform 変数の場所
  static GLint lensDistortionLoc;

  // Oculus Rift のレンズの拡大率の補正係数の uniform 変数の場所
  static GLint lensScaleLoc;

  // Oculus Rift 表示用の FBO のレンダーターゲット
  static const GLenum ocuFboDrawBuffers[];

  // Oculus Rift のヘッドトラッキングセンサ
  static Ptr<DeviceManager> pManager;
  Ptr<HMDDevice> pHmd;
  Ptr<SensorDevice> pSensor;
  HMDInfo hmdInfo;
  SensorFusion sensorFusion;
#endif

  // 参照カウント
  static unsigned int count;

  //
  // コピーコンストラクタ (コピー禁止)
  //
  Window(const Window &w);

  //
  // 代入 (コピー禁止)
  //
  Window &operator=(const Window &w);

public:

  //
  // コンストラクタ
  //
  Window(int width = 640, int height = 480, const char *title = "GLFW Window",
    GLFWmonitor *monitor = nullptr, GLFWwindow *share = nullptr);

  //
  // デストラクタ
  //
  virtual ~Window();

  //
  // ウィンドウの識別子の取得
  //
  const GLFWwindow *get() const
  {
    return window;
  }

  //
  // ウィンドウを閉じるべきかを判定する
  //
  //   ・描画ループの継続条件として使う
  //
  bool shouldClose() const
  {
    // ウィンドウを閉じるか ESC キーがタイプされていれば真
    return glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE);
  }

  //
  // 画面クリア
  //
  //   ・図形の描画開始前に呼び出す
  //   ・画面の消去などを行う
  //
  void clear();

  //
  // カラーバッファを入れ替えてイベントを取り出す
  //
  //   ・図形の描画終了後に呼び出す
  //   ・ダブルバッファリングのバッファの入れ替えを行う
  //   ・キーボード操作等のイベントを取り出す
  //
  void swapBuffers();

  //
  // ウィンドウのサイズ変更時の処理
  //
  //   ・ウィンドウのサイズ変更時にコールバック関数として呼び出される
  //   ・ウィンドウの作成時には明示的に呼び出す
  //
  static void resize(GLFWwindow *window, int width, int height);

  //
  // マウスボタンを操作したときの処理
  //
  //   ・マウスボタンを押したときにコールバック関数として呼び出される
  //
  static void mouse(GLFWwindow *window, int button, int action, int mods);

  //
  // マウスホイール操作時の処理
  //
  //   ・マウスホイールを操作した時にコールバック関数として呼び出される
  //
  static void wheel(GLFWwindow *window, double x, double y);

  //
  // キーボードをタイプした時の処理
  //
  //   ．キーボードをタイプした時にコールバック関数として呼び出される
  //
  static void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods);

#if STEREO == NONE
  //
  // モデルビュー変換行列を得る
  //
  const GgMatrix &getMv() const
  {
    return mv;
  }

  //
  // プロジェクション変換行列を得る
  //
  const GgMatrix &getMp() const
  {
    return mp;
  }
#else
  //
  // 左目用のモデルビュー変換行列を得る
  //
  //   ・左目の描画特有の処理を行う
  //
  GgMatrix getMvL() const;

  //
  // 左目用のプロジェクション変換行列を得る
  //
  const GgMatrix &getMpL() const
  {
    return mpL;
  }

  //
  // 右目用のモデルビュー変換行列を得る
  //
  //   ・右目の描画特有の処理を行う
  //
  GgMatrix getMvR() const;

  //
  // 右目用のプロジェクション変換行列を得る
  //
  const GgMatrix &getMpR() const
  {
    return mpR;
  }
#endif
};
