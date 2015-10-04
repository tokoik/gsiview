//
// �E�B���h�E�֘A�̏���
//
#include <iostream>
#include <algorithm>
#include "Window.h"

// Oculus Rift SDK ���C�u���� (LibOVR) �̑g�ݍ���
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
  , key(0)                                // �Ō�Ƀ^�C�v�����L�[
  , ex(startPosition[0])                  // �J������ x ���W
  , ey(startPosition[1])                  // �J������ y ���W
  , ez(startPosition[2])                  // �J������ z ���W
  , direction(0.0f)                       // �J�����̐i�s����
  , heading(0.0f)                         // �J�����̕��ʊp
  , pitch(0.0f)                           // �J�����̋p
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

  // ���݂̃E�B���h�E�������Ώۂɂ���
  glfwMakeContextCurrent(window);

  // �쐬�����E�B���h�E�ɑ΂���ݒ�
  glfwSwapInterval(1);

  // �E�B���h�E�̃T�C�Y�ύX���ɌĂяo�������̓o�^
  glfwSetFramebufferSizeCallback(window, resize);

  // �}�E�X�{�^���𑀍삵���Ƃ��̏���
  glfwSetMouseButtonCallback(window, mouse);

  // �}�E�X�z�C�[�����쎞�ɌĂяo������
  glfwSetScrollCallback(window, wheel);

  // �L�[�{�[�h�𑀍삵�����̏���
  glfwSetKeyCallback(window, keyboard);

  // �}�E�X�J�[�\����\������
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  // ���̃C���X�^���X�� this �|�C���^���L�^���Ă���
  glfwSetWindowUserPointer(window, this);

  // �Q�[���O���t�B�b�N�X���_�̓s���ɂ��ƂÂ�������
  if (!glCreateProgram) ggInit();

  // �W���C�X�e�b�N�̗L���𒲂ׂĔԍ������߂�
  joy = glfwJoystickPresent(count) ? count : -1;

  // �X�e�B�b�N�̒����ʒu�����߂�
  if (joy >= 0)
  {
    int axesCount;
    const float *const axes(glfwGetJoystickAxes(joy, &axesCount));

    if (axesCount > 3 + axesOffset)
    {
      // �N������̃X�e�B�b�N�̈ʒu����ɂ���
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
    // Oculus Rift �̏���\������
    std::cout
      << "\nProduct name: " << hmd->ProductName
      << "\nResolution:   " << hmd->Resolution.w << " x " << hmd->Resolution.h
      << "\nScreen Size:  " << hmd->CameraFrustumHFovInRadians
      << " x " << hmd->CameraFrustumVFovInRadians
      << "\nDepth Range:   " << hmd->CameraFrustumNearZInMeters
      << " - " << hmd->CameraFrustumFarZInMeters
      << "\n" << std::endl;
#  endif

    // Oculus Rift �����_�����O�p�̍��E�̖ڂ̃r���[�|�[�g
    eyeRenderViewport[0].Pos = ovrVector2i{ 0, 0 };
    eyeRenderViewport[0].Size = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left, hmd->DefaultEyeFov[0], 1.0f);
    eyeRenderViewport[1].Pos = ovrVector2i{ eyeRenderViewport[0].Size.w, 0 };
    eyeRenderViewport[1].Size = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right, hmd->DefaultEyeFov[1], 1.0f);

    // Oculus Rift �����_�����O�p�� FBO �̃T�C�Y
    renderTargetSize.w = eyeRenderViewport[0].Size.w + eyeRenderViewport[1].Size.w;
    renderTargetSize.h = std::max(eyeRenderViewport[0].Size.h, eyeRenderViewport[1].Size.h);

    // Oculus Rift �����_�����O�p�� FBO �̃J���[�o�b�t�@�Ƃ��Ďg���J���[�e�N�X�`���̍쐬
    glGenTextures(1, &ocuFboColor);
    glBindTexture(GL_TEXTURE_2D, ocuFboColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, renderTargetSize.w, renderTargetSize.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

    // Oculus Rift �����_�����O�p�� FBO �̃f�v�X�o�b�t�@�Ƃ��Ďg�������_�[�o�b�t�@�̍쐬
    glGenRenderbuffers(1, &ocuFboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, ocuFboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, renderTargetSize.w, renderTargetSize.h);

    // Oculus Rift �̃����_�����O�p�� FBO ���쐬����
    glGenFramebuffers(1, &ocuFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, ocuFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ocuFboColor, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ocuFboDepth);

    // �����_�[�^�[�Q�b�g�̃e�N�X�`�����Q�Ƃ���ݒ�
    eyeTexture[0].OGL.Header.API = eyeTexture[1].OGL.Header.API = ovrRenderAPI_OpenGL;
    eyeTexture[0].OGL.Header.TextureSize = eyeTexture[1].OGL.Header.TextureSize = renderTargetSize;
    eyeTexture[0].OGL.Header.RenderViewport = eyeRenderViewport[0];
    eyeTexture[1].OGL.Header.RenderViewport = eyeRenderViewport[1];
    eyeTexture[0].OGL.TexId = eyeTexture[1].OGL.TexId = ocuFboColor;

    // Oculus Rift �� OpenGL �Ń����_�����O���邽�߂̐ݒ�
    ovrGLConfig cfg;
    cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
    cfg.OGL.Header.BackBufferSize = hmd->Resolution;
    cfg.OGL.Header.Multisample = backBufferMultisample;
#if defined(_WIN32)
    cfg.OGL.Window = glfwGetWin32Window(window);
    cfg.OGL.DC = GetDC(cfg.OGL.Window);

    // �g���f�X�N�g�b�v�łȂ���� Direct Rendering ����
    if (!(hmd->HmdCaps & ovrHmdCap_ExtendDesktop))
      ovrHmd_AttachToWindow(hmd, cfg.OGL.Window, nullptr, nullptr);
#elif defined(X11)
    cfg.OGL.Disp = glfwGetX11Display();
#endif

    // Oculus Rift ��ݒ肷��
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

    // �g���b�L���O�E�Z���T�t���[�W����������������
    ovrHmd_ConfigureTracking(hmd,
      ovrTrackingCap_Orientation |
      ovrTrackingCap_MagYawCorrection |
      ovrTrackingCap_Position |
      0, 0);
  }
#endif

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
  // �Q�ƃJ�E���g��������
  --count;

#if STEREO == OCULUS
  // Oculus Rift �̃����_�����O�̐ݒ�������ݒ�ɖ߂�
  ovrHmd_ConfigureRendering(hmd, nullptr, 0, nullptr, nullptr);

  // Oculus Rift �̃f�o�C�X��j������
  ovrHmd_Destroy(hmd);
#endif

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

#if STEREO == NONE || STEREO == QUADBUFFER
  // �E�B���h�E�S�̂��r���[�|�[�g�ɂ���
  glViewport(0, 0, winW, winH);
#elif STEREO == OCULUS
  // �t���[���̃^�C�~���O�v���J�n
  frameTiming = ovrHmd_BeginFrame(hmd, 0);

  // FBO �ɕ`�悷��
  glBindFramebuffer(GL_FRAMEBUFFER, ocuFbo);
  glDrawBuffers(1, ocuFboDrawBuffers);
#endif

  // �J���[�o�b�t�@�ƃf�v�X�o�b�t�@����������
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

  // �C�x���g�����o��
  glfwPollEvents();

#if STEREO != OCULUS
  // �J���[�o�b�t�@�����ւ���
  glfwSwapBuffers(window);
#else
  // ���N�ƈ��S�Ɋւ���x���̕\����Ԃ��擾����
  ovrHSWDisplayState hswDisplayState;
  ovrHmd_GetHSWDisplayState(hmd, &hswDisplayState);

  // �x���\�������Ă����
  if (hswDisplayState.Displayed)
  {
    if (key)
    {
      // �����L�[���^�C�v���Ă���Όx��������
      ovrHmd_DismissHSWDisplay(hmd);
    }
    else
    {
      // Oculus Rift ��������y�������������ǂ��������o����
      const ovrTrackingState ts(ovrHmd_GetTrackingState(hmd, ovr_GetTimeInSeconds()));

      // �����̕ω������o���ꂽ��
      if (ts.StatusFlags & ovrStatus_OrientationTracked)
      {
        // ���̃Z���T�[�̉����x���擾����
        const ovrVector3f a(ts.RawSensorData.Accelerometer);

        // �����x�����ȏゾ������x��������
        if (a.x * a.x + a.y * a.y + a.z * a.z > 10000.0f) ovrHmd_DismissHSWDisplay(hmd);
      }
    }
  }

  // FBO �ւ̕`����I������
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

  // �}�E�X�̈ʒu�𒲂ׂ�
  double x, y;
  glfwGetCursorPos(window, &x, &y);

  // ���x�����x�ɔ�Ⴓ����
  const float speedFactor((fabs(ez) + 0.2f));

  // ���{�^���h���b�O
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1))
  {
    // �J�����̈ʒu���ړ�����
    const GLfloat speed(static_cast<GLfloat>(cy - y) * speedScale * speedFactor);
    ex += speed * sin(direction);
    ey += speed * cos(direction);

    // �J�����̐i�s������ς���
    direction += angleScale * static_cast<GLfloat>(x - cx);
  }

  // �E�{�^���h���b�O
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2))
  {
    // �}�E�X�{�^�����������ʒu����̕ψ�
    const GLfloat dx(static_cast<GLfloat>(x - cx));
    const GLfloat dy(static_cast<GLfloat>(y - cy));

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

  // �Q�[���p�b�h�ɂ�鑀��
  if (joy >= 0)
  {
    // �X�e�B�b�N
    int axesCount;
    const float *const axes(glfwGetJoystickAxes(joy, &axesCount));

    if (axesCount > 3 + axesOffset)
    {
      // �X�e�B�b�N�̑��x�W��
      GLfloat axesSpeedFactor = axesSpeedScale * speedFactor;

      // �J������O��Ɉړ�����
      const GLfloat advSpeed((axes[1] - origin[1]) * axesSpeedFactor);
      ex -= advSpeed * sin(direction);
      ey -= advSpeed * cos(direction);

      // �J���������E�Ɉړ�����
      const GLfloat latSpeed((axes[2 + axesOffset] - origin[2]) * axesSpeedFactor);
      ey -= latSpeed * sin(direction);
      ex += latSpeed * cos(direction);

      // �J�������㉺�Ɉړ�����
      ez -= (axes[3 + axesOffset] - origin[3]) * axesSpeedFactor;

      // �J�����̐i�s�������X�V����
      direction += (axes[0] - origin[0]) * axesAngleScale;
    }

    // �{�^��
    int btnsCount;
    const unsigned char *const btns(glfwGetJoystickButtons(joy, &btnsCount));
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

#if STEREO != NONE
#  if STEREO != OCULUS
  // �E���L�[����
  if (glfwGetKey(window, GLFW_KEY_RIGHT))
  {
    // �������g�傷��
    parallax += parallaxStep;
    updateStereoProjectionMatrix();
  }

  // �����L�[����
  if (glfwGetKey(window, GLFW_KEY_LEFT))
  {
    // �������k������
    parallax -= parallaxStep;
    updateStereoProjectionMatrix();
  }
#  else
  // �t���[���̃^�C�~���O�v���I��
  ovrHmd_EndFrame(hmd, eyePose, &eyeTexture[0].Texture);
#  endif
#endif
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
#if STEREO != OCULUS
    // �f�B�X�v���C�̃A�X�y�N�g�� w / h ����X�N���[���̕������߂�
    instance->scrW = instance->scrH * static_cast<GLfloat>(width) / static_cast<GLfloat>(height);

#  if STEREO == SIDEBYSIDE
    // �E�B���h�E�̉��������r���[�|�[�g�ɂ���
    width /= 2;
#  elif STEREO == TOPANDBOTTOM
    // �E�B���h�E�̏c�������r���[�|�[�g�ɂ���
    height /= 2;
#  endif
#endif

#if STEREO != OCULUS
    // �r���[�|�[�g�̑傫����ۑ����Ă���
    instance->winW = width;
    instance->winH = height;

#  if STEREO == NONE
    // �P�ᎋ�p�̓��e�ϊ��s������߂�
    instance->updateProjectionMatrix();
#  else
    // ���̎��p�̓��e�ϊ��s������߂�
    instance->updateStereoProjectionMatrix();
#  endif
#endif
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
    case GLFW_MOUSE_BUTTON_2:
      if (action)
      {
        // �h���b�O�J�n�ʒu��ۑ�����
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
    if (fabs(x) > fabs(y))
    {
      // �J���������E�Ɉړ�����
      const GLfloat latSpeed((fabs(instance->ez) * 5.0f + 1.0f) * wheelXStep * static_cast<GLfloat>(x));
      instance->ey -= latSpeed * sin(instance->direction);
      instance->ex += latSpeed * cos(instance->direction);
    }
    else
    {
      // �J�������㉺�Ɉړ�����
      const GLfloat advSpeed((fabs(instance->ez) * 5.0f + 1.0f) * wheelYStep * static_cast<GLfloat>(y));
      instance->ez += advSpeed;
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
        // �J�����̈ʒu�����Z�b�g����
        instance->ex = startPosition[0];
        instance->ey = startPosition[1];
        instance->ez = startPosition[2];
        instance->direction = 0.0f;
      case GLFW_KEY_O:
        // �J�����̌��������Z�b�g����
        instance->pitch = 0.0f;
      case GLFW_KEY_H:
        // �J�����̕��ʊp���������Z�b�g����
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
// ���ڗp�̃��f���r���[�ϊ��s��𓾂�
//
//   �E���ڂ̕`����L�̏������s��
//
GgMatrix Window::getMwL()
{
#  if STEREO != OCULUS
#    if STEREO == LINEBYLINE
  // �����Ԗڂ̑����������ɕ`�悷��
#    elif STEREO == TOPANDBOTTOM
  // �f�B�X�v���C�̏㔼�������ɕ`�悷��
  glViewport(0, winH, winW, winH);
#    elif STEREO == SIDEBYSIDE
  // �f�B�X�v���C�̍����������ɕ`�悷��
  glViewport(0, 0, winW, winH);
#    elif STEREO == QUADBUFFER
  // ���ڗp�o�b�t�@�ɕ`�悷��
  glDrawBuffer(GL_BACK_LEFT);
#    endif

  // ���ڂ����ɓ���������ɃV�[�����E�ɓ�����
  return ggTranslate(parallax, 0.0f, 0.0f) * mv;
#  else
  // Oculus Rift �̍��ڂ̎��ʎq
  const ovrEyeType &eyeL(hmd->EyeRenderOrder[0]);

  // Oculus Rift �̉E�ڂ̃r���[�|�[�g�ɕ`�悷��
  glViewport(eyeRenderViewport[eyeL].Pos.x, eyeRenderViewport[eyeL].Pos.y,
    eyeRenderViewport[eyeL].Size.w, eyeRenderViewport[eyeL].Size.h);

  // Oculus Rift �̍��ڂ̎p�����擾����
  eyePose[0] = ovrHmd_GetHmdPosePerEye(hmd, eyeL);

  // Oculus Rift �̍��ڂ̈ʒu�ƌ������擾����
  const ovrQuatf &o(eyePose[0].Orientation);
  const ovrVector3f &p(eyePose[0].Position);
  const ovrVector3f &q(eyeRenderDesc[eyeL].HmdToEyeViewOffset);

  // Oculus Rift �̉E�ڂ̌��������f���r���[�ϊ��s��ɔ��f����
  return ggQuaternionTransposeMatrix(GgQuaternion(o.x, o.y, o.z, o.w)) * ggTranslate(q.x - p.x, q.y - p.y, q.z - p.z) * mv;
#  endif
}

//
// �E�ڗp�̃��f���r���[�ϊ��s��𓾂�
//
//   �E�E�ڂ̕`����L�̏������s��
//
GgMatrix Window::getMwR()
{
#  if STEREO != OCULUS
#    if STEREO == LINEBYLINE
  // ��Ԗڂ̑����������ɕ`�悷��
#    elif STEREO == TOPANDBOTTOM
  // �f�B�X�v���C�̉����������ɕ`�悷��
  glViewport(0, 0, winW, winH);
#    elif STEREO == SIDEBYSIDE
  // �f�B�X�v���C�̉E���������ɕ`�悷��
  glViewport(winW, 0, winW, winH);
#    elif STEREO == QUADBUFFER
  // �E�ڗp�o�b�t�@�ɕ`�悷��
  glDrawBuffer(GL_BACK_RIGHT);
#    endif

  // �E�ڂ����ɓ���������ɃV�[�������ɓ�����
  return ggTranslate(-parallax, 0.0f, 0.0f) * mv;
#  else
  // Oculus Rift �̉E�ڂ̎��ʎq
  const ovrEyeType &eyeR(hmd->EyeRenderOrder[1]);

  // Oculus Rift �̉E�ڂ̃r���[�|�[�g��ݒ肷��
  glViewport(eyeRenderViewport[eyeR].Pos.x, eyeRenderViewport[eyeR].Pos.y,
    eyeRenderViewport[eyeR].Size.w, eyeRenderViewport[eyeR].Size.h);

  // Oculus Rift �̉E�ڂ̎p�����擾����
  eyePose[1] = ovrHmd_GetHmdPosePerEye(hmd, eyeR);

  // Oculus Rift �̉E�ڂ̈ʒu�ƌ������擾����
  const ovrQuatf &o(eyePose[1].Orientation);
  const ovrVector3f &p(eyePose[1].Position);
  const ovrVector3f &q(eyeRenderDesc[eyeR].HmdToEyeViewOffset);

  // Oculus Rift �̉E�ڂ̌��������f���r���[�ϊ��s��ɔ��f����
  return ggQuaternionTransposeMatrix(GgQuaternion(o.x, o.y, o.z, o.w)) * ggTranslate(q.x - p.x, q.y - p.y, q.z - p.z) * mv;
#  endif
}

#  if STEREO == OCULUS
// Oculus Rift �\���p�� FBO �̃����_�[�^�[�Q�b�g
const GLenum Window::ocuFboDrawBuffers[] = { GL_COLOR_ATTACHMENT0 };
#  endif
#endif

// �Q�ƃJ�E���g
unsigned int Window::count(0);
