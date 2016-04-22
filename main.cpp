//
// �n���@�n�} 3D �r���[�A
//

// OpenCV �̑g�ݍ���
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

// �W�����C�u����
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

// �E�B���h�E�֘A�̏���
#include "Window.h"

// �V�F�[�_
#define SHADER "simple"

namespace
{
  //
  // �f�[�^�t�@�C�� (Digital Elevation Model) �̓ǂݍ���
  //
  GLuint loadDem(const char *filename, GLsizei *count)
  {
    // �f�[�^�t�@�C�����J��
    std::ifstream file(filename);
    if (!file) return 0;

    // ���b�V���̕�����
    unsigned int slices(0), stacks(0);

    // �����f�[�^�̊i�[��
    std::vector<GLfloat> height;

    // �f�[�^�̓ǂݍ���
    std::string line;
    while (getline(file, line))
    {
      std::istringstream sline(line);
      ++stacks;

      // �����l����ǂݎ��
      GLfloat h;
      for (slices = 0; sline >> h;)
      {
        // �f�[�^��ۑ�����
        height.push_back(h);

        // �ǂݍ��񂾐��l�̐��𐔂���
        ++slices;

        // �R���}�^�s��������ǂݔ�΂�
        char c;
        sline >> c;
      }
    }

    // �f�[�^���̃`�F�b�N
    if (height.size() < 4 || slices * stacks != height.size()) return 0;

    // �c���̊i�q�_�̐��ł͂Ȃ��Ԋu�̐��ɂ���
    --slices;
    --stacks;

    // �f�[�^�̊i�[��
    std::vector<GLfloat> position, normal;

    // ���_�̖@���x�N�g�������߂�
    for (unsigned int k = 0, j = 0; j <= stacks; ++j)
    {
      for (unsigned int i = 0; i <= slices; ++i, ++k)
      {
        // �����Ώۂ̒��_�̎��͂̒��_�ԍ�
        const unsigned int kim = i > 0 ? k - 1 : k;
        const unsigned int kip = i < slices ? k + 1 : k;
        const unsigned int kjm = j > 0 ? k - slices - 1 : k;
        const unsigned int kjp = j < stacks ? k + slices + 1 : k;

        // �ʒu
        position.push_back(static_cast<GLfloat>(i) * 2.0f / static_cast<GLfloat>(slices) - 1.0f);
        position.push_back(1.0f - static_cast<GLfloat>(j) * 2.0f / static_cast<GLfloat>(stacks));
        position.push_back(height[k]);

        // �@��
        const GLfloat n[] =
        {
          (height[kim] - height[kip]) / static_cast<GLfloat>(stacks),
          (height[kjp] - height[kjm]) / static_cast<GLfloat>(slices),
          2.0f / (static_cast<GLfloat>(slices * stacks))
        };

        // �@���x�N�g���𐳋K�����ēo�^����
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

    // ���_�̃C���f�b�N�X (�ʃf�[�^)
    std::vector<GLuint> index;

    // ���_�̃C���f�b�N�X�����߂�
    for (unsigned int j = 0; j < stacks; ++j)
    {
      for (unsigned int i = 0; i < slices; ++i)
      {
        const int k((slices + 1) * j + i);

        // �㔼���̎O�p�`
        index.push_back(k);
        index.push_back(k + slices + 2);
        index.push_back(k + 1);

        // �������̎O�p�`
        index.push_back(k);
        index.push_back(k + slices + 1);
        index.push_back(k + slices + 2);
      }
    }

    // ���_�z��I�u�W�F�N�g
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // �����̒��_�o�b�t�@�I�u�W�F�N�g
    GLuint positionBuffer;
    glGenBuffers(1, &positionBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, position.size() * sizeof (GLfloat), &position[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // �@���̒��_�o�b�t�@�I�u�W�F�N�g
    GLuint normalBuffer;
    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, normal.size() * sizeof (GLfloat), &normal[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // ���_�̃C���f�b�N�X�o�b�t�@�I�u�W�F�N�g
    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index.size() * sizeof (GLuint), &index[0], GL_STATIC_DRAW);

    // �C���f�b�N�X�̐���Ԃ�
    *count = static_cast<GLsizei>(index.size());
    return vao;
  }
}

#if STEREO == OCULUS
namespace
{
  // Oculus Rift �̃Z�b�V�����f�[�^
  ovrSession session(nullptr);

  // Oculus Rift �̃Z�b�V������j������
  void destroySession()
  {
    ovr_Destroy(session);
  }
}
#endif

//
// ���C���v���O����
//
int main()
{
#if STEREO == OCULUS
  // Oculus Rift (LibOVR) ������������
  if (OVR_FAILURE(ovr_Initialize(nullptr)))
  {
    // Oculus Rift �̏������Ɏ��s����
    MessageBox(nullptr, TEXT("Oculus Rift ���������ł��܂���B"), TEXT("���܂�̂�"), MB_OK);
    return EXIT_FAILURE;
  }

  // �v���O�����I�����ɂ� LibOVR ���I������
  atexit(ovr_Shutdown);

  // Oculus Rift �̃f�o�C�X���쐬����
  ovrGraphicsLuid luid; // LUID �� OpenGL �ł͎g���Ă��Ȃ��炵��
  if (OVR_FAILURE(ovr_Create(&session, &luid)))
  {
    // Oculus Rift �̃f�o�C�X���쐬�ł��Ȃ�
    MessageBox(nullptr, TEXT("Oculus Rift ���g�p�ł��܂���B"), TEXT("���܂�̂�"), MB_OK);
    return EXIT_FAILURE;
  }

  // �v���O�����I�����ɂ̓Z�b�V������j������
  atexit(destroySession);
#endif

  // GLFW ������������
  if (glfwInit() == GL_FALSE)
  {
    // GLFW �̏������Ɏ��s����
    MessageBox(nullptr, TEXT("GLFW �̏������Ɏ��s���܂����B"), TEXT("���܂�̂�"), MB_OK);
    return EXIT_FAILURE;
  }

  // �v���O�����I�����ɂ� GLFW ���I������
  atexit(glfwTerminate);

  // OpenGL �E�B���h�E�̓���
  glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
  glfwWindowHint(GLFW_STEREO, STEREO == QUADBUFFER);
  glfwWindowHint(GLFW_DOUBLEBUFFER, STEREO != OCULUS);

  // �f�B�X�v���C�̏��
  GLFWmonitor *monitor;
  int window_width, window_height;

  // �t���X�N���[���\��
  if (STEREO != NONE && STEREO != OCULUS && !debug)
  {
    // �ڑ�����Ă��郂�j�^�̐��𐔂���
    int mcount;
    GLFWmonitor **const monitors = glfwGetMonitors(&mcount);

    // �Z�J���_�����j�^������΂�����g��
    monitor = monitors[mcount > useSecondary ? useSecondary : 0];

    // ���j�^�̃��[�h�𒲂ׂ�
    const GLFWvidmode *mode(glfwGetVideoMode(monitor));

    // �E�B���h�E�̃T�C�Y (�t���X�N���[��)
    window_width = mode->width;
    window_height = mode->height;
  }
  else
  {
    // �v���C�}�����j�^���E�B���h�E���[�h�Ŏg��
    monitor = nullptr;

    // �E�B���h�E�̃T�C�Y
    window_width = 960;
    window_height = 540;
  }

  // �E�B���h�E���J��
  Window window(window_width, window_height, "STER Display", monitor, nullptr
#if STEREO == OCULUS
    , session
#endif
    );
  if (!window.get())
  {
    // �E�B���h�E���쐬�ł��Ȃ�����
#if defined(_WIN32)
    MessageBox(nullptr, TEXT("GLFW �̃E�B���h�E���J���܂���ł����B"), TEXT("���܂�̂�"), MB_OK);
#else
    std::cerr << "Can't open GLFW window." << std::endl;
#endif
    return EXIT_FAILURE;
  }

  // �`��p�̃V�F�[�_�v���O������ǂݍ���
  GgSimpleShader shader(SHADER ".vert", SHADER ".frag");
  if (!shader.get())
  {
    // �V�F�[�_���ǂݍ��߂Ȃ�����
#if defined(_WIN32)
    MessageBox(nullptr, TEXT("�V�F�[�_�t�@�C���̓ǂݍ��݂Ɏ��s���܂����B"), TEXT("���܂�̂�"), MB_OK);
#else
    std::cerr << "Can't read shader file: " SHADER ".vert, " SHADER ".frag" << demfile << std::endl;
#endif
    return EXIT_FAILURE;
  }

  // �n�`�ɓ\��t����e�N�X�`���̃T���v���̏ꏊ�𓾂�
  GLint cmapLoc(glGetUniformLocation(shader.get(), "cmap"));

  // �n�`�f�[�^��ǂݍ���
  GLsizei count;
  const GLuint mesh(loadDem(demfile, &count));
  if (mesh == 0)
  {
    // �n�`�f�[�^���ǂݍ��߂Ȃ�����
#if defined(_WIN32)
    MessageBox(nullptr, TEXT("�f�[�^�t�@�C���̓ǂݍ��݂Ɏ��s���܂����B"), TEXT("���܂�̂�"), MB_OK);
#else
    std::cerr << "Can't read data file: " << demfile << std::endl;
#endif
    return EXIT_FAILURE;
  }

  // �ǂݍ��񂾒n�`�f�[�^��\������ۂ̃X�P�[��
  const GgMatrix mm(ggScale(demscale));

  // �n�`�ɓ\��t����e�N�X�`��
  GLuint tex(0);

  // �n�`�ɓ\��t����e�N�X�`���̉摜��ǂݍ���
  cv::Mat src(cv::imread(texfile));
  if (src.data)
  {
    // �e�N�X�`���ɓǂݍ��񂾉摜��]������
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, src.cols, src.rows, 0,
      GL_BGR, GL_UNSIGNED_BYTE, src.data);

    // �~�b�v�}�b�v���쐬���ėL���ɂ���
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // ���E�F�͍��ɂ��Ă��� (����� Oculus Rift �ւ̕\�����ɕ\���͈͊O�̐F�ɂȂ�)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
  }

#if USE_ANISOTROPIC_FILTERING
  // ��Ώۃt�B���^�����O�g���@�\��L���ɂ���
  GLfloat fLargest;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
#endif

  // �B�ʏ���������L���ɂ���
  glEnable(GL_DEPTH_TEST);

  // �w�ʂ̓J�����O���Ȃ�
  glDisable(GL_CULL_FACE);

  // �w�i�F��ݒ肷��
  glClearColor(back[0], back[1], back[2], back[3]);

  // �E�B���h�E���J���Ă���Ԃ���Ԃ��`�悷��
  while (!window.shouldClose())
  {
    // ��ʃN���A
    window.clear();

    for (int eye = 0; eye < (STEREO == NONE ? 1 : 2); ++eye)
    {
      // �`��̈��I������
      window.select(eye);

      // �`��p�̃V�F�[�_�v���O�����̎g�p�J�n
      shader.use();
      shader.setLight(light);
      shader.setMaterial(material);
      glUniform1i(cmapLoc, 0);

      // �e�N�X�`���̎w��
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, tex);

      // ���ڂ̃��f���r���[�v���W�F�N�V�����ϊ��s���ݒ肷��
      shader.loadMatrix(window.getMp(eye), window.getMw(eye) * mm);

      // �}�`�f�[�^�̎w��
      glBindVertexArray(mesh);

      // �`��
      glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
    }

    // �o�b�t�@�����ւ���
    window.swapBuffers();
  }
}
