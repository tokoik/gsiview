#pragma once

//
// �E�B���h�E�֘A�̏���
//

// �e��ݒ�
#include "config.h"

// Windows API �֘A�̐ݒ�
#if defined(_WIN32)
#  define NOMINMAX
#  define GLFW_EXPOSE_NATIVE_WIN32
#  define GLFW_EXPOSE_NATIVE_WGL
#  define OVR_OS_WIN32
#  include "glfw3native.h"
#  if defined(APIENTRY)
#    undef APIENTRY
#  endif
#endif

// Oculus Rift SDK ���C�u���� (LibOVR) �̑g�ݍ���
#include <OVR.h>
#include <OVR_CAPI_GL.h>

//
// �E�B���h�E�֘A�̏�����S������N���X
//
class Window
{
  // �E�B���h�E�̎��ʎq
  GLFWwindow *const window;

  // �X�N���[���̍���
  const GLfloat screenHeight;

  // �Ō�Ƀ^�C�v�����L�[
  int key;

  // �W���C�X�e�B�b�N�̔ԍ�
  int joy;

  // �X�e�B�b�N�̒����ʒu
  float origin[4];

  // �h���b�O�J�n�ʒu
  double cx, cy;

  // �J�����̈ʒu
  GLfloat ex, ey, ez;

  // �J�����̐i�s����
  GLfloat direction;

  // �J�����̌���
  GLfloat heading, pitch;

  // ���f���r���[�ϊ��s��
  GgMatrix mv;

  // �f�B�X�v���C�̃A�X�y�N�g��
  GLfloat aspect;

  // �Y�[����
  double zoom;

  // �r���[�|�[�g�̕��ƍ���
  int width, height;

  // ����
  GLfloat parallax;

  // �w�b�h�g���b�L���O�ɂ���]�s��
  GgMatrix moL, moR;

  // ���̎��p�̃��f���r���[�ϊ��s��
  GgMatrix mwL, mwR;

  // ���̎��p�̓��e�ϊ��s��
  GgMatrix mpL, mpR;

  // �X�N���[���̕��ƍ���
  GLfloat screenL[4], screenR[4];

  // Oculus Rift �̃Z�b�V����
  const ovrSession session;

  // Oculus Rift �̏��
  ovrHmdDesc hmdDesc;

  // Oculus Rift �̃����_�����O���
  ovrEyeRenderDesc eyeRenderDesc[ovrEye_Count];

  // Oculus Rift �̎��_���
  ovrPosef eyePose[ovrEye_Count];

  // Oculus Rift �ɓ]������`��f�[�^
  ovrLayer_Union layerData;

  // Oculus Rift �\���p�� FBO
  GLuint oculusFbo[ovrEye_Count];

  // �~���[�\���p�̃����_�����O�^�[�Q�b�g�̃e�N�X�`��
  ovrGLTexture *mirrorTexture;

  // �~���[�\���p�� FBO
  GLuint mirrorFbo;

  // �Q�ƃJ�E���g
  static unsigned int count;

  //
  // �������e�ϊ��s������߂�
  //
  //   �E�E�B���h�E�̃T�C�Y�ύX����J�����p�����[�^�̕ύX���ɌĂяo��
  //
  void updateProjectionMatrix();

  //
  // �R�s�[�R���X�g���N�^ (�R�s�[�֎~)
  //
  Window(const Window &w);

  //
  // ��� (����֎~)
  //
  Window &operator=(const Window &w);

public:

  //
  // �R���X�g���N�^
  //
  Window(int width = 640, int height = 480, const char *title = "GLFW Window"
    , GLFWmonitor *monitor = nullptr, GLFWwindow *share = nullptr
    , ovrSession session = nullptr
    );

  //
  // �f�X�g���N�^
  //
  virtual ~Window();

  //
  // �E�B���h�E�̎��ʎq�̎擾
  //
  const GLFWwindow *get() const
  {
    return window;
  }

  //
  // �E�B���h�E�����ׂ����𔻒肷��
  //
  //   �E�`�惋�[�v�̌p�������Ƃ��Ďg��
  //
  bool shouldClose() const
  {
    // �E�B���h�E����邩 ESC �L�[���^�C�v����Ă���ΐ^
    return glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE);
  }

  //
  // ��ʃN���A
  //
  //   �E�}�`�̕`��J�n�O�ɌĂяo��
  //   �E��ʂ̏����Ȃǂ��s��
  //
  void clear();

  //
  // �J���[�o�b�t�@�����ւ��ăC�x���g�����o��
  //
  //   �E�}�`�̕`��I����ɌĂяo��
  //   �E�_�u���o�b�t�@�����O�̃o�b�t�@�̓���ւ����s��
  //   �E�L�[�{�[�h���쓙�̃C�x���g�����o��
  //
  void swapBuffers();

  //
  // �E�B���h�E�̃T�C�Y�ύX���̏���
  //
  //   �E�E�B���h�E�̃T�C�Y�ύX���ɃR�[���o�b�N�֐��Ƃ��ČĂяo�����
  //   �E�E�B���h�E�̍쐬���ɂ͖����I�ɌĂяo��
  //
  static void resize(GLFWwindow *window, int width, int height);

  //
  // �}�E�X�{�^���𑀍삵���Ƃ��̏���
  //
  //   �E�}�E�X�{�^�����������Ƃ��ɃR�[���o�b�N�֐��Ƃ��ČĂяo�����
  //
  static void mouse(GLFWwindow *window, int button, int action, int mods);

  //
  // �}�E�X�z�C�[�����쎞�̏���
  //
  //   �E�}�E�X�z�C�[���𑀍삵�����ɃR�[���o�b�N�֐��Ƃ��ČĂяo�����
  //
  static void wheel(GLFWwindow *window, double x, double y);

  //
  // �L�[�{�[�h���^�C�v�������̏���
  //
  //   �D�L�[�{�[�h���^�C�v�������ɃR�[���o�b�N�֐��Ƃ��ČĂяo�����
  //
  static void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods);

  //
  // �ݒ�l�̏�����
  //
  void reset();

  //
  // ���ڗp�̕`��ݒ�
  //
  //   �E���ڂ̐}�`�̕`��J�n�O�ɌĂяo��
  //   �E�r���[�|�[�g�̐ݒ�Ȃǂ��s��
  //
  void selectL();

  //
  // Oculus Rift �̃w�b�h�n���b�L���O�ɂ�鍶�ڂ̉�]�s��𓾂�
  //
  const GgMatrix &getMoL() const
  {
    return moL;
  }

  //
  // ���ڗp�̃��f���r���[�ϊ��s��𓾂�
  //
  const GgMatrix &getMwL() const
  {
    return mwL;
  }

  //
  // ���ڗp�̃v���W�F�N�V�����ϊ��s��𓾂�
  //
  const GgMatrix &getMpL() const
  {
    return mpL;
  }

  //
  // ���ڗp�̃X�N���[���̕��ƍ��������o��
  //
  const GLfloat *getScreenL() const
  {
    return screenL;
  }

  //
  // �E�ڗp�̕`��ݒ�
  //
  //   �E�E�ڂ̐}�`�̕`��J�n�O�ɌĂяo��
  //   �E�r���[�|�[�g�̐ݒ�Ȃǂ��s��
  //
  void selectR();

  //
  // Oculus Rift �̃w�b�h�n���b�L���O�ɂ��E�ڂ̉�]�s��𓾂�
  //
  const GgMatrix &getMoR() const
  {
    return moR;
  }

  //
  // �E�ڗp�̃��f���r���[�ϊ��s��𓾂�
  //
  const GgMatrix &getMwR() const
  {
    return mwR;
  }

  //
  // �E�ڗp�̃v���W�F�N�V�����ϊ��s��𓾂�
  //
  const GgMatrix &getMpR() const
  {
    return mpR;
  }

  //
  // �E�ڗp�̃X�N���[���̕��ƍ��������o��
  //
  const GLfloat *getScreenR() const
  {
    return screenR;
  }
};
