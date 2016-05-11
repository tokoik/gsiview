//
// �E�B���h�E�֘A�̏���
//
#include <iostream>
#include "Window.h"

// Oculus Rift SDK ���C�u���� (LibOVR) �̑g�ݍ���
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
// Oculus Rift �̖ڂ̎��ʎq
const int eyeL(ovrEye_Left), eyeR(ovrEye_Right);
#else
// �ڂ̎��ʎq
const int eyeL(0), eyeR(1);
#endif

// Mac �� Linux �ł̓W���C�X�e�B�b�N�̉E���̃X�e�B�b�N�̔ԍ���������
#if defined(_WIN32)
const int axesOffset(0);
#else
const int axesOffset(1);
#endif

//
// �R���X�g���N�^
//
Window::Window(int width, int height, const char *title, GLFWmonitor *monitor, GLFWwindow *share)
  : window(glfwCreateWindow(width, height, title, monitor, share))
  , screenHeight(zNear * displayCenter / displayDistance)
#if STEREO == OCULUS
  , session(nullptr)
#endif
{
  // �E�B���h�E���J����Ă��Ȃ�������߂�
  if (!window) return;

  // �ݒ������������
  reset();

  // ���݂̃E�B���h�E�������Ώۂɂ���
  glfwMakeContextCurrent(window);

  // �Q�[���O���t�B�b�N�X���_�̓s���ɂ��ƂÂ����������s��
  ggInit();

#if STEREO == OCULUS
  //
  // Oculus Rift �̐ݒ�
  //

  // LUID �� OpenGL �ł͎g���Ă��Ȃ��炵��
  ovrGraphicsLuid luid;

  // Oculus Rift �̃Z�b�V�������쐬����
  if (OVR_FAILURE(ovr_Create(&const_cast<ovrSession>(session), &luid)))
  {
    // �\���p�̃E�B���h�E��j������
    glfwDestroyWindow(window);
    const_cast<GLFWwindow *>(window) = nullptr;
    return;
  }

  // Oculus Rift �̏������o��
  hmdDesc = ovr_GetHmdDesc(session);

#  if defined(_DEBUG)
  // Oculus Rift �̏���\������
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

  // Oculus Rift �\���p�� FBO ���쐬����
  for (int eye = 0; eye < ovrEye_Count; ++eye)
  {
    // Oculus Rift �\���p�� FBO �̃T�C�Y
    const auto renderTargetSize(ovr_GetFovTextureSize(session, ovrEyeType(eye), hmdDesc.DefaultEyeFov[eye], 1.0f));

    // Oculus Rift �̃����Y�␳���̐ݒ�l
    eyeRenderDesc[eye] = ovr_GetRenderDesc(session, ovrEyeType(eye), hmdDesc.DefaultEyeFov[eye]);

    // Oculus Rift �\���p�� FBO �̃J���[�o�b�t�@�Ƃ��Ďg���e�N�X�`���Z�b�g�̍쐬
    ovrSwapTextureSet *colorTexture;
    ovr_CreateSwapTextureSetGL(session, GL_SRGB8_ALPHA8, renderTargetSize.w, renderTargetSize.h, &colorTexture);

    // Oculus Rift �\���p�� FBO �̃f�v�X�o�b�t�@�Ƃ��Ďg���e�N�X�`���Z�b�g�̍쐬
    ovrSwapTextureSet *depthTexture;
    ovr_CreateSwapTextureSetGL(session, GL_DEPTH_COMPONENT32F, renderTargetSize.w, renderTargetSize.h, &depthTexture);

    // Oculus Rift �ɓ]������`��f�[�^���쐬����
    layerData.Header.Type = ovrLayerType_EyeFovDepth;
    layerData.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // OpenGL �Ȃ̂ō��������_
    layerData.EyeFov.ColorTexture[eye] = colorTexture;
    layerData.EyeFovDepth.DepthTexture[eye] = depthTexture;
    layerData.EyeFov.Viewport[eye].Pos = OVR::Vector2i(0, 0);
    layerData.EyeFov.Viewport[eye].Size = renderTargetSize;
    layerData.EyeFov.Fov[eye] = hmdDesc.DefaultEyeFov[eye];
  }

  // �~���[�\���p�� FBO ���쐬����
  if (OVR_SUCCESS(ovr_CreateMirrorTextureGL(session, GL_SRGB8_ALPHA8, width, height, reinterpret_cast<ovrTexture **>(&mirrorTexture))))
  {
    glGenFramebuffers(1, &mirrorFbo);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTexture->OGL.TexId, 0);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  }

  // Oculus Rift �̃����_�����O�p�� FBO ���쐬����
  glGenFramebuffers(ovrEye_Count, oculusFbo);

  // Oculus Rift �Ƀ����_�����O����Ƃ��� sRGB �J���[�X�y�[�X���g��
  glEnable(GL_FRAMEBUFFER_SRGB);
#endif

  //
  // �E�B���h�E�̐ݒ�
  //

  // �E�B���h�E�̃T�C�Y�ύX���ɌĂяo��������o�^����
  glfwSetFramebufferSizeCallback(window, resize);

  // �}�E�X�{�^���𑀍삵���Ƃ��̏�����o�^����
  glfwSetMouseButtonCallback(window, mouse);

  // �}�E�X�z�C�[�����쎞�ɌĂяo��������o�^����
  glfwSetScrollCallback(window, wheel);

  // �L�[�{�[�h�𑀍삵�����̏�����o�^����
  glfwSetKeyCallback(window, keyboard);

  // �}�E�X�J�[�\����\������
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  // ���̃C���X�^���X�� this �|�C���^���L�^���Ă���
  glfwSetWindowUserPointer(window, this);

  //
  // �W���C�X�e�B�b�N�̐ݒ�
  //

  // �W���C�X�e�b�N�̗L���𒲂ׂĔԍ������߂�
  joy = glfwJoystickPresent(count) ? count : -1;

  // �X�e�B�b�N�̒����ʒu�����߂�
  if (joy >= 0)
  {
    int axesCount;
    const auto *const axes(glfwGetJoystickAxes(joy, &axesCount));

    if (axesCount > 3 + axesOffset)
    {
      // �N������̃X�e�B�b�N�̈ʒu����ɂ���
      origin[0] = axes[0];
      origin[1] = axes[1];
      origin[2] = axes[2 + axesOffset];
      origin[3] = axes[3 + axesOffset];
    }
  }

  // ���e�ϊ��s��E�r���[�|�[�g������������
  resize(window, width, height);

  // �Q�ƃJ�E���g�𑝂�
  ++count;
}

//
// �f�X�g���N�^
//
Window::~Window()
{
  // �E�B���h�E���J����Ă��Ȃ�������߂�
  if (!window) return;

  // �Q�ƃJ�E���g��������
  --count;

#if STEREO == OCULUS
  // Oculus Rift �g�p��
  if (session)
  {
    // �~���[�\���p�� FBO ���폜����
    glDeleteFramebuffers(1, &mirrorFbo);

    // �~���[�\���Ɏg�����e�N�X�`�����J������
    glDeleteTextures(1, &mirrorTexture->OGL.TexId);
    ovr_DestroyMirrorTexture(session, reinterpret_cast<ovrTexture *>(mirrorTexture));

    // Oculus Rift �\���p�� FBO ���폜����
    for (int eye = 0; eye < ovrEye_Count; ++eye)
    {
      // Oculus Rift �̃����_�����O�p�� FBO ���폜����
      glDeleteFramebuffers(1, &oculusFbo[eye]);

      // �����_�����O�^�[�Q�b�g�Ɏg�����e�N�X�`�����J������
      auto *const colorTexture(layerData.EyeFov.ColorTexture[eye]);
      for (int i = 0; i < colorTexture->TextureCount; ++i)
      {
        const auto *const ctex(reinterpret_cast<ovrGLTexture *>(&colorTexture->Textures[i]));
        glDeleteTextures(1, &ctex->OGL.TexId);
      }
      ovr_DestroySwapTextureSet(session, colorTexture);

      // �f�v�X�o�b�t�@�Ƃ��Ďg�����e�N�X�`�����J������
      auto *const depthTexture(layerData.EyeFovDepth.DepthTexture[eye]);
      for (int i = 0; i < depthTexture->TextureCount; ++i)
      {
        const auto *const dtex(reinterpret_cast<ovrGLTexture *>(&depthTexture->Textures[i]));
        glDeleteTextures(1, &dtex->OGL.TexId);
      }
      ovr_DestroySwapTextureSet(session, depthTexture);
    }

    // Oculus Rift �̃Z�b�V������j������
    ovr_Destroy(session);
  }
#endif

  // �\���p�̃E�B���h�E�����
  glfwDestroyWindow(window);
}

//
// ��ʃN���A
//
//   �E�}�`�̕`��J�n�O�ɌĂяo��
//   �E��ʂ̏����Ȃǂ��s��
//
void Window::clear()
{
  // ���f���r���[�ϊ��s���ݒ肷��
  mv = ggRotateX(pitch).rotateZ(heading + direction).translate(-ex, -ey, -ez);

#if STEREO == OCULUS
  // Oculus Rift �g�p��
  if (session)
  {
    // �t���[���̃^�C�~���O�v���J�n
    const auto ftiming(ovr_GetPredictedDisplayTime(session, 0));

    // sensorSampleTime �̎擾�͉\�Ȍ��� ovr_GetTrackingState() �̋߂��ōs��
    layerData.EyeFov.SensorSampleTime = ovr_GetTimeInSeconds();

    // �w�b�h�g���b�L���O�̏�Ԃ��擾����
    const auto hmdState(ovr_GetTrackingState(session, ftiming, ovrTrue));

    // ���������E���o�����ƂɎ��_�̎p�����擾����
    const ovrVector3f viewOffset[2] = { eyeRenderDesc[0].HmdToEyeViewOffset, eyeRenderDesc[1].HmdToEyeViewOffset };
    ovr_CalcEyePoses(hmdState.HeadPose.ThePose, viewOffset, eyePose);
  }
  else
#endif
  {
    // �J���[�o�b�t�@�ƃf�v�X�o�b�t�@����������
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }
}

//
// �J���[�o�b�t�@�����ւ��ăC�x���g�����o��
//
//   �E�}�`�̕`��I����ɌĂяo��
//   �E�_�u���o�b�t�@�����O�̃o�b�t�@�̓���ւ����s��
//   �E�L�[�{�[�h���쓙�̃C�x���g�����o��
//
void Window::swapBuffers()
{
  // �G���[�`�F�b�N
  ggError("SwapBuffers");

#if STEREO == OCULUS
  // Oculus Rift �g�p��
  if (session)
  {
    // Oculus Rift ��̕`��ʒu�Ɗg�嗦�����߂�
    ovrViewScaleDesc viewScaleDesc;
    viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
    viewScaleDesc.HmdToEyeViewOffset[0] = eyeRenderDesc[0].HmdToEyeViewOffset;
    viewScaleDesc.HmdToEyeViewOffset[1] = eyeRenderDesc[1].HmdToEyeViewOffset;

    // �`��f�[�^���X�V����
    layerData.EyeFov.RenderPose[0] = eyePose[0];
    layerData.EyeFov.RenderPose[1] = eyePose[1];

    // �`��f�[�^�� Oculus Rift �ɓ]������
    const auto *const layers(&layerData.Header);
    const ovrResult result(ovr_SubmitFrame(session, 0, &viewScaleDesc, &layers, 1));
    // �]���Ɏ��s������ Oculus Rift �̐ݒ���ŏ������蒼���K�v������炵��
    // ���ǂ߂�ǂ������̂ł��Ȃ�

    // �����_�����O���ʂ��~���[�\���p�̃t���[���o�b�t�@�ɂ��]������
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    const auto w(mirrorTexture->OGL.Header.TextureSize.w);
    const auto h(mirrorTexture->OGL.Header.TextureSize.h);
    glBlitFramebuffer(0, h, w, 0, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    // �c���Ă��� OpenGL �R�}���h�����s����
    glFlush();
  }
  else
#endif
  {
    // �J���[�o�b�t�@�����ւ���
    glfwSwapBuffers(window);
  }

  // �C�x���g�����o��
  glfwPollEvents();

  //
  // �}�E�X�ɂ�鑀��
  //

  // �}�E�X�̈ʒu�𒲂ׂ�
  double x, y;
  glfwGetCursorPos(window, &x, &y);

  // ���x�����x�ɔ�Ⴓ����
  const float speedFactor((fabs(ez) + 0.2f));

  // ���{�^���h���b�O
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1))
  {
    // �J�����̈ʒu���ړ�����
    const auto speed(static_cast<GLfloat>(cy - y) * speedScale * speedFactor);
    ex += speed * sin(direction);
    ey += speed * cos(direction);

    // �J�����̐i�s������ς���
    direction += angleScale * static_cast<GLfloat>(x - cx);
  }

  // �E�{�^���h���b�O
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2))
  {
    // �}�E�X�{�^�����������ʒu����̕ψ�
    const auto dx(static_cast<GLfloat>(x - cx));
    const auto dy(static_cast<GLfloat>(y - cy));

    // �ړ��ʂ̑傫���������ύX������������₷���C������
    if (fabs(dx) > fabs(dy))
    {
      // �J�����̕��ʊp��ς���
      heading += angleScale * dx;
    }
    else
    {
      // �J�����̋p��ς���
      pitch += angleScale * dy;
    }
  }


  //
  // �W���C�X�e�B�b�N�ɂ�鑀��
  //

  // �W���C�X�e�B�b�N���L���Ȃ�
  if (joy >= 0)
  {
    // �X�e�B�b�N
    int axesCount;
    const auto *const axes(glfwGetJoystickAxes(joy, &axesCount));

    if (axesCount > 3 + axesOffset)
    {
      // �X�e�B�b�N�̑��x�W��
      const auto axesSpeedFactor(axesSpeedScale * speedFactor);

      // �J������O��Ɉړ�����
      const auto advSpeed((axes[1] - origin[1]) * axesSpeedFactor);
      ex -= advSpeed * sin(direction);
      ey -= advSpeed * cos(direction);

      // �J���������E�Ɉړ�����
      const auto latSpeed((axes[2 + axesOffset] - origin[2]) * axesSpeedFactor);
      ey -= latSpeed * sin(direction);
      ex += latSpeed * cos(direction);

      // �J�������㉺�Ɉړ�����
      ez -= (axes[3 + axesOffset] - origin[3]) * axesSpeedFactor;

      // �J�����̐i�s�������X�V����
      direction += (axes[0] - origin[0]) * axesAngleScale;
    }

    // �{�^��
    int btnsCount;
    const auto *const btns(glfwGetJoystickButtons(joy, &btnsCount));
    if (btnsCount > 3)
    {
      // �J�����̋p�𒲐�����
      pitch += btnsScale * static_cast<GLfloat>(btns[2] - btns[1]);

      // �J�����̕��ʊp�𒲐�����
      heading += btnsScale * static_cast<GLfloat>(btns[3] - btns[0]);
    }

    // �J�����̕��ʊp�𐳖ʂɖ߂�
    if (btnsCount > 4 && btns[4] > 0) heading = 0.0f;
  }

  // �E���L�[����
  if (glfwGetKey(window, GLFW_KEY_RIGHT))
  {
    // �������g�傷��
    parallax += parallaxStep;
    updateProjectionMatrix();
  }

  // �����L�[����
  if (glfwGetKey(window, GLFW_KEY_LEFT))
  {
    // �������k������
    parallax -= parallaxStep;
    updateProjectionMatrix();
  }
}

//
// �E�B���h�E�̃T�C�Y�ύX���̏���
//
//   �E�E�B���h�E�̃T�C�Y�ύX���ɃR�[���o�b�N�֐��Ƃ��ČĂяo�����
//   �E�E�B���h�E�̍쐬���ɂ͖����I�ɌĂяo��
//
void Window::resize(GLFWwindow *window, int width, int height)
{
  // ���̃C���X�^���X�� this �|�C���^�𓾂�
  Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

  if (instance)
  {
#if STEREO == OCULUS
    // Oculus Rift �g�p���ȊO
    if (!instance->session)
#endif
    {
      // �f�B�X�v���C�̂̃A�X�y�N�g������߂�
      instance->aspect = static_cast<GLfloat>(width) / static_cast<GLfloat>(height);

      switch (STEREO)
      {
      case SIDEBYSIDE:
        // �E�B���h�E�̉��������r���[�|�[�g�ɂ���
        width /= 2;
        break;
      case TOPANDBOTTOM:
        // �E�B���h�E�̏c�������r���[�|�[�g�ɂ���
        height /= 2;
        break;
      default:
        // �E�B���h�E�S�̂��r���[�|�[�g�ɂ��Ă���
        glViewport(0, 0, width, height);
        break;
      }

      // �r���[�|�[�g�̑傫����ۑ����Ă���
      instance->width = width;
      instance->height = height;
    }

    // �������e�ϊ��s������߂�
    instance->updateProjectionMatrix();
  }
}

//
// �}�E�X�{�^���𑀍삵���Ƃ��̏���
//
//   �E�}�E�X�{�^�����������Ƃ��ɃR�[���o�b�N�֐��Ƃ��ČĂяo�����
//
void Window::mouse(GLFWwindow *window, int button, int action, int mods)
{
  // ���̃C���X�^���X�� this �|�C���^�𓾂�
  Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

  if (instance)
  {
    // �}�E�X�̌��݈ʒu�����o��
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    switch (button)
    {
    case GLFW_MOUSE_BUTTON_1:
      // ���{�^�������������̏���
    case GLFW_MOUSE_BUTTON_2:
      // �E�{�^�������������̏���
      if (action)
      {
        // �h���b�O�J�n�ʒu��ۑ�����
        instance->cx = x;
        instance->cy = y;
      }
      break;
    case GLFW_MOUSE_BUTTON_3:
      // ���{�^�������������̏���
      break;
    default:
      break;
    }
  }
}

//
// �}�E�X�z�C�[�����쎞�̏���
//
//   �E�}�E�X�z�C�[���𑀍삵�����ɃR�[���o�b�N�֐��Ƃ��ČĂяo�����
//
void Window::wheel(GLFWwindow *window, double x, double y)
{
  // ���̃C���X�^���X�� this �|�C���^�𓾂�
  Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

  if (instance)
  {
    // �z�C�[�������ɓ������Ă�����
    if (fabs(x) > fabs(y))
    {
      // �J���������E�Ɉړ�����
      const GLfloat latSpeed((fabs(instance->ez) * 5.0f + 1.0f) * wheelXStep * static_cast<GLfloat>(x));
      instance->ey -= latSpeed * sin(instance->direction);
      instance->ex += latSpeed * cos(instance->direction);
    }
    else
    {
      // �V�t�g�L�[�������Ă�����
      if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT))
      {
        // �J�����̃Y�[�����𒲐�����
        instance->zoom += zoomStep * y;

        // �������e�ϊ��s����X�V����
        instance->updateProjectionMatrix();
      }
      else
      {
        // �J�������㉺�Ɉړ�����
        const GLfloat advSpeed((fabs(instance->ez) * 5.0f + 1.0f) * wheelYStep * static_cast<GLfloat>(y));
        instance->ez += advSpeed;
      }
    }
  }
}

//
// �L�[�{�[�h���^�C�v�������̏���
//
//   �D�L�[�{�[�h���^�C�v�������ɃR�[���o�b�N�֐��Ƃ��ČĂяo�����
//
void Window::keyboard(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  // ���̃C���X�^���X�� this �|�C���^�𓾂�
  Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

  if (instance)
  {
    if (action == GLFW_PRESS)
    {
      // �Ō�Ƀ^�C�v�����L�[���o���Ă���
      instance->key = key;

      // �L�[�{�[�h����ɂ�鏈��
      switch (key)
      {
      case GLFW_KEY_R:
        // �ݒ�����Z�b�g����
        instance->reset();
        instance->updateProjectionMatrix();
        break;
      case GLFW_KEY_P:
        // �J�����̈ʒu�����Z�b�g����
        instance->ex = startPosition[0];
        instance->ey = startPosition[1];
        instance->ez = startPosition[2];
        instance->direction = 0.0f;
        instance->updateProjectionMatrix();
        break;
      case GLFW_KEY_O:
        // �J�����̌��������Z�b�g����
        instance->pitch = startPitch;
      case GLFW_KEY_H:
        // �J�����̕��ʊp���������Z�b�g����
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
// �ݒ�l�̏�����
//
void Window::reset()
{
  // ���̂̈ʒu
  ex = startPosition[0];
  ey = startPosition[1];
  ez = startPosition[2];

  // �J�����̐i�s����
  direction = 0.0f;

  //�@�J�����̌���
  heading = 0.0f;
  pitch = startPitch;

  // ���̂ɑ΂���Y�[����
  zoom = initialZoom;

  // ����
  parallax = STEREO != NONE ? initialParallax : 0.0f;
}

//
// �������e�ϊ��s������߂�
//
//   �E�E�B���h�E�̃T�C�Y�ύX����J�����p�����[�^�̕ύX���ɌĂяo��
//
void Window::updateProjectionMatrix()
{
  // �Y�[����
  const auto zf(static_cast<GLfloat>(zoom) *zNear);

  // �\���̈�
  GLfloat leftL, rightL, bottomL, topL;
  GLfloat leftR, rightR, bottomR, topR;

#if STEREO == OCULUS
  // Oculus Rift �g�p��
  if (session)
  {
    // Oculus Rift �̍��̎���
    const auto fovL(eyeRenderDesc[eyeL].Fov);

    // �Жڂ̃E�C���h�E (�\���̈�) �𒲂ׂ�
    leftL = -fovL.LeftTan * zf;
    rightL = fovL.RightTan * zf;
    bottomL = -fovL.DownTan * zf;
    topL = fovL.UpTan * zf;

    // Oculus Rift �̉E�̎���
    const auto fovR(eyeRenderDesc[eyeR].Fov);

    // �Жڂ̃E�C���h�E (�\���̈�) �𒲂ׂ�
    leftR = -fovR.LeftTan * zf;
    rightR = fovR.RightTan * zf;
    bottomR = -fovR.DownTan * zf;
    topR = fovR.UpTan * zf;
  }
  else
#endif
  {
    // �X�N���[���̕�
    const auto screenWidth(screenHeight * aspect);

    // �����ɂ��X�N���[���̃I�t�Z�b�g��
    const auto offset(parallax * zNear / displayDistance);

    // ���̎���
    leftL = (-screenWidth + offset) * zf;
    rightL = (screenWidth + offset) * zf;
    bottomL = -screenHeight * zf;
    topL = screenHeight * zf;

    // Oculus Rift �ȊO�̗��̎��\���̏ꍇ
    if (STEREO != NONE)
    {
      // �E�̎���
      leftR = (-screenWidth - offset) * zf;
      rightR = (screenWidth - offset) * zf;
      bottomR = -screenHeight * zf;
      topR = screenHeight * zf;
    }
  }

  // ���̃X�N���[���̃T�C�Y�ƈʒu
  screen[eyeL][0] = (rightL - leftL) * 0.5f;
  screen[eyeL][1] = (topL - bottomL) * 0.5f;
  screen[eyeL][2] = (rightL + leftL) * 0.25f;
  screen[eyeL][3] = (topL + bottomL) * 0.25f;

  // ���̓������e�ϊ��s������߂�
  mp[eyeL].loadFrustum(leftL, rightL, bottomL, topL, zNear, zFar);

#if STEREO == OCULUS
  // Oculus Rift �g�p��
  if (session)
  {
    // TimeWarp �Ɏg���ϊ��s��̐���
    auto &posTimewarpProjectionDesc(layerData.EyeFovDepth.ProjectionDesc);
    posTimewarpProjectionDesc.Projection22 = (mp[eyeL].get()[4 * 2 + 2] + mp[eyeL].get()[4 * 3 + 2]) * 0.5f;
    posTimewarpProjectionDesc.Projection23 = mp[eyeL].get()[4 * 2 + 3] * 0.5f;
    posTimewarpProjectionDesc.Projection32 = mp[eyeL].get()[4 * 3 + 2];
  }
#endif

  // ���̎��\�����s���Ȃ�
  if (STEREO != NONE)
  {
    // �E�̃X�N���[���̃T�C�Y�ƈʒu
    screen[eyeR][0] = (rightR - leftR) * 0.5f;
    screen[eyeR][1] = (topR - bottomR) * 0.5f;
    screen[eyeR][2] = (rightR + leftR) * 0.25f;
    screen[eyeR][3] = (topR + bottomR) * 0.25f;

    // �E�̓������e�ϊ��s������߂�
    mp[eyeR].loadFrustum(leftR, rightR, bottomR, topR, zNear, zFar);
  }
}

//
// �`��ݒ�
//
//   �E���̐}�`�̕`��J�n�O�ɌĂяo��
//   �E�r���[�|�[�g�̐ݒ�Ȃǂ��s��
//
void Window::select(int eye)
{
#if STEREO == OCULUS
  // Oculus Rift �g�p��
  if (session)
  {
    // �����_�[�^�[�Q�b�g�ɕ`�悷��O�Ƀ����_�[�^�[�Q�b�g�̃C���f�b�N�X���C���N�������g����
    auto *const colorTexture(layerData.EyeFov.ColorTexture[eye]);
    colorTexture->CurrentIndex = (colorTexture->CurrentIndex + 1) % colorTexture->TextureCount;
    auto *const depthTexture(layerData.EyeFovDepth.DepthTexture[eye]);
    depthTexture->CurrentIndex = (depthTexture->CurrentIndex + 1) % depthTexture->TextureCount;

    // �����_�[�^�[�Q�b�g��؂�ւ���
    glBindFramebuffer(GL_FRAMEBUFFER, oculusFbo[eye]);
    const auto &ctex(reinterpret_cast<ovrGLTexture *>(&colorTexture->Textures[colorTexture->CurrentIndex]));
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ctex->OGL.TexId, 0);
    const auto &dtex(reinterpret_cast<ovrGLTexture *>(&depthTexture->Textures[depthTexture->CurrentIndex]));
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dtex->OGL.TexId, 0);

    // �r���[�|�[�g��ݒ肵�ĉ�ʃN���A
    const auto &vp(layerData.EyeFov.Viewport[eye]);
    glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Oculus Rift �̍��̈ʒu�ƌ������擾����
    const auto &o(eyePose[eye].Orientation);
    const auto &p(eyePose[eye].Position);
    const auto &q(eyeRenderDesc[eye].HmdToEyeViewOffset);

    // Oculus Rift �̕Жڂ̉�]�����߂�
    mo[eye] = ggQuaternionMatrix(GgQuaternion(o.x, o.y, o.z, o.w));

    // Oculus Rift �̕Жڂ̌��������f���r���[�ϊ��s��ɔ��f����
    mw[eye] = mo[eye].transpose() * ggTranslate(-q.x - p.x, -q.y - p.y, -q.z - p.z) * mv;

    return;
  }
#endif

  // Oculus Rift �ȊO�ɕ\������Ƃ�
  switch (STEREO)
  {
  case TOPANDBOTTOM:
    if (eye == eyeL)
    {
      // �f�B�X�v���C�̏㔼�������ɕ`�悷��
      glViewport(0, height, width, height);
    }
    else
    {
      // �f�B�X�v���C�̉����������ɕ`�悷��
      glViewport(0, 0, width, height);
    }
    break;
  case SIDEBYSIDE:
    if (eye == eyeL)
    {
      // �f�B�X�v���C�̍����������ɕ`�悷��
      glViewport(0, 0, width, height);
    }
    else
    {
      // �f�B�X�v���C�̉E���������ɕ`�悷��
      glViewport(width, 0, width, height);
    }
    break;
  case QUADBUFFER:
    // ���o�b�t�@�ɕ`�悷��
    glDrawBuffer(eye == eyeL ? GL_BACK_LEFT :  GL_BACK_RIGHT);
    break;
  default:
    break;
  }

  // �w�b�h�g���b�L���O�ɂ�鎋���̉�]���s��Ȃ�
  mo[eye] = ggIdentity();

  // �ڂ����炷����ɃV�[���𓮂���
  mw[eye] = ggTranslate(eye == eyeL ? parallax : -parallax, 0.0f, 0.0f) * mv;
}


// �Q�ƃJ�E���g
unsigned int Window::count(0);
