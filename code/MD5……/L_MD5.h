#pragma once
#define L_MD5_SAFE 
namespace  L_MD5
{
	void calMD5(char* out, char * in, unsigned long long sizehigh);
};
#define L_SHA1_SAFE 
namespace  L_SHA1
{
	void calSHA1(char* out, char * in, unsigned long long sizehigh);
};

