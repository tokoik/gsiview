//
// ウィンドウ関連の処理
//
#include <iostream>
#include "Window.h"

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
  , ex(startPosition[0])                  // カメラの x 座標
  , ey(startPosition[1])                  // カメラの y 座標
  , ez(startPosition[2])                  // カメラの z 座標
  , direction(0.0f)                       // カメラの進行方向
  , heading(0.0f)                         // カメラの方位角
  , pitch(0.0f)                           // カメラの仰角
#if STEREO != OCULUS && STEREO != NONE
  , parallax(initialParallax)
#endif
#if STEREO != OCULUS
  , scrH(zNear * screenCenter / screenDistance)
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
  // プログラムオブジェクト, VAO / VBO, Oculus Rift のデバイスマネージャーの作成は最初一度だけ行う
  if (count == 0)
  {
    // Oculus Rift のレンズの歪みを補正するシェーダプログラム
    ocuProgram = ggLoadShader("oculus.vert", "oculus.frag");
    ocuFboColorLoc = glGetUniformLocation(ocuProgram, "ocuFboColor");
    ocuAspectLoc = glGetUniformLocation(ocuProgram, "ocuAspect");
    projectionCenterOffsetLoc = glGetUniformLocation(ocuProgram, "projectionCenterOffset");
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

  // Oculus Rift のデバイスマネージャーが作成できたら情報を取得する
  if (pManager
    && (pHmd = *pManager->EnumerateDevices<HMDDevice>().CreateDevice())
    && pHmd->GetDeviceInfo(&hmdInfo)
    )
  {
#  if defined(_DEBUG)
    // 取得した情報を表示する
    std::cout << hmdInfo.DisplayDeviceName << std::endl;
    std::cout << "\nResolution:"
      << hmdInfo.HResolution << ", "
      << hmdInfo.VResolution << std::endl;
    std::cout << "\nScreen size: "
      << hmdInfo.HScreenSize << ", "
      << hmdInfo.VScreenSize << std::endl;
    std::cout << "\nVertical Screen Center: "
      << hmdInfo.VScreenCenter << std::endl;
    std::cout << "\nEye to Screen Distance: "
      << hmdInfo.EyeToScreenDistance << std::endl;
    std::cout << "\nLens Separation Distance: "
      << hmdInfo.LensSeparationDistance << std::endl;
    std::cout << "\nInterpupillary Distance: "
      << hmdInfo.InterpupillaryDistance << std::endl;
    std::cout << "\nDistortion: "
      << hmdInfo.DistortionK[0] << ", "
      << hmdInfo.DistortionK[1] << ", "
      << hmdInfo.DistortionK[2] << ", "
      << hmdInfo.DistortionK[3] << std::endl;
    std::cout << std::endl;
#  endif

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
  else
  {
    // Oculus Rift をつながずにデバッグする時の設定
    scrW = scrH = zNear * 0.0468f / 0.041f;
    parallax = 0.064f * 0.5f;
    projectionCenterOffset = 1.0f - 2.0f * 0.0635f / 0.14976f;
    lensDistortion[0] = 1.0f;
    lensDistortion[1] = 0.2f;
    lensDistortion[2] = 0.24f;
    lensDistortion[3] = 0.0f;
    ocuAspect = 0.14976f * 0.5f / 0.0936f;
    pSensor = nullptr;
  }

  // レンズの歪み補正に伴う拡大率の補正
  lensScale = 1.0f / (lensDistortion[0] + lensDistortion[1] + lensDistortion[2] + lensDistortion[3]);

  // Oculus Rift の左目用と右目用の FBO の準備
  glGenFramebuffers(2, ocuFbo);

  // Oculus Rift 表示用の FBO のデプスバッファとして使うレンダーバッファの作成
  glGenRenderbuffers(1, &ocuFboDepth);
  glBindRenderbuffer(GL_RENDERBUFFER, ocuFboDepth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fboWidth, fboHeight);

  // Oculus Rift 表示用の FBO のカラーバッファとして使うカラーテクスチャの作成
  glGenTextures(2, ocuFboColor);
  for (int i = 0; i < 2; ++i)
  {
    // 左右の目のそれぞれの表示サイズより少し大きなテクスチャメモリの確保
    glBindTexture(GL_TEXTURE_2D, ocuFboColor[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fboWidth, fboHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

    // 左右の目のそれぞれについて FBO を作成する
    glBindFramebuffer(GL_FRAMEBUFFER, ocuFbo[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
      GL_TEXTURE_2D, ocuFboColor[i], 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
      GL_RENDERBUFFER, ocuFboDepth);
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
  // プログラムオブジェクト, VAO / VBO, Oculus Rift のデバイスマネージャーは最後に削除する
  if (count == 0)
  {
    // プログラムオブジェクトの削除
    glDeleteProgram(ocuProgram);

    // VAO の削除
    glDeleteBuffers(1, &ocuVbo);
    glDeleteVertexArrays(1, &ocuVao);
  }

  // FBO の削除
  glDeleteTextures(1, &ocuFboDepth);
  glDeleteTextures(2, ocuFboColor);
  glDeleteFramebuffers(2, ocuFbo);
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

//
// カラーバッファを入れ替えてイベントを取り出す
//
//   ・図形の描画終了後に呼び出す
//   ・ダブルバッファリングのバッファの入れ替えを行う
//   ・キーボード操作等のイベントを取り出す
//
void Window::swapBuffers()
{
#if STEREO == OCULUS
  // ディスプレイに描く
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDrawBuffer(GL_BACK);

  // 隠面消去処理は行わない
  glDisable(GL_DEPTH_TEST);

  // 表示領域を覆う矩形
  glBindVertexArray(ocuVao);

  // Oculus Rift のレンズ補正用シェーダ
  glUseProgram(ocuProgram);

  // FBO に描画した結果を参照するテクスチャユニット
  glActiveTexture(GL_TEXTURE0);
  glUniform1i(ocuFboColorLoc, 0);

  // Oculus Rift のアスペクト比
  glUniform1f(ocuAspectLoc, ocuAspect);

  // レンズの歪みの補正係数
  glUniform4fv(lensDistortionLoc, 1, lensDistortion);

  // レンズの拡大率の補正係数
  glUniform1f(lensScaleLoc, lensScale);

  // 左目の描画
  glUniform1f(projectionCenterOffsetLoc, -projectionCenterOffset);
  glViewport(0, 0, winW, winH);
  glBindTexture(GL_TEXTURE_2D, ocuFboColor[0]);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  // 右目の描画
  glUniform1f(projectionCenterOffsetLoc, projectionCenterOffset);
  glViewport(winW, 0, winW, winH);
  glBindTexture(GL_TEXTURE_2D, ocuFboColor[1]);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
#endif

  // エラーチェック
  ggError("SwapBuffers");

  // カラーバッファを入れ替える
  glfwSwapBuffers(window);

  // イベントを取り出す
  glfwPollEvents();

  // マウスの位置を調べる
  double x, y;
  glfwGetCursorPos(window, &x, &y);

  // 速度を高度に比例させる
  const double factor((fabs(ez) * 0.5f + 1.0f));

  // 左ボタンドラッグ
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1))
  {
    // カメラの位置を移動する
    const GLfloat speed(static_cast<GLfloat>(cy - y) * speedScale * factor);
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
      // カメラを前後に移動する
      const GLfloat advSpeed((axes[1] - origin[1]) * axesSpeedScale * factor);
      ex -= advSpeed * sin(direction);
      ey -= advSpeed * cos(direction);

      // カメラを左右に移動する
      const GLfloat latSpeed((axes[2 + axesOffset] - origin[2]) * axesSpeedScale * factor);
      ey -= latSpeed * sin(direction);
      ex += latSpeed * cos(direction);

      // カメラを上下に移動する
      ez -= (axes[3 + axesOffset] - origin[3]) * axesSpeedScale * factor;

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
  // 左矢印キー操作
  if (glfwGetKey(window, GLFW_KEY_LEFT))
  {
#  if STEREO == OCULUS
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT))
    {
      // レンズ間隔を縮小する
      projectionCenterOffset -= projectionCenterOffsetStep;
    }
    else
    {
      // 視差を縮小する
      parallax -= parallaxStep;
      updateStereoProjectionMatrix();
    }
#  else
    // 視差を縮小する
    parallax -= parallaxStep;
    updateStereoProjectionMatrix();
#  endif
  }

  // 右矢印キー操作
  if (glfwGetKey(window, GLFW_KEY_RIGHT))
  {
#  if STEREO == OCULUS
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT))
    {
      // レンズ間隔を拡大する
      projectionCenterOffset += projectionCenterOffsetStep;
    }
    else
    {
      // 視差を拡大する
      parallax += parallaxStep;
      updateStereoProjectionMatrix();
    }
#  else
    // 視差を拡大する
    parallax += parallaxStep;
    updateStereoProjectionMatrix();
#  endif
  }

#  if STEREO == OCULUS
  // 左矢印キー操作
  if (glfwGetKey(window, GLFW_KEY_DOWN))
  {
    // レンズの拡大率の補正係数を下げる
    lensScale -= lensScaleStep;
  }

  // 右矢印キー操作
  if (glfwGetKey(window, GLFW_KEY_UP))
  {
    // レンズの拡大率の補正係数を上げる
    lensScale += lensScaleStep;
  }
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
#endif

#if STEREO == SIDEBYSIDE || STEREO == OCULUS
    // ウィンドウの横半分をビューポートにする
    width /= 2;
#elif STEREO == TOPANDBOTTOM
    // ウィンドウの縦半分をビューポートにする
    height /= 2;
#else
    // ウィンドウ全体をビューポートにする
    glViewport(0, 0, width, height);
#endif

#if STEREO == NONE
    // 投影変換行列を求める
    instance->updateProjectionMatrix();
#else
    // ステレオ表示の時はビューポートの大きさを保存しておく
    instance->winW = width;
    instance->winH = height;

    // 投影変換行列を求める
    instance->updateStereoProjectionMatrix();
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
GgMatrix Window::getMvL() const
{
#  if STEREO == LINEBYLINE
  // 偶数番目の走査線だけに描画する
#  elif STEREO == TOPANDBOTTOM
  // ディスプレイの上半分だけに描画する
  glViewport(0, winH, winW, winH);
#  elif STEREO == SIDEBYSIDE
  // ディスプレイの左半分だけに描画する
  glViewport(0, 0, winW, winH);
#  elif STEREO == QUADBUFFER
  // 左目用バッファに描画する
  glDrawBuffer(GL_BACK_LEFT);
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
  // 奇数番目の走査線だけに描画する
#  elif STEREO == TOPANDBOTTOM
  // ディスプレイの下半分だけに描画する
  glViewport(0, 0, winW, winH);
#  elif STEREO == SIDEBYSIDE
  // ディスプレイの右半分だけに描画する
  glViewport(winW, 0, winW, winH);
#  elif STEREO == QUADBUFFER
  // 右目用バッファに描画する
  glDrawBuffer(GL_BACK_RIGHT);
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

#  if STEREO == OCULUS
// Oculus Rift 表示に使う矩形
GLuint Window::ocuVao, Window::ocuVbo;

// Oculus Rift 表示用のシェーダプログラム
GLuint Window::ocuProgram;

// Oculus Rift 表示用の FBO のテクスチャユニットの uniform 変数の場所
GLint Window::ocuFboColorLoc;

// Oculus Rift の画面のアスペクト比の uniform 変数の場所
GLint Window::ocuAspectLoc;

// Oculus Rift のレンズの中心の画面の中心からのずれの uniform 変数の場所
GLint Window::projectionCenterOffsetLoc;

// Oculus Rift のレンズの歪みの補正係数の uniform 変数の場所
GLint Window::lensDistortionLoc;

// Oculus Rift のレンズの拡大率の補正係数の uniform 変数の場所
GLint Window::lensScaleLoc;

// Oculus Rift 表示用の FBO のレンダーターゲット
const GLenum Window::ocuFboDrawBuffers[] = { GL_COLOR_ATTACHMENT0 };

// Oculus Rift のヘッドトラッキングセンサ
Ptr<DeviceManager> Window::pManager;
#  endif
#endif

// 参照カウント
unsigned int Window::count(0);
