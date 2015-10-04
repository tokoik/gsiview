#pragma once

//
// �E�B���h�E�֘A�̏���
//

// �e��ݒ�
#include "config.h"

// Oculus Rift SDK ���C�u���� (LibOVR) �̑g�ݍ���
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
// �E�B���h�E�֘A�̏�����S������N���X
//
class Window
{
  // �E�B���h�E�̎��ʎq
  GLFWwindow *const window;

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

#if STEREO != OCULUS
  // �E�B���h�E�̕��ƍ���
  int winW, winH;

  // �X�N���[���̕��ƍ���
  GLfloat scrW, scrH;

#  if STEREO == NONE
  // ���e�ϊ��s��
  GgMatrix mp;

  //
  // �������e�ϊ��s������߂�
  //
  //   �E�E�B���h�E�̃T�C�Y�ύX����J�����p�����[�^�̕ύX���ɌĂяo��
  //
  void updateProjectionMatrix()
  {
    mp.loadFrustum(-scrW, scrW, -scrH, scrH, zNear, zFar);
  }
#  else
  // ����
  GLfloat parallax;

  // ���̎��p�̓��e�ϊ��s��
  GgMatrix mpL, mpR;

  //
  // ���̎��p�̓������e�ϊ��s������߂�
  //
  //   �E�E�B���h�E�̃T�C�Y�ύX����J�����p�����[�^�̕ύX���ɌĂяo��
  //
  void updateStereoProjectionMatrix()
  {
    // �����ɂ��X�N���[���̃I�t�Z�b�g��
    const GLfloat shift(parallax * zNear / screenDistance);

    // ���̎��p�̓������e�ϊ��s��
    mpL.loadFrustum(-scrW + shift, scrW + shift, -scrH, scrH, zNear, zFar);
    mpR.loadFrustum(-scrW - shift, scrW - shift, -scrH, scrH, zNear, zFar);
  }
#  endif
#else
  // Oculus Rift �\���p�� FBO
  GLuint ocuFbo;

  // Oculus Rift �\���p�� FBO �̃J���[�o�b�t�@�Ɏg���e�N�X�`��
  GLuint ocuFboColor;

  // Oculus Rift �\���p�� FBO �̃f�v�X�o�b�t�@�Ɏg�������_�[�o�b�t�@
  GLuint ocuFboDepth;

  // Oculus Rift �\���p�� FBO �̃����_�[�^�[�Q�b�g
  static const GLenum ocuFboDrawBuffers[];

  // Oculus Rift �\���p�� FBO �̃T�C�Y
  ovrSizei renderTargetSize;

  // Oculus Rift �̃r���[�|�[�g
  ovrRecti eyeRenderViewport[2];

  // Oculus Rift �̃����_�����O���
  ovrEyeRenderDesc eyeRenderDesc[2];

  // Oculus Rift �̎��_���
  ovrPosef eyePose[2];

  // Oculus rift �\���p�̃����_�����O�^�[�Q�b�g�̃e�N�X�`��
  ovrGLTexture eyeTexture[2];

  // Oculus Rift �ւ̃����_�����O�̃^�C�~���O�v��
  ovrFrameTiming frameTiming;

  // Oculus Rift �̃f�o�C�X
  const ovrHmd hmd;
#endif

  // �Q�ƃJ�E���g
  static unsigned int count;

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
  Window(int width = 640, int height = 480, const char *title = "GLFW Window",
    GLFWmonitor *monitor = nullptr, GLFWwindow *share = nullptr);

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

#if STEREO == NONE
  //
  // ���f���r���[�ϊ��s��𓾂�
  //
  const GgMatrix &getMw() const
  {
    return mv;
  }

  //
  // �v���W�F�N�V�����ϊ��s��𓾂�
  //
  const GgMatrix &getMp() const
  {
    return mp;
  }
#else
  //
  // ���ڗp�̃��f���r���[�ϊ��s��𓾂�
  //
  //   �E���ڂ̕`����L�̏������s��
  //
  GgMatrix getMwL();

  //
  // ���ڗp�̃v���W�F�N�V�����ϊ��s��𓾂�
  //
#  if STEREO != OCULUS
  const GgMatrix &getMpL() const
  {
    return mpL;
  }
#  else
  const GgMatrix getMpL() const
  {
    // Oculus Rift �̍��ڂ̎��ʎq
    const ovrFovPort &fov(eyeRenderDesc[hmd->EyeRenderOrder[0]].Fov);

    // ���ڂ̓������e�ϊ��s��
    const GLfloat left(-fov.LeftTan * zNear);
    const GLfloat right(fov.RightTan * zNear);
    const GLfloat bottom(-fov.DownTan * zNear);
    const GLfloat top(fov.UpTan * zNear);
    return ggFrustum(left, right, bottom, top, zNear, zFar);
  }
#  endif

  //
  // �E�ڗp�̃��f���r���[�ϊ��s��𓾂�
  //
  //   �E�E�ڂ̕`����L�̏������s��
  //
  GgMatrix getMwR();

  //
  // �E�ڗp�̃v���W�F�N�V�����ϊ��s��𓾂�
  //
#  if STEREO != OCULUS
  const GgMatrix &getMpR() const
  {
    return mpR;
  }
#  else
  const GgMatrix getMpR() const
  {
    // Oculus Rift �̍��ڂ̎��ʎq
    const ovrFovPort &fov(eyeRenderDesc[hmd->EyeRenderOrder[1]].Fov);

    // ���ڂ̓������e�ϊ��s��
    const GLfloat left(-fov.LeftTan * zNear);
    const GLfloat right(fov.RightTan * zNear);
    const GLfloat bottom(-fov.DownTan * zNear);
    const GLfloat top(fov.UpTan * zNear);
    return ggFrustum(left, right, bottom, top, zNear, zFar);
  }
#  endif
#endif
};
