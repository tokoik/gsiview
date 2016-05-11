//
// ウィンドウ関連の処理
//
#include <iostream>
#include "Window.h"

// Oculus Rift SDK ライブラリ (LibOVR) の組み込み
#if STEREO == OCULUS && defined(_WIN32)
#  define OCULUSDIR "C:\\OculusSDK"
#  if defined(_WIN64)
#    define PLATFORM "x64"
#  else
#    define PLATFORM "Win32"
#  endif
#  if _MSC_VER >= 1900
#    define VS "VS2015"
#  elif _MSC_VER >= 1800
#    define VS "VS2013"
#  elif _MSC_VER >= 1700
#    define VS "VS2012"
#  elif _MSC_VER >= 1600
#    define VS "VS2010"
#  else
#    error "Unsupported compiler version."
#  endif
#  pragma comment(lib, OCULUSDIR "\\LibOVR\\Lib\\Windows\\" PLATFORM "\\Release\\" VS "\\libOVR.lib")
#  pragma comment(lib, "winmm.lib")
// Oculus Rift の目の識別子
const int eyeL(ovrEye_Left), eyeR(ovrEye_Right);
#else
// 目の識別子
const int eyeL(0), eyeR(1);
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
  , screenHeight(zNear * displayCenter / displayDistance)
#if STEREO == OCULUS
  , session(nullptr)
#endif
{
  // ウィンドウが開かれていなかったら戻る
  if (!window) return;

  // 設定を初期化する
  reset();

  // 現在のウィンドウを処理対象にする
  glfwMakeContextCurrent(window);

  // ゲームグラフィックス特論の都合にもとづく初期化を行う
  ggInit();

#if STEREO == OCULUS
  //
  // Oculus Rift の設定
  //

  // LUID は OpenGL では使っていないらしい
  ovrGraphicsLuid luid;

  // Oculus Rift のセッションを作成する
  if (OVR_FAILURE(ovr_Create(&const_cast<ovrSession>(session), &luid)))
  {
    // 表示用のウィンドウを破棄する
    glfwDestroyWindow(window);
    const_cast<GLFWwindow *>(window) = nullptr;
    return;
  }

  // Oculus Rift の情報を取り出す
  hmdDesc = ovr_GetHmdDesc(session);

#  if defined(_DEBUG)
  // Oculus Rift の情報を表示する
  std::cout
    << "\nProduct name: " << hmdDesc.ProductName
    << "\nResolution:   " << hmdDesc.Resolution.w << " x " << hmdDesc.Resolution.h
    << "\nDefault Fov:  (" << hmdDesc.DefaultEyeFov[ovrEye_Left].LeftTan
    << "," << hmdDesc.DefaultEyeFov[ovrEye_Left].DownTan
    << ") - (" << hmdDesc.DefaultEyeFov[ovrEye_Left].RightTan
    << "," << hmdDesc.DefaultEyeFov[ovrEye_Left].UpTan
    << ")\n              (" << hmdDesc.DefaultEyeFov[ovrEye_Right].LeftTan
    << "," << hmdDesc.DefaultEyeFov[ovrEye_Right].DownTan
    << ") - (" << hmdDesc.DefaultEyeFov[ovrEye_Right].RightTan
    << "," << hmdDesc.DefaultEyeFov[ovrEye_Right].UpTan
    << ")\nMaximum Fov:  (" << hmdDesc.MaxEyeFov[ovrEye_Left].LeftTan
    << "," << hmdDesc.MaxEyeFov[ovrEye_Left].DownTan
    << ") - (" << hmdDesc.MaxEyeFov[ovrEye_Left].RightTan
    << "," << hmdDesc.MaxEyeFov[ovrEye_Left].UpTan
    << ")\n              (" << hmdDesc.MaxEyeFov[ovrEye_Right].LeftTan
    << "," << hmdDesc.MaxEyeFov[ovrEye_Right].DownTan
    << ") - (" << hmdDesc.MaxEyeFov[ovrEye_Right].RightTan
    << "," << hmdDesc.MaxEyeFov[ovrEye_Right].UpTan
    << ")\n" << std::endl;
#  endif

  // Oculus Rift 表示用の FBO を作成する
  for (int eye = 0; eye < ovrEye_Count; ++eye)
  {
    // Oculus Rift 表示用の FBO のサイズ
    const auto renderTargetSize(ovr_GetFovTextureSize(session, ovrEyeType(eye), hmdDesc.DefaultEyeFov[eye], 1.0f));

    // Oculus Rift のレンズ補正等の設定値
    eyeRenderDesc[eye] = ovr_GetRenderDesc(session, ovrEyeType(eye), hmdDesc.DefaultEyeFov[eye]);

    // Oculus Rift 表示用の FBO のカラーバッファとして使うテクスチャセットの作成
    ovrSwapTextureSet *colorTexture;
    ovr_CreateSwapTextureSetGL(session, GL_SRGB8_ALPHA8, renderTargetSize.w, renderTargetSize.h, &colorTexture);

    // Oculus Rift 表示用の FBO のデプスバッファとして使うテクスチャセットの作成
    ovrSwapTextureSet *depthTexture;
    ovr_CreateSwapTextureSetGL(session, GL_DEPTH_COMPONENT32F, renderTargetSize.w, renderTargetSize.h, &depthTexture);

    // Oculus Rift に転送する描画データを作成する
    layerData.Header.Type = ovrLayerType_EyeFovDepth;
    layerData.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // OpenGL なので左下が原点
    layerData.EyeFov.ColorTexture[eye] = colorTexture;
    layerData.EyeFovDepth.DepthTexture[eye] = depthTexture;
    layerData.EyeFov.Viewport[eye].Pos = OVR::Vector2i(0, 0);
    layerData.EyeFov.Viewport[eye].Size = renderTargetSize;
    layerData.EyeFov.Fov[eye] = hmdDesc.DefaultEyeFov[eye];
  }

  // ミラー表示用の FBO を作成する
  if (OVR_SUCCESS(ovr_CreateMirrorTextureGL(session, GL_SRGB8_ALPHA8, width, height, reinterpret_cast<ovrTexture **>(&mirrorTexture))))
  {
    glGenFramebuffers(1, &mirrorFbo);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTexture->OGL.TexId, 0);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  }

  // Oculus Rift のレンダリング用の FBO を作成する
  glGenFramebuffers(ovrEye_Count, oculusFbo);

  // Oculus Rift にレンダリングするときは sRGB カラースペースを使う
  glEnable(GL_FRAMEBUFFER_SRGB);
#endif

  //
  // ウィンドウの設定
  //

  // ウィンドウのサイズ変更時に呼び出す処理を登録する
  glfwSetFramebufferSizeCallback(window, resize);

  // マウスボタンを操作したときの処理を登録する
  glfwSetMouseButtonCallback(window, mouse);

  // マウスホイール操作時に呼び出す処理を登録する
  glfwSetScrollCallback(window, wheel);

  // キーボードを操作した時の処理を登録する
  glfwSetKeyCallback(window, keyboard);

  // マウスカーソルを表示する
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  // このインスタンスの this ポインタを記録しておく
  glfwSetWindowUserPointer(window, this);

  //
  // ジョイスティックの設定
  //

  // ジョイステックの有無を調べて番号を決める
  joy = glfwJoystickPresent(count) ? count : -1;

  // スティックの中立位置を求める
  if (joy >= 0)
  {
    int axesCount;
    const auto *const axes(glfwGetJoystickAxes(joy, &axesCount));

    if (axesCount > 3 + axesOffset)
    {
      // 起動直後のスティックの位置を基準にする
      origin[0] = axes[0];
      origin[1] = axes[1];
      origin[2] = axes[2 + axesOffset];
      origin[3] = axes[3 + axesOffset];
    }
  }

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
  // ウィンドウが開かれていなかったら戻る
  if (!window) return;

  // 参照カウントを減じる
  --count;

#if STEREO == OCULUS
  // Oculus Rift 使用時
  if (session)
  {
    // ミラー表示用の FBO を削除する
    glDeleteFramebuffers(1, &mirrorFbo);

    // ミラー表示に使ったテクスチャを開放する
    glDeleteTextures(1, &mirrorTexture->OGL.TexId);
    ovr_DestroyMirrorTexture(session, reinterpret_cast<ovrTexture *>(mirrorTexture));

    // Oculus Rift 表示用の FBO を削除する
    for (int eye = 0; eye < ovrEye_Count; ++eye)
    {
      // Oculus Rift のレンダリング用の FBO を削除する
      glDeleteFramebuffers(1, &oculusFbo[eye]);

      // レンダリングターゲットに使ったテクスチャを開放する
      auto *const colorTexture(layerData.EyeFov.ColorTexture[eye]);
      for (int i = 0; i < colorTexture->TextureCount; ++i)
      {
        const auto *const ctex(reinterpret_cast<ovrGLTexture *>(&colorTexture->Textures[i]));
        glDeleteTextures(1, &ctex->OGL.TexId);
      }
      ovr_DestroySwapTextureSet(session, colorTexture);

      // デプスバッファとして使ったテクスチャを開放する
      auto *const depthTexture(layerData.EyeFovDepth.DepthTexture[eye]);
      for (int i = 0; i < depthTexture->TextureCount; ++i)
      {
        const auto *const dtex(reinterpret_cast<ovrGLTexture *>(&depthTexture->Textures[i]));
        glDeleteTextures(1, &dtex->OGL.TexId);
      }
      ovr_DestroySwapTextureSet(session, depthTexture);
    }

    // Oculus Rift のセッションを破棄する
    ovr_Destroy(session);
  }
#endif

  // 表示用のウィンドウを閉じる
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

#if STEREO == OCULUS
  // Oculus Rift 使用時
  if (session)
  {
    // フレームのタイミング計測開始
    const auto ftiming(ovr_GetPredictedDisplayTime(session, 0));

    // sensorSampleTime の取得は可能な限り ovr_GetTrackingState() の近くで行う
    layerData.EyeFov.SensorSampleTime = ovr_GetTimeInSeconds();

    // ヘッドトラッキングの状態を取得する
    const auto hmdState(ovr_GetTrackingState(session, ftiming, ovrTrue));

    // 正しい瞳孔感覚をもとに視点の姿勢を取得する
    const ovrVector3f viewOffset[2] = { eyeRenderDesc[0].HmdToEyeViewOffset, eyeRenderDesc[1].HmdToEyeViewOffset };
    ovr_CalcEyePoses(hmdState.HeadPose.ThePose, viewOffset, eyePose);
  }
  else
#endif
  {
    // カラーバッファとデプスバッファを消去する
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }
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

#if STEREO == OCULUS
  // Oculus Rift 使用時
  if (session)
  {
    // Oculus Rift 上の描画位置と拡大率を求める
    ovrViewScaleDesc viewScaleDesc;
    viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
    viewScaleDesc.HmdToEyeViewOffset[0] = eyeRenderDesc[0].HmdToEyeViewOffset;
    viewScaleDesc.HmdToEyeViewOffset[1] = eyeRenderDesc[1].HmdToEyeViewOffset;

    // 描画データを更新する
    layerData.EyeFov.RenderPose[0] = eyePose[0];
    layerData.EyeFov.RenderPose[1] = eyePose[1];

    // 描画データを Oculus Rift に転送する
    const auto *const layers(&layerData.Header);
    const ovrResult result(ovr_SubmitFrame(session, 0, &viewScaleDesc, &layers, 1));
    // 転送に失敗したら Oculus Rift の設定を最初からやり直す必要があるらしい
    // けどめんどくさいのでしない

    // レンダリング結果をミラー表示用のフレームバッファにも転送する
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    const auto w(mirrorTexture->OGL.Header.TextureSize.w);
    const auto h(mirrorTexture->OGL.Header.TextureSize.h);
    glBlitFramebuffer(0, h, w, 0, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    // 残っている OpenGL コマンドを実行する
    glFlush();
  }
  else
#endif
  {
    // カラーバッファを入れ替える
    glfwSwapBuffers(window);
  }

  // イベントを取り出す
  glfwPollEvents();

  //
  // マウスによる操作
  //

  // マウスの位置を調べる
  double x, y;
  glfwGetCursorPos(window, &x, &y);

  // 速度を高度に比例させる
  const float speedFactor((fabs(ez) + 0.2f));

  // 左ボタンドラッグ
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1))
  {
    // カメラの位置を移動する
    const auto speed(static_cast<GLfloat>(cy - y) * speedScale * speedFactor);
    ex += speed * sin(direction);
    ey += speed * cos(direction);

    // カメラの進行方向を変える
    direction += angleScale * static_cast<GLfloat>(x - cx);
  }

  // 右ボタンドラッグ
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2))
  {
    // マウスボタンを押した位置からの変位
    const auto dx(static_cast<GLfloat>(x - cx));
    const auto dy(static_cast<GLfloat>(y - cy));

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


  //
  // ジョイスティックによる操作
  //

  // ジョイスティックが有効なら
  if (joy >= 0)
  {
    // スティック
    int axesCount;
    const auto *const axes(glfwGetJoystickAxes(joy, &axesCount));

    if (axesCount > 3 + axesOffset)
    {
      // スティックの速度係数
      const auto axesSpeedFactor(axesSpeedScale * speedFactor);

      // カメラを前後に移動する
      const auto advSpeed((axes[1] - origin[1]) * axesSpeedFactor);
      ex -= advSpeed * sin(direction);
      ey -= advSpeed * cos(direction);

      // カメラを左右に移動する
      const auto latSpeed((axes[2 + axesOffset] - origin[2]) * axesSpeedFactor);
      ey -= latSpeed * sin(direction);
      ex += latSpeed * cos(direction);

      // カメラを上下に移動する
      ez -= (axes[3 + axesOffset] - origin[3]) * axesSpeedFactor;

      // カメラの進行方向を更新する
      direction += (axes[0] - origin[0]) * axesAngleScale;
    }

    // ボタン
    int btnsCount;
    const auto *const btns(glfwGetJoystickButtons(joy, &btnsCount));
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

  // 右矢印キー操作
  if (glfwGetKey(window, GLFW_KEY_RIGHT))
  {
    // 視差を拡大する
    parallax += parallaxStep;
    updateProjectionMatrix();
  }

  // 左矢印キー操作
  if (glfwGetKey(window, GLFW_KEY_LEFT))
  {
    // 視差を縮小する
    parallax -= parallaxStep;
    updateProjectionMatrix();
  }
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
#if STEREO == OCULUS
    // Oculus Rift 使用時以外
    if (!instance->session)
#endif
    {
      // ディスプレイののアスペクト比を求める
      instance->aspect = static_cast<GLfloat>(width) / static_cast<GLfloat>(height);

      switch (STEREO)
      {
      case SIDEBYSIDE:
        // ウィンドウの横半分をビューポートにする
        width /= 2;
        break;
      case TOPANDBOTTOM:
        // ウィンドウの縦半分をビューポートにする
        height /= 2;
        break;
      default:
        // ウィンドウ全体をビューポートにしておく
        glViewport(0, 0, width, height);
        break;
      }

      // ビューポートの大きさを保存しておく
      instance->width = width;
      instance->height = height;
    }

    // 透視投影変換行列を求める
    instance->updateProjectionMatrix();
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
      // 左ボタンを押した時の処理
    case GLFW_MOUSE_BUTTON_2:
      // 右ボタンを押した時の処理
      if (action)
      {
        // ドラッグ開始位置を保存する
        instance->cx = x;
        instance->cy = y;
      }
      break;
    case GLFW_MOUSE_BUTTON_3:
      // 中ボタンを押した時の処理
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
    // ホイールを横に動かしていたら
    if (fabs(x) > fabs(y))
    {
      // カメラを左右に移動する
      const GLfloat latSpeed((fabs(instance->ez) * 5.0f + 1.0f) * wheelXStep * static_cast<GLfloat>(x));
      instance->ey -= latSpeed * sin(instance->direction);
      instance->ex += latSpeed * cos(instance->direction);
    }
    else
    {
      // シフトキーを押していたら
      if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT))
      {
        // カメラのズーム率を調整する
        instance->zoom += zoomStep * y;

        // 透視投影変換行列を更新する
        instance->updateProjectionMatrix();
      }
      else
      {
        // カメラを上下に移動する
        const GLfloat advSpeed((fabs(instance->ez) * 5.0f + 1.0f) * wheelYStep * static_cast<GLfloat>(y));
        instance->ez += advSpeed;
      }
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
        // 設定をリセットする
        instance->reset();
        instance->updateProjectionMatrix();
        break;
      case GLFW_KEY_P:
        // カメラの位置をリセットする
        instance->ex = startPosition[0];
        instance->ey = startPosition[1];
        instance->ez = startPosition[2];
        instance->direction = 0.0f;
        instance->updateProjectionMatrix();
        break;
      case GLFW_KEY_O:
        // カメラの向きをリセットする
        instance->pitch = startPitch;
      case GLFW_KEY_H:
        // カメラの方位角だけをリセットする
        instance->heading = 0.0f;
        instance->updateProjectionMatrix();
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

//
// 設定値の初期化
//
void Window::reset()
{
  // 物体の位置
  ex = startPosition[0];
  ey = startPosition[1];
  ez = startPosition[2];

  // カメラの進行方向
  direction = 0.0f;

  //　カメラの向き
  heading = 0.0f;
  pitch = startPitch;

  // 物体に対するズーム率
  zoom = initialZoom;

  // 視差
  parallax = STEREO != NONE ? initialParallax : 0.0f;
}

//
// 透視投影変換行列を求める
//
//   ・ウィンドウのサイズ変更時やカメラパラメータの変更時に呼び出す
//
void Window::updateProjectionMatrix()
{
  // ズーム率
  const auto zf(static_cast<GLfloat>(zoom) *zNear);

  // 表示領域
  GLfloat leftL, rightL, bottomL, topL;
  GLfloat leftR, rightR, bottomR, topR;

#if STEREO == OCULUS
  // Oculus Rift 使用時
  if (session)
  {
    // Oculus Rift の左の視野
    const auto fovL(eyeRenderDesc[eyeL].Fov);

    // 片目のウインドウ (表示領域) を調べる
    leftL = -fovL.LeftTan * zf;
    rightL = fovL.RightTan * zf;
    bottomL = -fovL.DownTan * zf;
    topL = fovL.UpTan * zf;

    // Oculus Rift の右の視野
    const auto fovR(eyeRenderDesc[eyeR].Fov);

    // 片目のウインドウ (表示領域) を調べる
    leftR = -fovR.LeftTan * zf;
    rightR = fovR.RightTan * zf;
    bottomR = -fovR.DownTan * zf;
    topR = fovR.UpTan * zf;
  }
  else
#endif
  {
    // スクリーンの幅
    const auto screenWidth(screenHeight * aspect);

    // 視差によるスクリーンのオフセット量
    const auto offset(parallax * zNear / displayDistance);

    // 左の視野
    leftL = (-screenWidth + offset) * zf;
    rightL = (screenWidth + offset) * zf;
    bottomL = -screenHeight * zf;
    topL = screenHeight * zf;

    // Oculus Rift 以外の立体視表示の場合
    if (STEREO != NONE)
    {
      // 右の視野
      leftR = (-screenWidth - offset) * zf;
      rightR = (screenWidth - offset) * zf;
      bottomR = -screenHeight * zf;
      topR = screenHeight * zf;
    }
  }

  // 左のスクリーンのサイズと位置
  screen[eyeL][0] = (rightL - leftL) * 0.5f;
  screen[eyeL][1] = (topL - bottomL) * 0.5f;
  screen[eyeL][2] = (rightL + leftL) * 0.25f;
  screen[eyeL][3] = (topL + bottomL) * 0.25f;

  // 左の透視投影変換行列を求める
  mp[eyeL].loadFrustum(leftL, rightL, bottomL, topL, zNear, zFar);

#if STEREO == OCULUS
  // Oculus Rift 使用時
  if (session)
  {
    // TimeWarp に使う変換行列の成分
    auto &posTimewarpProjectionDesc(layerData.EyeFovDepth.ProjectionDesc);
    posTimewarpProjectionDesc.Projection22 = (mp[eyeL].get()[4 * 2 + 2] + mp[eyeL].get()[4 * 3 + 2]) * 0.5f;
    posTimewarpProjectionDesc.Projection23 = mp[eyeL].get()[4 * 2 + 3] * 0.5f;
    posTimewarpProjectionDesc.Projection32 = mp[eyeL].get()[4 * 3 + 2];
  }
#endif

  // 立体視表示を行うなら
  if (STEREO != NONE)
  {
    // 右のスクリーンのサイズと位置
    screen[eyeR][0] = (rightR - leftR) * 0.5f;
    screen[eyeR][1] = (topR - bottomR) * 0.5f;
    screen[eyeR][2] = (rightR + leftR) * 0.25f;
    screen[eyeR][3] = (topR + bottomR) * 0.25f;

    // 右の透視投影変換行列を求める
    mp[eyeR].loadFrustum(leftR, rightR, bottomR, topR, zNear, zFar);
  }
}

//
// 描画設定
//
//   ・左の図形の描画開始前に呼び出す
//   ・ビューポートの設定などを行う
//
void Window::select(int eye)
{
#if STEREO == OCULUS
  // Oculus Rift 使用時
  if (session)
  {
    // レンダーターゲットに描画する前にレンダーターゲットのインデックスをインクリメントする
    auto *const colorTexture(layerData.EyeFov.ColorTexture[eye]);
    colorTexture->CurrentIndex = (colorTexture->CurrentIndex + 1) % colorTexture->TextureCount;
    auto *const depthTexture(layerData.EyeFovDepth.DepthTexture[eye]);
    depthTexture->CurrentIndex = (depthTexture->CurrentIndex + 1) % depthTexture->TextureCount;

    // レンダーターゲットを切り替える
    glBindFramebuffer(GL_FRAMEBUFFER, oculusFbo[eye]);
    const auto &ctex(reinterpret_cast<ovrGLTexture *>(&colorTexture->Textures[colorTexture->CurrentIndex]));
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ctex->OGL.TexId, 0);
    const auto &dtex(reinterpret_cast<ovrGLTexture *>(&depthTexture->Textures[depthTexture->CurrentIndex]));
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dtex->OGL.TexId, 0);

    // ビューポートを設定して画面クリア
    const auto &vp(layerData.EyeFov.Viewport[eye]);
    glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Oculus Rift の左の位置と向きを取得する
    const auto &o(eyePose[eye].Orientation);
    const auto &p(eyePose[eye].Position);
    const auto &q(eyeRenderDesc[eye].HmdToEyeViewOffset);

    // Oculus Rift の片目の回転を求める
    mo[eye] = ggQuaternionMatrix(GgQuaternion(o.x, o.y, o.z, o.w));

    // Oculus Rift の片目の向きをモデルビュー変換行列に反映する
    mw[eye] = mo[eye].transpose() * ggTranslate(-q.x - p.x, -q.y - p.y, -q.z - p.z) * mv;

    return;
  }
#endif

  // Oculus Rift 以外に表示するとき
  switch (STEREO)
  {
  case TOPANDBOTTOM:
    if (eye == eyeL)
    {
      // ディスプレイの上半分だけに描画する
      glViewport(0, height, width, height);
    }
    else
    {
      // ディスプレイの下半分だけに描画する
      glViewport(0, 0, width, height);
    }
    break;
  case SIDEBYSIDE:
    if (eye == eyeL)
    {
      // ディスプレイの左半分だけに描画する
      glViewport(0, 0, width, height);
    }
    else
    {
      // ディスプレイの右半分だけに描画する
      glViewport(width, 0, width, height);
    }
    break;
  case QUADBUFFER:
    // 左バッファに描画する
    glDrawBuffer(eye == eyeL ? GL_BACK_LEFT :  GL_BACK_RIGHT);
    break;
  default:
    break;
  }

  // ヘッドトラッキングによる視線の回転を行わない
  mo[eye] = ggIdentity();

  // 目をずらす代わりにシーンを動かす
  mw[eye] = ggTranslate(eye == eyeL ? parallax : -parallax, 0.0f, 0.0f) * mv;
}


// 参照カウント
unsigned int Window::count(0);
