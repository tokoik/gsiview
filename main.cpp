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
#  pragma comment(lib, "opencv_core" CV_VERSION_STR CV_EXT_STR)
#  pragma comment(lib, "opencv_highgui" CV_VERSION_STR CV_EXT_STR)
#  pragma comment(lib, "opencv_imgproc" CV_VERSION_STR CV_EXT_STR)
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

  //
  // 終了処理
  //
  void cleanup()
  {
    // GLFW の終了処理
    glfwTerminate();
  }
}

//
// メインプログラム
//
int main()
{
  // GLFW を初期化する
  if (glfwInit() == GL_FALSE)
  {
    // GLFW の初期化に失敗した
#if defined(_WIN32)
    MessageBox(NULL, TEXT("GLFW の初期化に失敗しました。"), TEXT("すまんのう"), MB_OK);
#else
    std::cerr << "Can't initialize GLFW" << std::endl;
#endif
    return EXIT_FAILURE;
  }

  // プログラム終了時の処理を登録する
  atexit(cleanup);

#if STEREO == OCULUS
  // Oculus Rift (LibOVR) を初期化する
  System::Init(Log::ConfigureDefaultLog(LogMask_All));
#endif

  // OpenGL Version 3.2 Core Profile を選択する
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_STEREO, STEREO == QUADBUFFER);

#if STEREO != NONE && !defined(_DEBUG)
  // 接続されているモニタを捜す
  int mcount;
  GLFWmonitor **const monitors = glfwGetMonitors(&mcount);

  // セカンダリモニタがあればそれを使う
  GLFWmonitor *const monitor(monitors[mcount > 1 ? useSecondary : 0]);

  // モニタのモードを調べる
  const GLFWvidmode* mode(glfwGetVideoMode(monitor));

  // フルスクリーンでウィンドウを開く
  Window window(mode->width, mode->height, "GSI 3D Viewer (STEREO)", monitor);
#else
  // ウィンドウモードでウィンドウを開く
  Window window(1280, 800, "GSI 3D Viewer");
#endif
  if (!window.get())
  {
    // ウィンドウが作成できなかった
#if defined(_WIN32)
    MessageBox(NULL, TEXT("GLFW のウィンドウが開けませんでした。"), TEXT("すまんのう"), MB_OK);
#else
    std::cerr << "Can't open GLFW window" << std::endl;
#endif
    exit(1);
  }

  // 隠面消去処理を有効にする
  glEnable(GL_DEPTH_TEST);

  // 背面はカリングしない
  glDisable(GL_CULL_FACE);

  // 背景色を設定する
  glClearColor(back[0], back[1], back[2], back[3]);

  // 描画用のシェーダプログラムを読み込む
  GgSimpleShader simple("simple.vert", "simple.frag");

  // 地形に貼り付けるテクスチャのサンプラの場所を得る
  GLint cmapLoc(glGetUniformLocation(simple.get(), "cmap"));

  // 地形データを読み込む
  GLsizei count;
  const GLuint mesh(loadDem(demfile, &count));
  if (mesh == 0)
  {
    // 地形データが読み込めなかった
#if defined(_WIN32)
    MessageBox(NULL, TEXT("データファイルの読み込みに失敗しました。"), TEXT("すまんのう"), MB_OK);
#else
    std::cerr << "Can't read data file: " << demfile << std::endl;
#endif
    exit(1);
  }

  // 読み込んだ地形データを表示する際のスケール
  const GgMatrix mm(ggScale(demscale));

  // 地形に貼り付けるテクスチャの画像を読み込む
  cv::Mat src(cv::imread(texfile));
  cv::Mat dst(cv::Size(texWidth, texHeight), CV_8UC3);
  if (src.data)
  {
    // 画像が読み込めたら 2D テクスチャに使うサイズに拡大縮小する
    cv::resize(src, dst, dst.size(), cv::INTER_LANCZOS4);
  }
  else
  {
    // 画像が読み込めていなかったら単一色を設定しておく
    dst = cv::Scalar(20, 60, 20);
  }

  // テクスチャメモリを確保して読み込んだ画像を転送する
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0,
    GL_BGR, GL_UNSIGNED_BYTE, dst.data);

  // ミップマップを作成して有効にする
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  // 境界色は黒にしておく (これは Oculus Rift への表示時に表示範囲外の色になる)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

#if USE_ANISOTROPIC_FILTERING
  // 非対象フィルタリング拡張機能を有効にする
  GLfloat fLargest;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
#endif

  // ウィンドウが開いている間くり返し描画する
  while (!window.shouldClose())
  {
    // 画面クリア
    window.clear();

    // 描画用のシェーダプログラムの使用開始
    simple.use();
    simple.setLight(light);
    simple.setMaterial(material);
    glUniform1i(cmapLoc, 0);

    // テクスチャの指定
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    // 図形データの指定
    glBindVertexArray(mesh);

#if STEREO == NONE
    // 単眼のモデルビュープロジェクション変換行列を設定する
    simple.loadMatrix(window.getMp(), window.getMw() * mm);

    // 描画
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
#else
    // 左目のモデルビュープロジェクション変換行列を設定する
    simple.loadMatrix(window.getMpL(), window.getMwL() * mm);

    // 描画
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);

    // 右目のモデルビュープロジェクション変換行列を設定する
    simple.loadMatrix(window.getMpR(), window.getMwR() * mm);

    // 描画
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
#endif

    // バッファを入れ替える
    window.swapBuffers();
  }
}
