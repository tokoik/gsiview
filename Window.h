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
#    define OVR_OS_WIN32
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

  // スクリーンの高さ
  const GLfloat screenHeight;

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

  // ディスプレイのアスペクト比
  GLfloat aspect;

  // ズーム率
  double zoom;

  // ビューポートの幅と高さ
  int width, height;

  // 視差
  GLfloat parallax;

  // ヘッドトラッキングによる回転行列
  GgMatrix mo[2];

  // 立体視用のモデルビュー変換行列
  GgMatrix mw[2];

  // 立体視用の投影変換行列
  GgMatrix mp[2];

  // スクリーンの幅と高さ
  GLfloat screen[2][4];

#if STEREO == OCULUS
  // Oculus Rift のセッション
  const ovrSession session;

  // Oculus Rift の情報
  ovrHmdDesc hmdDesc;

  // Oculus Rift のレンダリング情報
  ovrEyeRenderDesc eyeRenderDesc[ovrEye_Count];

  // Oculus Rift の視点情報
  ovrPosef eyePose[ovrEye_Count];

  // Oculus Rift に転送する描画データ
  ovrLayer_Union layerData;

  // Oculus Rift 表示用の FBO
  GLuint oculusFbo[ovrEye_Count];

  // ミラー表示用のレンダリングターゲットのテクスチャ
  ovrGLTexture *mirrorTexture;

  // ミラー表示用の FBO
  GLuint mirrorFbo;
#endif

  // 参照カウント
  static unsigned int count;

  //
  // 透視投影変換行列を求める
  //
  //   ・ウィンドウのサイズ変更時やカメラパラメータの変更時に呼び出す
  //
  void updateProjectionMatrix();

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
  Window(int width = 640, int height = 480, const char *title = "GLFW Window"
    , GLFWmonitor *monitor = nullptr, GLFWwindow *share = nullptr
#if STEREO == OCULUS
    , ovrSession session = nullptr
#endif
    );

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

  //
  // 設定値の初期化
  //
  void reset();

  //
  // 描画設定
  //
  //   ・左目の図形の描画開始前に呼び出す
  //   ・ビューポートの設定などを行う
  //
  void select(int eye);

  //
  // Oculus Rift のヘッド地ラッキングによる左目の回転行列を得る
  //
  const GgMatrix &getMo(int eye) const
  {
    return mo[eye];
  }

  //
  // 左目用のモデルビュー変換行列を得る
  //
  const GgMatrix &getMw(int eye) const
  {
    return mw[eye];
  }

  //
  // 左目用のプロジェクション変換行列を得る
  //
  const GgMatrix &getMp(int eye) const
  {
    return mp[eye];
  }

  //
  // 左目用のスクリーンの幅と高さを取り出す
  //
  const GLfloat *getScreen(int eye) const
  {
    return screen[eye];
  }
};
