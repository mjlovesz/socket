#include "L_MD5.h"
#include <iostream>

typedef unsigned long L_UINT;
typedef unsigned char L_UINT8;
typedef unsigned long long L_UINT64;
typedef L_UINT (*fX)(L_UINT,L_UINT,L_UINT);

namespace L_SAFE_TOOL{
	//转换大小头函数
	template <typename T>
	T TO_BIG_ENDIAN(T data)
	{
		int n = sizeof(T)-1;
		char * cdata = (char *)(&data);
		for (int i = 0; (i<<1) < n; ++i)
		{
			cdata[i]		= cdata[i] ^ cdata[n-i];
			cdata[n - i]	= cdata[i] ^ cdata[n-i];
			cdata[i]		= cdata[i] ^ cdata[n-i];
		}
		return data;
	};

	template <typename T>
	T& SELF_TO_BIG_ENDIAN(T& data)
	{
		int n = sizeof(T)-1;
		char * cdata = (char *)(&data);
		for (int i = 0; (i<<1) < n; ++i)
		{
			cdata[i]		= cdata[i] ^ cdata[n-i];
			cdata[n - i]	= cdata[i] ^ cdata[n-i];
			cdata[i]		= cdata[i] ^ cdata[n-i];
		}
		return data;
	};
	//计算函数
	inline L_UINT F(L_UINT X,L_UINT Y,L_UINT Z)
	{
		return (X&Y)|((~X)&Z);
	};
	inline L_UINT G(L_UINT X,L_UINT Y,L_UINT Z)
	{
		return (X&Z)|(Y&(~Z));
	};
	inline L_UINT H(L_UINT X,L_UINT Y,L_UINT Z)
	{
		return X^Y^Z;
	};
	inline L_UINT I(L_UINT X,L_UINT Y,L_UINT Z)
	{
		return Y^(X|(~Z));
	};
	inline L_UINT J(L_UINT X,L_UINT Y,L_UINT Z)
	{
		return (X&Y)|(X&Z)|(Y&Z);
	};
	//循环左移
	L_UINT inline Sn( L_UINT x, L_UINT s )
	{
		return ( x << s ) | ( x >> ( 32 - s ) );
	}
	//填充到512位
	L_UINT Fill512(L_UINT uOut[32],L_UINT8* uIN,L_UINT uSizeLeft,L_UINT64 size,bool bBig_Flag = false)
	{
		L_UINT8 * cOut = (L_UINT8*)uOut;
		memset(uOut, 0, sizeof(L_UINT) * sizeof(uOut));
		memmove(uOut, uIN, uSizeLeft);
		*(((L_UINT8*)uOut) + uSizeLeft) = (L_UINT8)0x80;
		size = size * sizeof(L_UINT8) * 8;
		if (bBig_Flag)
			SELF_TO_BIG_ENDIAN(size);
		if (uSizeLeft > 14 * 4)
		{
			*((L_UINT64*)(uOut + 14 + 16)) = size;
			return 64 * 2 / sizeof(L_UINT8);
		}
		else
		{
			*((L_UINT64*)(uOut + 14)) = size;
			return 64 / sizeof(L_UINT8);
		}
	}
	//
}
using namespace L_SAFE_TOOL;


namespace L_MD5
{
#ifndef L_MD5_SELF
	const static L_UINT m_sin_n[64]
	={	0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,
		0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be,0x6b901122,0xfd987193,0xa679438e,0x49b40821,
		0xf61e2562,0xc040b340,0x265e5a51,0xe9b6c7aa,0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,
		0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,0xa9e3e905,0xfcefa3f8,0x676f02d9,0x8d2a4c8a,
		0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c,0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,
		0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05,0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,
		0xf4292244,0x432aff97,0xab9423a7,0xfc93a039,0x655b59c3,0x8f0ccc92,0xffeff47d,0x85845dd1,
		0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1,0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391
	};
	const static L_UINT m_s[4][4]
	={
		{7, 12, 17, 22},
		{5,  9, 14, 20},
		{4, 11, 16, 23},
		{6, 10, 15, 21}
	};
	const static L_UINT m_k[4][16]
	={
		{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},//0+1
		{1,6,11,0,5,10,15,4,9,14,3,8,13,2,7,12},//1+5
		{5,8,11,14,1,4,7,10,13,0,3,6,9,12,15,2},//5+3
		{0,7,14,5,12,3,10,1,8,15,6,13,4,11,2,9},//0+7
	};
	const static L_UINT ABCD_Orange[4] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};
#else
	const L_UINT m_sin_n[64]
	={	L_MD5_SELF(0xd76aa478),L_MD5_SELF(0xe8c7b756),L_MD5_SELF(0x242070db),L_MD5_SELF(0xc1bdceee),L_MD5_SELF(0xf57c0faf),L_MD5_SELF(0x4787c62a),L_MD5_SELF(0xa8304613),L_MD5_SELF(0xfd469501),
		L_MD5_SELF(0x698098d8),L_MD5_SELF(0x8b44f7af),L_MD5_SELF(0xffff5bb1),L_MD5_SELF(0x895cd7be),L_MD5_SELF(0x6b901122),L_MD5_SELF(0xfd987193),L_MD5_SELF(0xa679438e),L_MD5_SELF(0x49b40821),
		L_MD5_SELF(0xf61e2562),L_MD5_SELF(0xc040b340),L_MD5_SELF(0x265e5a51),L_MD5_SELF(0xe9b6c7aa),L_MD5_SELF(0xd62f105d),L_MD5_SELF(0x02441453),L_MD5_SELF(0xd8a1e681),L_MD5_SELF(0xe7d3fbc8),
		L_MD5_SELF(0x21e1cde6),L_MD5_SELF(0xc33707d6),L_MD5_SELF(0xf4d50d87),L_MD5_SELF(0x455a14ed),L_MD5_SELF(0xa9e3e905),L_MD5_SELF(0xfcefa3f8),L_MD5_SELF(0x676f02d9),L_MD5_SELF(0x8d2a4c8a),
		L_MD5_SELF(0xfffa3942),L_MD5_SELF(0x8771f681),L_MD5_SELF(0x6d9d6122),L_MD5_SELF(0xfde5380c),L_MD5_SELF(0xa4beea44),L_MD5_SELF(0x4bdecfa9),L_MD5_SELF(0xf6bb4b60),L_MD5_SELF(0xbebfbc70),
		L_MD5_SELF(0x289b7ec6),L_MD5_SELF(0xeaa127fa),L_MD5_SELF(0xd4ef3085),L_MD5_SELF(0x04881d05),L_MD5_SELF(0xd9d4d039),L_MD5_SELF(0xe6db99e5),L_MD5_SELF(0x1fa27cf8),L_MD5_SELF(0xc4ac5665),
		L_MD5_SELF(0xf4292244),L_MD5_SELF(0x432aff97),L_MD5_SELF(0xab9423a7),L_MD5_SELF(0xfc93a039),L_MD5_SELF(0x655b59c3),L_MD5_SELF(0x8f0ccc92),L_MD5_SELF(0xffeff47d),L_MD5_SELF(0x85845dd1),
		L_MD5_SELF(0x6fa87e4f),L_MD5_SELF(0xfe2ce6e0),L_MD5_SELF(0xa3014314),L_MD5_SELF(0x4e0811a1),L_MD5_SELF(0xf7537e82),L_MD5_SELF(0xbd3af235),L_MD5_SELF(0x2ad7d2bb),L_MD5_SELF(0xeb86d391)
	};
	const L_UINT m_s[4][4]
	={
		{7, 12, 17, 22},
		{5,  9, 14, 20},
		{4, 11, 16, 23},
		{6, 10, 15, 21}
	};
	const static L_UINT m_k[4][16]
	={
		{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},//0+1
		{1,6,11,0,5,10,15,4,9,14,3,8,13,2,7,12},//1+5
		{5,8,11,14,1,4,7,10,13,0,3,6,9,12,15,2},//5+3
		{0,7,14,5,12,3,10,1,8,15,6,13,4,11,2,9},//0+7
	};
	const static L_UINT ABCD_Orange[4] = {L_MD5_SELF(0x67452301),L_MD5_SELF( 0xefcdab89),L_MD5_SELF( 0x98badcfe),L_MD5_SELF( 0x10325476)};
#endif // !L_MD5_SAFE
	static inline L_UINT XX(L_UINT& W,L_UINT X,L_UINT XYZ,L_UINT Mj,L_UINT s,L_UINT ti);
	inline void	One512(L_UINT* ABCD,L_UINT* u16_32);
}

inline L_UINT L_MD5::XX(L_UINT& W,L_UINT X,L_UINT XYZ,L_UINT Mj,L_UINT s,L_UINT ti)
{
	return W = X + Sn((W + XYZ + Mj + ti), s);
}

void L_MD5::One512(L_UINT* ABCD,L_UINT* u16_32)
{
	L_UINT ABCDtmp[4];
	memmove(ABCDtmp,ABCD,sizeof(L_UINT) * 4);

	for (int j = 0; j < 4; ++j)
	{
		XX(ABCDtmp[0],ABCDtmp[1],F(ABCDtmp[1],ABCDtmp[2],ABCDtmp[3]),u16_32[m_k[0][j * 4 + 0]],m_s[0][0],m_sin_n[0 * 16 + j * 4  + 0]);
		XX(ABCDtmp[3],ABCDtmp[0],F(ABCDtmp[0],ABCDtmp[1],ABCDtmp[2]),u16_32[m_k[0][j * 4 + 1]],m_s[0][1],m_sin_n[0 * 16 + j * 4  + 1]);
		XX(ABCDtmp[2],ABCDtmp[3],F(ABCDtmp[3],ABCDtmp[0],ABCDtmp[1]),u16_32[m_k[0][j * 4 + 2]],m_s[0][2],m_sin_n[0 * 16 + j * 4  + 2]);
		XX(ABCDtmp[1],ABCDtmp[2],F(ABCDtmp[2],ABCDtmp[3],ABCDtmp[0]),u16_32[m_k[0][j * 4 + 3]],m_s[0][3],m_sin_n[0 * 16 + j * 4  + 3]);
	}
	for (int j = 0; j < 4; ++j)
	{
		XX(ABCDtmp[0],ABCDtmp[1],G(ABCDtmp[1],ABCDtmp[2],ABCDtmp[3]),u16_32[m_k[1][j * 4 + 0]],m_s[1][0],m_sin_n[1 * 16 + j * 4  + 0]);
		XX(ABCDtmp[3],ABCDtmp[0],G(ABCDtmp[0],ABCDtmp[1],ABCDtmp[2]),u16_32[m_k[1][j * 4 + 1]],m_s[1][1],m_sin_n[1 * 16 + j * 4  + 1]);
		XX(ABCDtmp[2],ABCDtmp[3],G(ABCDtmp[3],ABCDtmp[0],ABCDtmp[1]),u16_32[m_k[1][j * 4 + 2]],m_s[1][2],m_sin_n[1 * 16 + j * 4  + 2]);
		XX(ABCDtmp[1],ABCDtmp[2],G(ABCDtmp[2],ABCDtmp[3],ABCDtmp[0]),u16_32[m_k[1][j * 4 + 3]],m_s[1][3],m_sin_n[1 * 16 + j * 4  + 3]);
	}
	for (int j = 0; j < 4; ++j)
	{
		XX(ABCDtmp[0],ABCDtmp[1],H(ABCDtmp[1],ABCDtmp[2],ABCDtmp[3]),u16_32[m_k[2][j * 4 + 0]],m_s[2][0],m_sin_n[2 * 16 + j * 4  + 0]);
		XX(ABCDtmp[3],ABCDtmp[0],H(ABCDtmp[0],ABCDtmp[1],ABCDtmp[2]),u16_32[m_k[2][j * 4 + 1]],m_s[2][1],m_sin_n[2 * 16 + j * 4  + 1]);
		XX(ABCDtmp[2],ABCDtmp[3],H(ABCDtmp[3],ABCDtmp[0],ABCDtmp[1]),u16_32[m_k[2][j * 4 + 2]],m_s[2][2],m_sin_n[2 * 16 + j * 4  + 2]);
		XX(ABCDtmp[1],ABCDtmp[2],H(ABCDtmp[2],ABCDtmp[3],ABCDtmp[0]),u16_32[m_k[2][j * 4 + 3]],m_s[2][3],m_sin_n[2 * 16 + j * 4  + 3]);
	}
	for (int j = 0; j < 4; ++j)
	{
		XX(ABCDtmp[0],ABCDtmp[1],I(ABCDtmp[1],ABCDtmp[2],ABCDtmp[3]),u16_32[m_k[3][j * 4 + 0]],m_s[3][0],m_sin_n[3 * 16 + j * 4  + 0]);
		XX(ABCDtmp[3],ABCDtmp[0],I(ABCDtmp[0],ABCDtmp[1],ABCDtmp[2]),u16_32[m_k[3][j * 4 + 1]],m_s[3][1],m_sin_n[3 * 16 + j * 4  + 1]);
		XX(ABCDtmp[2],ABCDtmp[3],I(ABCDtmp[3],ABCDtmp[0],ABCDtmp[1]),u16_32[m_k[3][j * 4 + 2]],m_s[3][2],m_sin_n[3 * 16 + j * 4  + 2]);
		XX(ABCDtmp[1],ABCDtmp[2],I(ABCDtmp[2],ABCDtmp[3],ABCDtmp[0]),u16_32[m_k[3][j * 4 + 3]],m_s[3][3],m_sin_n[3 * 16 + j * 4  + 3]);
	}

	ABCD[0] += ABCDtmp[0];
	ABCD[1] += ABCDtmp[1];
	ABCD[2] += ABCDtmp[2];
	ABCD[3] += ABCDtmp[3];
}
//out必须大于32
void L_MD5::calMD5(char* out, L_UINT8 * in, L_UINT64 size)
{
	L_UINT64 llsize = size;
	L_UINT ABCD[4];
	memmove(ABCD,ABCD_Orange,sizeof(L_UINT)*4);
	L_UINT8 * cABCD = (L_UINT8 *)ABCD;
	L_UINT* uIN = (L_UINT*)in;

	while (llsize >= 64 / sizeof(L_UINT8))
	{
		One512(ABCD,uIN);
		uIN += 64 / sizeof(L_UINT);
		llsize -= 64 / sizeof(L_UINT8);
	}

	L_UINT uOut[32]={0};
	llsize = Fill512(uOut, (L_UINT8*)uIN, (L_UINT)llsize, size);//填充
	uIN = uOut;
	while (llsize >= 64 / sizeof(L_UINT8))
	{
		One512(ABCD,uIN);
		uIN += 64 / sizeof(L_UINT);
		llsize -= 64 / sizeof(L_UINT8);
	}

	//out = (char*)realloc(out,33);//传进来要保证长度大于32
	for (int i = 0; i < sizeof(L_UINT) * 4 / sizeof(char); ++i)
		sprintf_s (out + i * 2, 3, "%02x", cABCD[i]);
}

namespace  L_SHA1
{
#ifndef L_SHA_SELF
	static const L_UINT K[] = {0x5A827999,0x6ED9EBA1,0x8F1BBCDC,0xCA62C1D6};
	static const L_UINT ABCDE_orange[]={0x67452301,0xEFCDAB89,0x98BADCFE,0x10325476,0xC3D2E1F0};
#else
	static const L_UINT K[] = {L_SHA_SELF(0x5A827999),L_SHA_SELF(0x6ED9EBA1),L_SHA_SELF(0x8F1BBCDC),L_SHA_SELF(0xCA62C1D6-1)};
	static const L_UINT ABCDE_orange[]={L_SHA_SELF(0x67452301),L_SHA_SELF(0xEFCDAB89),L_SHA_SELF(0x98BADCFE),L_SHA_SELF(0x10325476),L_SHA_SELF(0xC3D2E1F0)};
#endif	
	inline void	One512(L_UINT* ABCDE,L_UINT* u16_32);
};

inline void	L_SHA1::One512(L_UINT* ABCDE,L_UINT* u16_32)
{
	L_UINT ABCDEtmp[5];
	memmove(ABCDEtmp,ABCDE,sizeof(L_UINT) * 5);
	L_UINT W[80];
	memmove(W,u16_32,sizeof(L_UINT) * 16);
	for (int i = 0; i < 16; ++i)
	{
		W[i] = TO_BIG_ENDIAN(W[i]);
	}
	for (int i = 16; i < 80; ++i)
	{
		W[i] = Sn((W[i-3] ^ W[i-8] ^ W[i-14] ^ W[i-16]),1);
	}
	L_UINT TEMP;
	for (int j = 0; j < 20; ++j)
	{
		TEMP = Sn(ABCDEtmp[0],5) + F(ABCDEtmp[1],ABCDEtmp[2],ABCDEtmp[3]) + ABCDEtmp[4] + W[0 * 20 + j] + K[0];
		ABCDEtmp[4] = ABCDEtmp[3];  ABCDEtmp[3] = ABCDEtmp[2];  ABCDEtmp[2] = Sn(ABCDEtmp[1],30); ABCDEtmp[1] = ABCDEtmp[0];  ABCDEtmp[0] = TEMP;
	}
	for (int j = 0; j < 20; ++j)
	{
		TEMP = Sn(ABCDEtmp[0],5) + H(ABCDEtmp[1],ABCDEtmp[2],ABCDEtmp[3]) + ABCDEtmp[4] + W[1 * 20 + j] + K[1];
		ABCDEtmp[4] = ABCDEtmp[3];  ABCDEtmp[3] = ABCDEtmp[2];  ABCDEtmp[2] = Sn(ABCDEtmp[1],30); ABCDEtmp[1] = ABCDEtmp[0];  ABCDEtmp[0] = TEMP;
	}
	for (int j = 0; j < 20; ++j)
	{
		TEMP = Sn(ABCDEtmp[0],5) + J(ABCDEtmp[1],ABCDEtmp[2],ABCDEtmp[3]) + ABCDEtmp[4] + W[2 * 20 + j] + K[2];
		ABCDEtmp[4] = ABCDEtmp[3];  ABCDEtmp[3] = ABCDEtmp[2];  ABCDEtmp[2] = Sn(ABCDEtmp[1],30); ABCDEtmp[1] = ABCDEtmp[0];  ABCDEtmp[0] = TEMP;
	}
	for (int j = 0; j < 20; ++j)
	{
		TEMP = Sn(ABCDEtmp[0],5) + H(ABCDEtmp[1],ABCDEtmp[2],ABCDEtmp[3]) + ABCDEtmp[4] + W[3 * 20 + j] + K[3];
		ABCDEtmp[4] = ABCDEtmp[3];  ABCDEtmp[3] = ABCDEtmp[2];  ABCDEtmp[2] = Sn(ABCDEtmp[1],30); ABCDEtmp[1] = ABCDEtmp[0];  ABCDEtmp[0] = TEMP;
	}


	ABCDE[0] += ABCDEtmp[0];
	ABCDE[1] += ABCDEtmp[1];
	ABCDE[2] += ABCDEtmp[2];
	ABCDE[3] += ABCDEtmp[3];
	ABCDE[4] += ABCDEtmp[4];
}
//需要保证入参out能大于40个
void L_SHA1::calSHA1(char* out, L_UINT8 * in, unsigned long long size)
{
	L_UINT64 llsize = size;

	L_UINT ABCDE[5]={0x67452301,0xEFCDAB89,0x98BADCFE,0x10325476,0xC3D2E1F0};
	L_UINT8 * cABCDE = (L_UINT8 *)ABCDE;
	L_UINT* uIN = (L_UINT*)in;
	while (llsize >= 64 / sizeof(L_UINT8))
	{
		One512(ABCDE,uIN);
		uIN += 64 / sizeof(L_UINT);
		llsize -= 64 / sizeof(L_UINT8);
	}
	L_UINT uOut[32]={0};
	llsize = Fill512(uOut, (L_UINT8*)uIN, (L_UINT)llsize,size,true);
	uIN = uOut;
	while (llsize >= 64 / sizeof(L_UINT8))
	{
		One512(ABCDE,uIN);
		uIN += 64 / sizeof(L_UINT);
		llsize -= 64 / sizeof(L_UINT8);
	}
	for (int i=0 ; i<5;++i)
	{
		ABCDE[i] = TO_BIG_ENDIAN(ABCDE[i]);
	}
	//out = (char*)realloc(out,33);//传进来要保证长度大于32
	for (int i = 0; i < sizeof(L_UINT) * 5 / sizeof(char); ++i)
		sprintf_s (out + i * 2, 3, "%02x", cABCDE[i]);
}

