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
#  pragma comment(lib, "opencv_core" CV_VERSION_STR CV_EXT_STR)
#  pragma comment(lib, "opencv_highgui" CV_VERSION_STR CV_EXT_STR)
#  pragma comment(lib, "opencv_imgproc" CV_VERSION_STR CV_EXT_STR)
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

  //
  // �I������
  //
  void cleanup()
  {
    // GLFW �̏I������
    glfwTerminate();
  }
}

//
// ���C���v���O����
//
int main()
{
  // GLFW ������������
  if (glfwInit() == GL_FALSE)
  {
    // GLFW �̏������Ɏ��s����
#if defined(_WIN32)
    MessageBox(NULL, TEXT("GLFW �̏������Ɏ��s���܂����B"), TEXT("���܂�̂�"), MB_OK);
#else
    std::cerr << "Can't initialize GLFW" << std::endl;
#endif
    return EXIT_FAILURE;
  }

  // �v���O�����I�����̏�����o�^����
  atexit(cleanup);

#if STEREO == OCULUS
  // Oculus Rift (LibOVR) ������������
  System::Init(Log::ConfigureDefaultLog(LogMask_All));
#endif

  // OpenGL Version 3.2 Core Profile ��I������
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_STEREO, STEREO == QUADBUFFER);

#if STEREO != NONE && !defined(_DEBUG)
  // �ڑ�����Ă��郂�j�^��{��
  int mcount;
  GLFWmonitor **const monitors = glfwGetMonitors(&mcount);

  // �Z�J���_�����j�^������΂�����g��
  GLFWmonitor *const monitor(monitors[mcount > 1 ? useSecondary : 0]);

  // ���j�^�̃��[�h�𒲂ׂ�
  const GLFWvidmode* mode(glfwGetVideoMode(monitor));

  // �t���X�N���[���ŃE�B���h�E���J��
  Window window(mode->width, mode->height, "GSI 3D Viewer (STEREO)", monitor);
#else
  // �E�B���h�E���[�h�ŃE�B���h�E���J��
  Window window(1280, 800, "GSI 3D Viewer");
#endif
  if (!window.get())
  {
    // �E�B���h�E���쐬�ł��Ȃ�����
#if defined(_WIN32)
    MessageBox(NULL, TEXT("GLFW �̃E�B���h�E���J���܂���ł����B"), TEXT("���܂�̂�"), MB_OK);
#else
    std::cerr << "Can't open GLFW window" << std::endl;
#endif
    exit(1);
  }

  // �B�ʏ���������L���ɂ���
  glEnable(GL_DEPTH_TEST);

  // �w�ʂ̓J�����O���Ȃ�
  glDisable(GL_CULL_FACE);

  // �w�i�F��ݒ肷��
  glClearColor(back[0], back[1], back[2], back[3]);

  // �`��p�̃V�F�[�_�v���O������ǂݍ���
  GgSimpleShader simple("simple.vert", "simple.frag");

  // �n�`�ɓ\��t����e�N�X�`���̃T���v���̏ꏊ�𓾂�
  GLint cmapLoc(glGetUniformLocation(simple.get(), "cmap"));

  // �n�`�f�[�^��ǂݍ���
  GLsizei count;
  const GLuint mesh(loadDem(demfile, &count));
  if (mesh == 0)
  {
    // �n�`�f�[�^���ǂݍ��߂Ȃ�����
#if defined(_WIN32)
    MessageBox(NULL, TEXT("�f�[�^�t�@�C���̓ǂݍ��݂Ɏ��s���܂����B"), TEXT("���܂�̂�"), MB_OK);
#else
    std::cerr << "Can't read data file: " << demfile << std::endl;
#endif
    exit(1);
  }

  // �ǂݍ��񂾒n�`�f�[�^��\������ۂ̃X�P�[��
  const GgMatrix mm(ggScale(demscale));

  // �n�`�ɓ\��t����e�N�X�`���̉摜��ǂݍ���
  cv::Mat src(cv::imread(texfile));
  cv::Mat dst(cv::Size(texWidth, texHeight), CV_8UC3);
  if (src.data)
  {
    // �摜���ǂݍ��߂��� 2D �e�N�X�`���Ɏg���T�C�Y�Ɋg��k������
    cv::resize(src, dst, dst.size(), cv::INTER_LANCZOS4);
  }
  else
  {
    // �摜���ǂݍ��߂Ă��Ȃ�������P��F��ݒ肵�Ă���
    dst = cv::Scalar(20, 60, 20);
  }

  // �e�N�X�`�����������m�ۂ��ēǂݍ��񂾉摜��]������
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0,
    GL_BGR, GL_UNSIGNED_BYTE, dst.data);

  // �~�b�v�}�b�v���쐬���ėL���ɂ���
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  // ���E�F�͍��ɂ��Ă��� (����� Oculus Rift �ւ̕\�����ɕ\���͈͊O�̐F�ɂȂ�)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

#if USE_ANISOTROPIC_FILTERING
  // ��Ώۃt�B���^�����O�g���@�\��L���ɂ���
  GLfloat fLargest;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
#endif

  // �E�B���h�E���J���Ă���Ԃ���Ԃ��`�悷��
  while (!window.shouldClose())
  {
    // ��ʃN���A
    window.clear();

    // �`��p�̃V�F�[�_�v���O�����̎g�p�J�n
    simple.use();
    simple.setLight(light);
    simple.setMaterial(material);
    glUniform1i(cmapLoc, 0);

    // �e�N�X�`���̎w��
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    // �}�`�f�[�^�̎w��
    glBindVertexArray(mesh);

#if STEREO == NONE
    // �P��̃��f���r���[�v���W�F�N�V�����ϊ��s���ݒ肷��
    simple.loadMatrix(window.getMp(), window.getMw() * mm);

    // �`��
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
#else
    // ���ڂ̃��f���r���[�v���W�F�N�V�����ϊ��s���ݒ肷��
    simple.loadMatrix(window.getMpL(), window.getMwL() * mm);

    // �`��
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);

    // �E�ڂ̃��f���r���[�v���W�F�N�V�����ϊ��s���ݒ肷��
    simple.loadMatrix(window.getMpR(), window.getMwR() * mm);

    // �`��
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
#endif

    // �o�b�t�@�����ւ���
    window.swapBuffers();
  }
}
