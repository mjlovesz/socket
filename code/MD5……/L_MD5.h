#pragma once
#define L_MD5_SELF_(x) ((x)-1) //ʹ���Լ�����ĳ�ʼ���ݡ����������MD5ֵ���׼��һ��
namespace  L_MD5
{
	void calMD5(char* out,unsigned char * in, unsigned long long sizehigh);
};
#define L_SHA_SELF_(x) ((x)-1)
namespace  L_SHA1
{
	void calSHA1(char* out, unsigned char * in, unsigned long long sizehigh);
};

