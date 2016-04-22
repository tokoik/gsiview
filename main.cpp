//
// 地理院地図 3D ビューア
//

// OpenCV の組み込み
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#if defined(_WIN32)
#  include <windows.h>
#  define _USE_MATH_DEFINES
#  define CV_VERSION_STR CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)
#  if defined(_DEBUG)
#    define CV_EXT_STR "d.lib"
#  else
#    define CV_EXT_STR ".lib"
#  endif
#  pragma comment(lib, "ws2_32.lib")
#  if CV_MAJOR_VERSION < 3
#    pragma comment(lib, "IlmImf" CV_EXT_STR)
#    pragma comment(lib, "libjpeg" CV_EXT_STR)
#    pragma comment(lib, "libjasper" CV_EXT_STR)
#    pragma comment(lib, "libpng" CV_EXT_STR)
#    pragma comment(lib, "libtiff" CV_EXT_STR)
#    pragma comment(lib, "zlib" CV_EXT_STR)
#    pragma comment(lib, "opencv_core" CV_VERSION_STR CV_EXT_STR)
#    pragma comment(lib, "opencv_highgui" CV_VERSION_STR CV_EXT_STR)
#    pragma comment(lib, "opencv_imgproc" CV_VERSION_STR CV_EXT_STR)
#  else
#    pragma comment(lib, "opencv_world" CV_VERSION_STR CV_EXT_STR)
#  endif
#endif

// 標準ライブラリ
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

// ウィンドウ関連の処理
#include "Window.h"

// シェーダ
#define SHADER "simple"

namespace
{
  //
  // データファイル (Digital Elevation Model) の読み込み
  //
  GLuint loadDem(const char *filename, GLsizei *count)
  {
    // データファイルを開く
    std::ifstream file(filename);
    if (!file) return 0;

    // メッシュの分割数
    unsigned int slices(0), stacks(0);

    // 高さデータの格納先
    std::vector<GLfloat> height;

    // データの読み込み
    std::string line;
    while (getline(file, line))
    {
      std::istringstream sline(line);
      ++stacks;

      // 実数値を一つ読み取る
      GLfloat h;
      for (slices = 0; sline >> h;)
      {
        // データを保存する
        height.push_back(h);

        // 読み込んだ数値の数を数える
        ++slices;

        // コンマ／行末文字を読み飛ばす
        char c;
        sline >> c;
      }
    }

    // データ数のチェック
    if (height.size() < 4 || slices * stacks != height.size()) return 0;

    // 縦横の格子点の数ではなく間隔の数にする
    --slices;
    --stacks;

    // データの格納先
    std::vector<GLfloat> position, normal;

    // 頂点の法線ベクトルを求める
    for (unsigned int k = 0, j = 0; j <= stacks; ++j)
    {
      for (unsigned int i = 0; i <= slices; ++i, ++k)
      {
        // 処理対象の頂点の周囲の頂点番号
        const unsigned int kim = i > 0 ? k - 1 : k;
        const unsigned int kip = i < slices ? k + 1 : k;
        const unsigned int kjm = j > 0 ? k - slices - 1 : k;
        const unsigned int kjp = j < stacks ? k + slices + 1 : k;

        // 位置
        position.push_back(static_cast<GLfloat>(i) * 2.0f / static_cast<GLfloat>(slices) - 1.0f);
        position.push_back(1.0f - static_cast<GLfloat>(j) * 2.0f / static_cast<GLfloat>(stacks));
        position.push_back(height[k]);

        // 法線
        const GLfloat n[] =
        {
          (height[kim] - height[kip]) / static_cast<GLfloat>(stacks),
          (height[kjp] - height[kjm]) / static_cast<GLfloat>(slices),
          2.0f / (static_cast<GLfloat>(slices * stacks))
        };

        // 法線ベクトルを正規化して登録する
        const GLfloat l = n[0] * n[0] + n[1] * n[1] + n[2] * n[2];
        if (l > 0.0f)
        {
          normal.push_back(n[0] / l);
          normal.push_back(n[1] / l);
          normal.push_back(n[2] / l);
        }
        else{
          normal.push_back(0.0f);
          normal.push_back(0.0f);
          normal.push_back(0.0f);
        }
      }
    }

    // 頂点のインデックス (面データ)
    std::vector<GLuint> index;

    // 頂点のインデックスを求める
    for (unsigned int j = 0; j < stacks; ++j)
    {
      for (unsigned int i = 0; i < slices; ++i)
      {
        const int k((slices + 1) * j + i);

        // 上半分の三角形
        index.push_back(k);
        index.push_back(k + slices + 2);
        index.push_back(k + 1);

        // 下半分の三角形
        index.push_back(k);
        index.push_back(k + slices + 1);
        index.push_back(k + slices + 2);
      }
    }

    // 頂点配列オブジェクト
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // 高さの頂点バッファオブジェクト
    GLuint positionBuffer;
    glGenBuffers(1, &positionBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, position.size() * sizeof (GLfloat), &position[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // 法線の頂点バッファオブジェクト
    GLuint normalBuffer;
    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, normal.size() * sizeof (GLfloat), &normal[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // 頂点のインデックスバッファオブジェクト
    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index.size() * sizeof (GLuint), &index[0], GL_STATIC_DRAW);

    // インデックスの数を返す
    *count = static_cast<GLsizei>(index.size());
    return vao;
  }
}

#if STEREO == OCULUS
namespace
{
  // Oculus Rift のセッションデータ
  ovrSession session(nullptr);

  // Oculus Rift のセッションを破棄する
  void destroySession()
  {
    ovr_Destroy(session);
  }
}
#endif

//
// メインプログラム
//
int main()
{
#if STEREO == OCULUS
  // Oculus Rift (LibOVR) を初期化する
  if (OVR_FAILURE(ovr_Initialize(nullptr)))
  {
    // Oculus Rift の初期化に失敗した
    MessageBox(nullptr, TEXT("Oculus Rift が初期化できません。"), TEXT("すまんのう"), MB_OK);
    return EXIT_FAILURE;
  }

  // プログラム終了時には LibOVR を終了する
  atexit(ovr_Shutdown);

  // Oculus Rift のデバイスを作成する
  ovrGraphicsLuid luid; // LUID は OpenGL では使っていないらしい
  if (OVR_FAILURE(ovr_Create(&session, &luid)))
  {
    // Oculus Rift のデバイスが作成できない
    MessageBox(nullptr, TEXT("Oculus Rift が使用できません。"), TEXT("すまんのう"), MB_OK);
    return EXIT_FAILURE;
  }

  // プログラム終了時にはセッションを破棄する
  atexit(destroySession);
#endif

  // GLFW を初期化する
  if (glfwInit() == GL_FALSE)
  {
    // GLFW の初期化に失敗した
    MessageBox(nullptr, TEXT("GLFW の初期化に失敗しました。"), TEXT("すまんのう"), MB_OK);
    return EXIT_FAILURE;
  }

  // プログラム終了時には GLFW を終了する
  atexit(glfwTerminate);

  // OpenGL ウィンドウの特性
  glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
  glfwWindowHint(GLFW_STEREO, STEREO == QUADBUFFER);
  glfwWindowHint(GLFW_DOUBLEBUFFER, STEREO != OCULUS);

  // ディスプレイの情報
  GLFWmonitor *monitor;
  int window_width, window_height;

  // フルスクリーン表示
  if (STEREO != NONE && STEREO != OCULUS && !debug)
  {
    // 接続されているモニタの数を数える
    int mcount;
    GLFWmonitor **const monitors = glfwGetMonitors(&mcount);

    // セカンダリモニタがあればそれを使う
    monitor = monitors[mcount > useSecondary ? useSecondary : 0];

    // モニタのモードを調べる
    const GLFWvidmode *mode(glfwGetVideoMode(monitor));

    // ウィンドウのサイズ (フルスクリーン)
    window_width = mode->width;
    window_height = mode->height;
  }
  else
  {
    // プライマリモニタをウィンドウモードで使う
    monitor = nullptr;

    // ウィンドウのサイズ
    window_width = 960;
    window_height = 540;
  }

  // ウィンドウを開く
  Window window(window_width, window_height, "STER Display", monitor, nullptr
#if STEREO == OCULUS
    , session
#endif
    );
  if (!window.get())
  {
    // ウィンドウが作成できなかった
#if defined(_WIN32)
    MessageBox(nullptr, TEXT("GLFW のウィンドウが開けませんでした。"), TEXT("すまんのう"), MB_OK);
#else
    std::cerr << "Can't open GLFW window." << std::endl;
#endif
    return EXIT_FAILURE;
  }

  // 描画用のシェーダプログラムを読み込む
  GgSimpleShader shader(SHADER ".vert", SHADER ".frag");
  if (!shader.get())
  {
    // シェーダが読み込めなかった
#if defined(_WIN32)
    MessageBox(nullptr, TEXT("シェーダファイルの読み込みに失敗しました。"), TEXT("すまんのう"), MB_OK);
#else
    std::cerr << "Can't read shader file: " SHADER ".vert, " SHADER ".frag" << demfile << std::endl;
#endif
    return EXIT_FAILURE;
  }

  // 地形に貼り付けるテクスチャのサンプラの場所を得る
  GLint cmapLoc(glGetUniformLocation(shader.get(), "cmap"));

  // 地形データを読み込む
  GLsizei count;
  const GLuint mesh(loadDem(demfile, &count));
  if (mesh == 0)
  {
    // 地形データが読み込めなかった
#if defined(_WIN32)
    MessageBox(nullptr, TEXT("データファイルの読み込みに失敗しました。"), TEXT("すまんのう"), MB_OK);
#else
    std::cerr << "Can't read data file: " << demfile << std::endl;
#endif
    return EXIT_FAILURE;
  }

  // 読み込んだ地形データを表示する際のスケール
  const GgMatrix mm(ggScale(demscale));

  // 地形に貼り付けるテクスチャ
  GLuint tex(0);

  // 地形に貼り付けるテクスチャの画像を読み込む
  cv::Mat src(cv::imread(texfile));
  if (src.data)
  {
    // テクスチャに読み込んだ画像を転送する
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, src.cols, src.rows, 0,
      GL_BGR, GL_UNSIGNED_BYTE, src.data);

    // ミップマップを作成して有効にする
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // 境界色は黒にしておく (これは Oculus Rift への表示時に表示範囲外の色になる)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
  }

#if USE_ANISOTROPIC_FILTERING
  // 非対象フィルタリング拡張機能を有効にする
  GLfloat fLargest;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
#endif

  // 隠面消去処理を有効にする
  glEnable(GL_DEPTH_TEST);

  // 背面はカリングしない
  glDisable(GL_CULL_FACE);

  // 背景色を設定する
  glClearColor(back[0], back[1], back[2], back[3]);

  // ウィンドウが開いている間くり返し描画する
  while (!window.shouldClose())
  {
    // 画面クリア
    window.clear();

    for (int eye = 0; eye < (STEREO == NONE ? 1 : 2); ++eye)
    {
      // 描画領域を選択する
      window.select(eye);

      // 描画用のシェーダプログラムの使用開始
      shader.use();
      shader.setLight(light);
      shader.setMaterial(material);
      glUniform1i(cmapLoc, 0);

      // テクスチャの指定
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, tex);

      // 左目のモデルビュープロジェクション変換行列を設定する
      shader.loadMatrix(window.getMp(eye), window.getMw(eye) * mm);

      // 図形データの指定
      glBindVertexArray(mesh);

      // 描画
      glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
    }

    // バッファを入れ替える
    window.swapBuffers();
  }
}
