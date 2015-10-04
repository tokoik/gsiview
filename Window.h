#pragma once

//
// ウィンドウ関連の処理
//

// 各種設定
#include "config.h"

// Oculus Rift SDK ライブラリ (LibOVR) の組み込み
#if STEREO == OCULUS
#  if defined(_WIN32)
#    define NOMINMAX
#    define GLFW_EXPOSE_NATIVE_WIN32
#    define GLFW_EXPOSE_NATIVE_WGL
#    include "glfw3native.h"
#    if defined(APIENTRY)
#      undef APIENTRY
#    endif
#  endif
#  include <OVR.h>
#  include <OVR_CAPI_GL.h>
#endif

//
// ウィンドウ関連の処理を担当するクラス
//
class Window
{
  // ウィンドウの識別子
  GLFWwindow *const window;

  // 最後にタイプしたキー
  int key;

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

#if STEREO != OCULUS
  // ウィンドウの幅と高さ
  int winW, winH;

  // スクリーンの幅と高さ
  GLfloat scrW, scrH;

#  if STEREO == NONE
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
#  else
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
    const GLfloat shift(parallax * zNear / screenDistance);

    // 立体視用の透視投影変換行列
    mpL.loadFrustum(-scrW + shift, scrW + shift, -scrH, scrH, zNear, zFar);
    mpR.loadFrustum(-scrW - shift, scrW - shift, -scrH, scrH, zNear, zFar);
  }
#  endif
#else
  // Oculus Rift 表示用の FBO
  GLuint ocuFbo;

  // Oculus Rift 表示用の FBO のカラーバッファに使うテクスチャ
  GLuint ocuFboColor;

  // Oculus Rift 表示用の FBO のデプスバッファに使うレンダーバッファ
  GLuint ocuFboDepth;

  // Oculus Rift 表示用の FBO のレンダーターゲット
  static const GLenum ocuFboDrawBuffers[];

  // Oculus Rift 表示用の FBO のサイズ
  ovrSizei renderTargetSize;

  // Oculus Rift のビューポート
  ovrRecti eyeRenderViewport[2];

  // Oculus Rift のレンダリング情報
  ovrEyeRenderDesc eyeRenderDesc[2];

  // Oculus Rift の視点情報
  ovrPosef eyePose[2];

  // Oculus rift 表示用のレンダリングターゲットのテクスチャ
  ovrGLTexture eyeTexture[2];

  // Oculus Rift へのレンダリングのタイミング計測
  ovrFrameTiming frameTiming;

  // Oculus Rift のデバイス
  const ovrHmd hmd;
#endif

  // 参照カウント
  static unsigned int count;

  //
  // コピーコンストラクタ (コピー禁止)
  //
  Window(const Window &w);

  //
  // 代入 (代入禁止)
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
  const GgMatrix &getMw() const
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
  GgMatrix getMwL();

  //
  // 左目用のプロジェクション変換行列を得る
  //
#  if STEREO != OCULUS
  const GgMatrix &getMpL() const
  {
    return mpL;
  }
#  else
  const GgMatrix getMpL() const
  {
    // Oculus Rift の左目の識別子
    const ovrFovPort &fov(eyeRenderDesc[hmd->EyeRenderOrder[0]].Fov);

    // 左目の透視投影変換行列
    const GLfloat left(-fov.LeftTan * zNear);
    const GLfloat right(fov.RightTan * zNear);
    const GLfloat bottom(-fov.DownTan * zNear);
    const GLfloat top(fov.UpTan * zNear);
    return ggFrustum(left, right, bottom, top, zNear, zFar);
  }
#  endif

  //
  // 右目用のモデルビュー変換行列を得る
  //
  //   ・右目の描画特有の処理を行う
  //
  GgMatrix getMwR();

  //
  // 右目用のプロジェクション変換行列を得る
  //
#  if STEREO != OCULUS
  const GgMatrix &getMpR() const
  {
    return mpR;
  }
#  else
  const GgMatrix getMpR() const
  {
    // Oculus Rift の左目の識別子
    const ovrFovPort &fov(eyeRenderDesc[hmd->EyeRenderOrder[1]].Fov);

    // 左目の透視投影変換行列
    const GLfloat left(-fov.LeftTan * zNear);
    const GLfloat right(fov.RightTan * zNear);
    const GLfloat bottom(-fov.DownTan * zNear);
    const GLfloat top(fov.UpTan * zNear);
    return ggFrustum(left, right, bottom, top, zNear, zFar);
  }
#  endif
#endif
};
