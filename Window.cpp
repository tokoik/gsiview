//
// ウィンドウ関連の処理
//
#include <iostream>
#include <algorithm>
#include "Window.h"

// Oculus Rift SDK ライブラリ (LibOVR) の組み込み
#if STEREO == OCULUS
#  if defined(_WIN32)
#    if defined(_WIN64)
#      define OVR_LIB "libovr64"
#    else
#      define OVR_LIB "libovr"
#    endif
#    if defined(_DEBUG)
#      pragma comment(lib, OVR_LIB "d.lib")
#    else
#      pragma comment(lib, OVR_LIB ".lib")
#    endif
#    pragma comment(lib, "winmm.lib")
#  endif
#endif

// Mac と Linux ではジョイスティックの右側のスティックの番号が一つずれる
#if defined(_WIN32)
const int axesOffset(0);
#else
const int axesOffset(1);
#endif

//
// コンストラクタ
//
Window::Window(int width, int height, const char *title, GLFWmonitor *monitor, GLFWwindow *share)
  : window(glfwCreateWindow(width, height, title, monitor, share))
  , key(0)                                // 最後にタイプしたキー
  , ex(startPosition[0])                  // カメラの x 座標
  , ey(startPosition[1])                  // カメラの y 座標
  , ez(startPosition[2])                  // カメラの z 座標
  , direction(0.0f)                       // カメラの進行方向
  , heading(0.0f)                         // カメラの方位角
  , pitch(0.0f)                           // カメラの仰角
#if STEREO != OCULUS
#  if STEREO != NONE
  , parallax(initialParallax)
#  endif
  , scrH(zNear * screenCenter / screenDistance)
#else
  , hmd(ovrHmd_Create(count))
#endif
{
  if (!window) return;

  // 現在のウィンドウを処理対象にする
  glfwMakeContextCurrent(window);

  // 作成したウィンドウに対する設定
  glfwSwapInterval(1);

  // ウィンドウのサイズ変更時に呼び出す処理の登録
  glfwSetFramebufferSizeCallback(window, resize);

  // マウスボタンを操作したときの処理
  glfwSetMouseButtonCallback(window, mouse);

  // マウスホイール操作時に呼び出す処理
  glfwSetScrollCallback(window, wheel);

  // キーボードを操作した時の処理
  glfwSetKeyCallback(window, keyboard);

  // マウスカーソルを表示する
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  // このインスタンスの this ポインタを記録しておく
  glfwSetWindowUserPointer(window, this);

  // ゲームグラフィックス特論の都合にもとづく初期化
  if (!glCreateProgram) ggInit();

  // ジョイステックの有無を調べて番号を決める
  joy = glfwJoystickPresent(count) ? count : -1;

  // スティックの中立位置を求める
  if (joy >= 0)
  {
    int axesCount;
    const float *const axes(glfwGetJoystickAxes(joy, &axesCount));

    if (axesCount > 3 + axesOffset)
    {
      // 起動直後のスティックの位置を基準にする
      origin[0] = axes[0];
      origin[1] = axes[1];
      origin[2] = axes[2 + axesOffset];
      origin[3] = axes[3 + axesOffset];
    }
  }

#if STEREO == OCULUS
  if (hmd)
  {
#  if defined(_DEBUG)
    // Oculus Rift の情報を表示する
    std::cout
      << "\nProduct name: " << hmd->ProductName
      << "\nResolution:   " << hmd->Resolution.w << " x " << hmd->Resolution.h
      << "\nScreen Size:  " << hmd->CameraFrustumHFovInRadians
      << " x " << hmd->CameraFrustumVFovInRadians
      << "\nDepth Range:   " << hmd->CameraFrustumNearZInMeters
      << " - " << hmd->CameraFrustumFarZInMeters
      << "\n" << std::endl;
#  endif

    // Oculus Rift レンダリング用の左右の目のビューポート
    eyeRenderViewport[0].Pos = ovrVector2i{ 0, 0 };
    eyeRenderViewport[0].Size = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left, hmd->DefaultEyeFov[0], 1.0f);
    eyeRenderViewport[1].Pos = ovrVector2i{ eyeRenderViewport[0].Size.w, 0 };
    eyeRenderViewport[1].Size = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right, hmd->DefaultEyeFov[1], 1.0f);

    // Oculus Rift レンダリング用の FBO のサイズ
    renderTargetSize.w = eyeRenderViewport[0].Size.w + eyeRenderViewport[1].Size.w;
    renderTargetSize.h = std::max(eyeRenderViewport[0].Size.h, eyeRenderViewport[1].Size.h);

    // Oculus Rift レンダリング用の FBO のカラーバッファとして使うカラーテクスチャの作成
    glGenTextures(1, &ocuFboColor);
    glBindTexture(GL_TEXTURE_2D, ocuFboColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, renderTargetSize.w, renderTargetSize.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

    // Oculus Rift レンダリング用の FBO のデプスバッファとして使うレンダーバッファの作成
    glGenRenderbuffers(1, &ocuFboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, ocuFboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, renderTargetSize.w, renderTargetSize.h);

    // Oculus Rift のレンダリング用の FBO を作成する
    glGenFramebuffers(1, &ocuFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, ocuFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ocuFboColor, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ocuFboDepth);

    // レンダーターゲットのテクスチャを参照する設定
    eyeTexture[0].OGL.Header.API = eyeTexture[1].OGL.Header.API = ovrRenderAPI_OpenGL;
    eyeTexture[0].OGL.Header.TextureSize = eyeTexture[1].OGL.Header.TextureSize = renderTargetSize;
    eyeTexture[0].OGL.Header.RenderViewport = eyeRenderViewport[0];
    eyeTexture[1].OGL.Header.RenderViewport = eyeRenderViewport[1];
    eyeTexture[0].OGL.TexId = eyeTexture[1].OGL.TexId = ocuFboColor;

    // Oculus Rift に OpenGL でレンダリングするための設定
    ovrGLConfig cfg;
    cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
    cfg.OGL.Header.BackBufferSize = hmd->Resolution;
    cfg.OGL.Header.Multisample = backBufferMultisample;
#if defined(_WIN32)
    cfg.OGL.Window = glfwGetWin32Window(window);
    cfg.OGL.DC = GetDC(cfg.OGL.Window);

    // 拡張デスクトップでなければ Direct Rendering する
    if (!(hmd->HmdCaps & ovrHmdCap_ExtendDesktop))
      ovrHmd_AttachToWindow(hmd, cfg.OGL.Window, nullptr, nullptr);
#elif defined(X11)
    cfg.OGL.Disp = glfwGetX11Display();
#endif

    // Oculus Rift を設定する
    ovrHmd_ConfigureRendering(hmd, &cfg.Config,
      ovrDistortionCap_Chromatic |
      ovrDistortionCap_Vignette |
      ovrDistortionCap_TimeWarp |
      ovrDistortionCap_Overdrive |
      0,
      hmd->DefaultEyeFov, eyeRenderDesc);
    ovrHmd_SetEnabledCaps(hmd,
      ovrHmdCap_LowPersistence |
      ovrHmdCap_DynamicPrediction |
      0);

    // トラッキング・センサフュージョンを初期化する
    ovrHmd_ConfigureTracking(hmd,
      ovrTrackingCap_Orientation |
      ovrTrackingCap_MagYawCorrection |
      ovrTrackingCap_Position |
      0, 0);
  }
#endif

  // 投影変換行列・ビューポートを初期化する
  resize(window, width, height);

  // 参照カウントを増す
  ++count;
}

//
// デストラクタ
//
Window::~Window()
{
  // 参照カウントを減じる
  --count;

#if STEREO == OCULUS
  // Oculus Rift のレンダリングの設定を初期設定に戻す
  ovrHmd_ConfigureRendering(hmd, nullptr, 0, nullptr, nullptr);

  // Oculus Rift のデバイスを破棄する
  ovrHmd_Destroy(hmd);
#endif

  glfwDestroyWindow(window);
}

//
// 画面クリア
//
//   ・図形の描画開始前に呼び出す
//   ・画面の消去などを行う
//
void Window::clear()
{
  // モデルビュー変換行列を設定する
  mv = ggRotateX(pitch).rotateZ(heading + direction).translate(-ex, -ey, -ez);

#if STEREO == NONE || STEREO == QUADBUFFER
  // ウィンドウ全体をビューポートにする
  glViewport(0, 0, winW, winH);
#elif STEREO == OCULUS
  // フレームのタイミング計測開始
  frameTiming = ovrHmd_BeginFrame(hmd, 0);

  // FBO に描画する
  glBindFramebuffer(GL_FRAMEBUFFER, ocuFbo);
  glDrawBuffers(1, ocuFboDrawBuffers);
#endif

  // カラーバッファとデプスバッファを消去する
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//
// カラーバッファを入れ替えてイベントを取り出す
//
//   ・図形の描画終了後に呼び出す
//   ・ダブルバッファリングのバッファの入れ替えを行う
//   ・キーボード操作等のイベントを取り出す
//
void Window::swapBuffers()
{
  // エラーチェック
  ggError("SwapBuffers");

  // イベントを取り出す
  glfwPollEvents();

#if STEREO != OCULUS
  // カラーバッファを入れ替える
  glfwSwapBuffers(window);
#else
  // 健康と安全に関する警告の表示状態を取得する
  ovrHSWDisplayState hswDisplayState;
  ovrHmd_GetHSWDisplayState(hmd, &hswDisplayState);

  // 警告表示をしていれば
  if (hswDisplayState.Displayed)
  {
    if (key)
    {
      // 何かキーをタイプしていれば警告を消す
      ovrHmd_DismissHSWDisplay(hmd);
    }
    else
    {
      // Oculus Rift を横から軽くたたいたかどうかを検出する
      const ovrTrackingState ts(ovrHmd_GetTrackingState(hmd, ovr_GetTimeInSeconds()));

      // 向きの変化が検出されたら
      if (ts.StatusFlags & ovrStatus_OrientationTracked)
      {
        // 生のセンサーの加速度を取得する
        const ovrVector3f a(ts.RawSensorData.Accelerometer);

        // 加速度が一定以上だったら警告を消す
        if (a.x * a.x + a.y * a.y + a.z * a.z > 10000.0f) ovrHmd_DismissHSWDisplay(hmd);
      }
    }
  }

  // FBO への描画を終了する
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

  // マウスの位置を調べる
  double x, y;
  glfwGetCursorPos(window, &x, &y);

  // 速度を高度に比例させる
  const float speedFactor((fabs(ez) + 0.2f));

  // 左ボタンドラッグ
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1))
  {
    // カメラの位置を移動する
    const GLfloat speed(static_cast<GLfloat>(cy - y) * speedScale * speedFactor);
    ex += speed * sin(direction);
    ey += speed * cos(direction);

    // カメラの進行方向を変える
    direction += angleScale * static_cast<GLfloat>(x - cx);
  }

  // 右ボタンドラッグ
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2))
  {
    // マウスボタンを押した位置からの変位
    const GLfloat dx(static_cast<GLfloat>(x - cx));
    const GLfloat dy(static_cast<GLfloat>(y - cy));

    // 移動量の大きい方だけ変更する方が扱いやすい気がする
    if (fabs(dx) > fabs(dy))
    {
      // カメラの方位角を変える
      heading += angleScale * dx;
    }
    else
    {
      // カメラの仰角を変える
      pitch += angleScale * dy;
    }
  }

  // ゲームパッドによる操作
  if (joy >= 0)
  {
    // スティック
    int axesCount;
    const float *const axes(glfwGetJoystickAxes(joy, &axesCount));

    if (axesCount > 3 + axesOffset)
    {
      // スティックの速度係数
      GLfloat axesSpeedFactor = axesSpeedScale * speedFactor;

      // カメラを前後に移動する
      const GLfloat advSpeed((axes[1] - origin[1]) * axesSpeedFactor);
      ex -= advSpeed * sin(direction);
      ey -= advSpeed * cos(direction);

      // カメラを左右に移動する
      const GLfloat latSpeed((axes[2 + axesOffset] - origin[2]) * axesSpeedFactor);
      ey -= latSpeed * sin(direction);
      ex += latSpeed * cos(direction);

      // カメラを上下に移動する
      ez -= (axes[3 + axesOffset] - origin[3]) * axesSpeedFactor;

      // カメラの進行方向を更新する
      direction += (axes[0] - origin[0]) * axesAngleScale;
    }

    // ボタン
    int btnsCount;
    const unsigned char *const btns(glfwGetJoystickButtons(joy, &btnsCount));
    if (btnsCount > 3)
    {
      // カメラの仰角を調整する
      pitch += btnsScale * static_cast<GLfloat>(btns[2] - btns[1]);

      // カメラの方位角を調整する
      heading += btnsScale * static_cast<GLfloat>(btns[3] - btns[0]);
    }

    // カメラの方位角を正面に戻す
    if (btnsCount > 4 && btns[4] > 0) heading = 0.0f;
  }

#if STEREO != NONE
#  if STEREO != OCULUS
  // 右矢印キー操作
  if (glfwGetKey(window, GLFW_KEY_RIGHT))
  {
    // 視差を拡大する
    parallax += parallaxStep;
    updateStereoProjectionMatrix();
  }

  // 左矢印キー操作
  if (glfwGetKey(window, GLFW_KEY_LEFT))
  {
    // 視差を縮小する
    parallax -= parallaxStep;
    updateStereoProjectionMatrix();
  }
#  else
  // フレームのタイミング計測終了
  ovrHmd_EndFrame(hmd, eyePose, &eyeTexture[0].Texture);
#  endif
#endif
}

//
// ウィンドウのサイズ変更時の処理
//
//   ・ウィンドウのサイズ変更時にコールバック関数として呼び出される
//   ・ウィンドウの作成時には明示的に呼び出す
//
void Window::resize(GLFWwindow *window, int width, int height)
{
  // このインスタンスの this ポインタを得る
  Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

  if (instance)
  {
#if STEREO != OCULUS
    // ディスプレイのアスペクト比 w / h からスクリーンの幅を求める
    instance->scrW = instance->scrH * static_cast<GLfloat>(width) / static_cast<GLfloat>(height);

#  if STEREO == SIDEBYSIDE
    // ウィンドウの横半分をビューポートにする
    width /= 2;
#  elif STEREO == TOPANDBOTTOM
    // ウィンドウの縦半分をビューポートにする
    height /= 2;
#  endif
#endif

#if STEREO != OCULUS
    // ビューポートの大きさを保存しておく
    instance->winW = width;
    instance->winH = height;

#  if STEREO == NONE
    // 単眼視用の投影変換行列を求める
    instance->updateProjectionMatrix();
#  else
    // 立体視用の投影変換行列を求める
    instance->updateStereoProjectionMatrix();
#  endif
#endif
  }
}

//
// マウスボタンを操作したときの処理
//
//   ・マウスボタンを押したときにコールバック関数として呼び出される
//
void Window::mouse(GLFWwindow *window, int button, int action, int mods)
{
  // このインスタンスの this ポインタを得る
  Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

  if (instance)
  {
    // マウスの現在位置を取り出す
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    switch (button)
    {
    case GLFW_MOUSE_BUTTON_1:
    case GLFW_MOUSE_BUTTON_2:
      if (action)
      {
        // ドラッグ開始位置を保存する
        instance->cx = x;
        instance->cy = y;
      }
      break;
    case GLFW_MOUSE_BUTTON_3:
      break;
      break;
    default:
      break;
    }
  }
}

//
// マウスホイール操作時の処理
//
//   ・マウスホイールを操作した時にコールバック関数として呼び出される
//
void Window::wheel(GLFWwindow *window, double x, double y)
{
  // このインスタンスの this ポインタを得る
  Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

  if (instance)
  {
    if (fabs(x) > fabs(y))
    {
      // カメラを左右に移動する
      const GLfloat latSpeed((fabs(instance->ez) * 5.0f + 1.0f) * wheelXStep * static_cast<GLfloat>(x));
      instance->ey -= latSpeed * sin(instance->direction);
      instance->ex += latSpeed * cos(instance->direction);
    }
    else
    {
      // カメラを上下に移動する
      const GLfloat advSpeed((fabs(instance->ez) * 5.0f + 1.0f) * wheelYStep * static_cast<GLfloat>(y));
      instance->ez += advSpeed;
    }
  }
}

//
// キーボードをタイプした時の処理
//
//   ．キーボードをタイプした時にコールバック関数として呼び出される
//
void Window::keyboard(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  // このインスタンスの this ポインタを得る
  Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

  if (instance)
  {
    if (action == GLFW_PRESS)
    {
      // 最後にタイプしたキーを覚えておく
      instance->key = key;

      // キーボード操作による処理
      switch (key)
      {
      case GLFW_KEY_R:
        // カメラの位置をリセットする
        instance->ex = startPosition[0];
        instance->ey = startPosition[1];
        instance->ez = startPosition[2];
        instance->direction = 0.0f;
      case GLFW_KEY_O:
        // カメラの向きをリセットする
        instance->pitch = 0.0f;
      case GLFW_KEY_H:
        // カメラの方位角だけをリセットする
        instance->heading = 0.0f;
        break;
      case GLFW_KEY_SPACE:
        break;
      case GLFW_KEY_BACKSPACE:
      case GLFW_KEY_DELETE:
        break;
      case GLFW_KEY_UP:
        break;
      case GLFW_KEY_DOWN:
        break;
      case GLFW_KEY_RIGHT:
        break;
      case GLFW_KEY_LEFT:
        break;
      default:
        break;
      }
    }
  }
}

#if STEREO != NONE
//
// 左目用のモデルビュー変換行列を得る
//
//   ・左目の描画特有の処理を行う
//
GgMatrix Window::getMwL()
{
#  if STEREO != OCULUS
#    if STEREO == LINEBYLINE
  // 偶数番目の走査線だけに描画する
#    elif STEREO == TOPANDBOTTOM
  // ディスプレイの上半分だけに描画する
  glViewport(0, winH, winW, winH);
#    elif STEREO == SIDEBYSIDE
  // ディスプレイの左半分だけに描画する
  glViewport(0, 0, winW, winH);
#    elif STEREO == QUADBUFFER
  // 左目用バッファに描画する
  glDrawBuffer(GL_BACK_LEFT);
#    endif

  // 左目を左に動かす代わりにシーンを右に動かす
  return ggTranslate(parallax, 0.0f, 0.0f) * mv;
#  else
  // Oculus Rift の左目の識別子
  const ovrEyeType &eyeL(hmd->EyeRenderOrder[0]);

  // Oculus Rift の右目のビューポートに描画する
  glViewport(eyeRenderViewport[eyeL].Pos.x, eyeRenderViewport[eyeL].Pos.y,
    eyeRenderViewport[eyeL].Size.w, eyeRenderViewport[eyeL].Size.h);

  // Oculus Rift の左目の姿勢を取得する
  eyePose[0] = ovrHmd_GetHmdPosePerEye(hmd, eyeL);

  // Oculus Rift の左目の位置と向きを取得する
  const ovrQuatf &o(eyePose[0].Orientation);
  const ovrVector3f &p(eyePose[0].Position);
  const ovrVector3f &q(eyeRenderDesc[eyeL].HmdToEyeViewOffset);

  // Oculus Rift の右目の向きをモデルビュー変換行列に反映する
  return ggQuaternionTransposeMatrix(GgQuaternion(o.x, o.y, o.z, o.w)) * ggTranslate(q.x - p.x, q.y - p.y, q.z - p.z) * mv;
#  endif
}

//
// 右目用のモデルビュー変換行列を得る
//
//   ・右目の描画特有の処理を行う
//
GgMatrix Window::getMwR()
{
#  if STEREO != OCULUS
#    if STEREO == LINEBYLINE
  // 奇数番目の走査線だけに描画する
#    elif STEREO == TOPANDBOTTOM
  // ディスプレイの下半分だけに描画する
  glViewport(0, 0, winW, winH);
#    elif STEREO == SIDEBYSIDE
  // ディスプレイの右半分だけに描画する
  glViewport(winW, 0, winW, winH);
#    elif STEREO == QUADBUFFER
  // 右目用バッファに描画する
  glDrawBuffer(GL_BACK_RIGHT);
#    endif

  // 右目を左に動かす代わりにシーンを左に動かす
  return ggTranslate(-parallax, 0.0f, 0.0f) * mv;
#  else
  // Oculus Rift の右目の識別子
  const ovrEyeType &eyeR(hmd->EyeRenderOrder[1]);

  // Oculus Rift の右目のビューポートを設定する
  glViewport(eyeRenderViewport[eyeR].Pos.x, eyeRenderViewport[eyeR].Pos.y,
    eyeRenderViewport[eyeR].Size.w, eyeRenderViewport[eyeR].Size.h);

  // Oculus Rift の右目の姿勢を取得する
  eyePose[1] = ovrHmd_GetHmdPosePerEye(hmd, eyeR);

  // Oculus Rift の右目の位置と向きを取得する
  const ovrQuatf &o(eyePose[1].Orientation);
  const ovrVector3f &p(eyePose[1].Position);
  const ovrVector3f &q(eyeRenderDesc[eyeR].HmdToEyeViewOffset);

  // Oculus Rift の右目の向きをモデルビュー変換行列に反映する
  return ggQuaternionTransposeMatrix(GgQuaternion(o.x, o.y, o.z, o.w)) * ggTranslate(q.x - p.x, q.y - p.y, q.z - p.z) * mv;
#  endif
}

#  if STEREO == OCULUS
// Oculus Rift 表示用の FBO のレンダーターゲット
const GLenum Window::ocuFboDrawBuffers[] = { GL_COLOR_ATTACHMENT0 };
#  endif
#endif

// 参照カウント
unsigned int Window::count(0);
