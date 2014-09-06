#pragma once

//
// �e��ݒ�
//

// �⏕�v���O����
#include "gg.h"
using namespace gg;

// ���̎��̐ݒ�
#define NONE          0                                 // �P�ᎋ
#define LINEBYLINE    1                                 // �C���^�[���[�X�i���T�|�[�g�j
#define TOPANDBOTTOM  2                                 // �㉺
#define SIDEBYSIDE    3                                 // ���E
#define QUADBUFFER    4                                 // �N���b�h�o�b�t�@�X�e���I
#define OCULUS        5                                 // Oculus Rift (HMD)

// ���̎��̕���
#define STEREO        NONE

// ���̎����L�̃p�����[�^
#if STEREO != NONE
const int useSecondary(1);                              // 1 �Ȃ�Z�J���_�����j�^�ɕ\��
const GLfloat initialParallax(0.032f);                  // �����̏����l (�P�� m)
const GLfloat parallaxStep(0.001f);                     // �����̕ύX�X�e�b�v (�P�� m)
const GLfloat screenDistance(2.0f);                     // ���ۂ̃X�N���[���܂ł̋��� (�P�� m)

// Oculus Rift �ŗL�̃p�����[�^
#  if STEREO == OCULUS
const GLfloat lensScaleStep(0.001f);                    // �����Y�̊g�嗦�̕␳�W���̒����X�e�b�v
const GLfloat projectionCenterOffsetStep(0.001f);       // �����Y�̒��S�ʒu�̒����X�e�b�v
const GLuint fboWidth(1024), fboHeight(1024);           // �␳�Ɏg�� FBO �̃T�C�Y
#  endif
#endif

// �n�`�f�[�^
const char demfile[] = "dem.csv";                       // �f�W�^���W���n�}�f�[�^
const GLfloat demscale[] = { 5.0f, 5.0f, 0.1f, 1.0f };  // �n�}�f�[�^�̃X�P�[��
const char texfile[] = "texture.png";                   // �n�}�̃e�N�X�`��
const GLsizei texWidth(2048), texHeight(2048);          // �e�N�X�`���������̃T�C�Y
#define USE_ANISOTROPIC_FILTERING 1                     // 1 �Ȃ��Ώ̃t�B���^�����O�g���@�\���g��

// �J�����̏������
const GLfloat startPosition[] = { 0.0f, 0.0f, 20.0f };  // �J�����̏����ʒu
const GLfloat displayCenter(0.5f);                      // �f�B�X�v���C�̒��S�ʒu (�����̔���
const GLfloat displayDepth(1.5f);                       // �ϑ��҂ƃf�B�X�v���C�ʂƂ̋���
const GLfloat zNear(0.1f);                              // �O���ʂ܂ł̋���
const GLfloat zFar(50.0f);                              // ����ʂ܂ł̋���

// �i�r�Q�[�V�����̑��x����
const GLfloat speedScale(0.00005f);                     // �t���[��������̈ړ����x�W��
const GLfloat angleScale(0.00001f);                     // �t���[��������̉�]���x�W��
const GLfloat heightStep(0.005f);                       // �J�����̍����̒����W��
const GLfloat axesSpeedScale(0.020f);                   // �Q�[���p�b�h�̃X�e�B�b�N�̑��x�̌W��
const GLfloat axesAngleScale(0.005f);                   // �Q�[���p�b�h�̃X�e�B�b�N�̊p���x�̌W��
const GLfloat btnsScale(0.005f);                        // �Q�[���p�b�h�̃{�^���̌W��

// ����
const GgSimpleShader::Light light =
{
  { 0.4f, 0.4f, 0.4f, 1.0f },                           // ��������
  { 0.8f, 0.8f, 0.8f, 1.0f },                           // �g�U���ˌ�����
  { 0.8f, 0.8f, 0.8f, 1.0f },                           // ���ʌ�����
  { 0.0f, 0.5f, 1.0f, 0.0f }                            // �ʒu
};

// �ގ�
const GgSimpleShader::Material material =
{
  { 0.8f, 0.8f, 0.8f, 1.0f },                           // �����̔��ˌW��
  { 0.8f, 0.8f, 0.8f, 1.0f },                           // �g�U���ˌW��
  { 0.2f, 0.2f, 0.2f, 1.0f },                           // ���ʔ��ˌW��
  50.0f                                                 // �P���W��
};

// ���E�F (Oculus Rift �\�����̕\���͈͊O�̐F)
const GLfloat border[] = { 0.0, 0.0, 0.0, 0.0 };
