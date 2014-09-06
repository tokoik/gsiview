#version 150 core
#extension GL_ARB_explicit_attrib_location : enable

// �e�N�X�`��
uniform sampler2D cmap;                             // �g�U���ːF

// ���X�^���C�U����󂯎�钸�_�����̕�Ԓl
in vec4 idiff;                                      // �g�U���ˌ����x
in vec4 ispec;                                      // ���ʔ��ˌ����x
in vec2 t;                                          // �e�N�X�`�����W

// �t���[���o�b�t�@�ɏo�͂���f�[�^
layout (location = 0) out vec4 fc;                  // �t���O�����g�̐F

void main()
{
  fc = texture(cmap, t) * idiff + ispec;
}
