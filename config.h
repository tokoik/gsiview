#pragma once

//
// �e��ݒ�
//

// �⏕�v���O����
#include "gg.h"
using namespace gg;

// ���̎��̐ݒ�
#define NONE          0                                 // �P�ᎋ
#define LINEBYLINE    1                                 // �C���^�[���[�X�i�������j
#define TOPANDBOTTOM  2                                 // �㉺
#define SIDEBYSIDE    3                                 // ���E
#define QUADBUFFER    4                                 // �N���b�h�o�b�t�@�X�e���I
#define OCULUS        5                                 // Oculus Rift (HMD)

// ���̎��̕���
#define STEREO        OCULUS

// �Z�J���_�����j�^�̎g�p
const int useSecondary(1);                              // 1 �Ȃ�Z�J���_�����j�^�ɕ\��

// �n�`�f�[�^
const char demfile[] = "dem.csv";                       // �f�W�^���W���n�}�f�[�^
const GLfloat demscale[] = { 5.0f, 5.0f, 0.1f, 1.0f };  // �n�}�f�[�^�̃X�P�[��
const char texfile[] = "texture.png";                   // �n�}�̃e�N�X�`��
#define USE_ANISOTROPIC_FILTERING 1                     // 1 �Ȃ��Ώ̃t�B���^�����O�g���@�\���g��

// �J�����̏������ (�P�� m)
const GLfloat startPosition[] = { 0.0f, 0.0f, 5.0f };   // �J�����̏����ʒu
const GLfloat startPitch(-1.5707963f);                  // �ŏ��͐��ʂ������Ă���
const GLfloat displayCenter(0.5f);                      // �f�B�X�v���C�̒��S�ʒu (�����̔���)
const GLfloat displayDistance(1.5f);                    // �ϑ��҂ƃf�B�X�v���C�ʂƂ̋���
const GLfloat zNear(0.1f);                              // �O���ʂ܂ł̋���
const GLfloat zFar(500.0f);                             // ����ʂ܂ł̋���

// ����
const GLfloat initialParallax(0.032f);                  // �����̏����l (�P�� m)
const GLfloat parallaxStep(0.001f);                     // �����̕ύX�X�e�b�v (�P�� m)

// �i�r�Q�[�V�����̑��x����
const GLfloat initialZoom(1.0f);                        // �Y�[�����̏����l
const double zoomStep(0.01);                            // �Y�[���������̃X�e�b�v
const GLfloat speedScale(0.00002f);                     // �t���[��������̈ړ����x�W��
const GLfloat angleScale(0.00002f);                     // �t���[��������̉�]���x�W��
#if defined(__APPLE__)
const GLfloat wheelXStep(0.001f);                       // Magic Mouse �� X �����̌W��
const GLfloat wheelYStep(0.001f);                       // Magic Mouse �� Y �����̌W��
#else
const GLfloat wheelXStep(0.005f);                       // �}�E�X�z�C�[���� X �����̌W��
const GLfloat wheelYStep(0.005f);                       // �}�E�X�z�C�[���� Y �����̌W��
#endif
const GLfloat axesSpeedScale(0.01f);                    // �Q�[���p�b�h�̃X�e�B�b�N�̑��x�̌W��
const GLfloat axesAngleScale(0.01f);                    // �Q�[���p�b�h�̃X�e�B�b�N�̊p���x�̌W��
const GLfloat btnsScale(0.01f);                         // �Q�[���p�b�h�̃{�^���̌W��

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

// �w�i�F
const GLfloat back[] = { 0.2f, 0.3f, 0.4f, 0.0f };

// �f�o�b�O���[�h
#if defined(_DEBUG)
const bool debug(true);
#else
const bool debug(false);
#endif
