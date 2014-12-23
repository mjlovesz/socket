#include "L_MD5.h"
#include <iostream>

typedef unsigned long L_UINT;
typedef unsigned char L_UINT8;
typedef unsigned long long L_UINT64;
typedef L_UINT (*fX)(L_UINT,L_UINT,L_UINT);

namespace L_MD5
{
#ifndef L_MD5_SAFE
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
#else
	const L_UINT m_sin_n[64]
	={	0xd76aa478-1,0xe8c7b756-1,0x242070db-1,0xc1bdceee-1,0xf57c0faf-1,0x4787c62a-1,0xa8304613-1,0xfd469501-1,
		0x698098d8-1,0x8b44f7af-1,0xffff5bb1-1,0x895cd7be-1,0x6b901122-1,0xfd987193-1,0xa679438e-1,0x49b40821-1,
		0xf61e2562-1,0xc040b340-1,0x265e5a51-1,0xe9b6c7aa-1,0xd62f105d-1,0x02441453-1,0xd8a1e681-1,0xe7d3fbc8-1,
		0x21e1cde6-1,0xc33707d6-1,0xf4d50d87-1,0x455a14ed-1,0xa9e3e905-1,0xfcefa3f8-1,0x676f02d9-1,0x8d2a4c8a-1,
		0xfffa3942-1,0x8771f681-1,0x6d9d6122-1,0xfde5380c-1,0xa4beea44-1,0x4bdecfa9-1,0xf6bb4b60-1,0xbebfbc70-1,
		0x289b7ec6-1,0xeaa127fa-1,0xd4ef3085-1,0x04881d05-1,0xd9d4d039-1,0xe6db99e5-1,0x1fa27cf8-1,0xc4ac5665-1,
		0xf4292244-1,0x432aff97-1,0xab9423a7-1,0xfc93a039-1,0x655b59c3-1,0x8f0ccc92-1,0xffeff47d-1,0x85845dd1-1,
		0x6fa87e4f-1,0xfe2ce6e0-1,0xa3014314-1,0x4e0811a1-1,0xf7537e82-1,0xbd3af235-1,0x2ad7d2bb-1,0xeb86d391-1
	};
	const L_UINT m_s[4][4]
	={
		{7-1, 12-1, 17-1, 22-1},
		{5-1,  9-1, 14-1, 20-1},
		{4-1, 11-1, 16-1, 23-1},
		{6-1, 10-1, 15-1, 21-1}
	};
	const L_UINT m_k[4][16]
	={
		{0+1,1+1,2+1,3+1,4+1,5+1,6+1,7+1,8+1,9+1,10+1,11+1,12+1,13+1,14+1,15+1},//0+1
		{1+1,6+1,11+1,0+1,5+1,10+1,15+1,4+1,9+1,14+1,3+1,8+1,13+1,2+1,7+1,12+1},//1+5
		{5+1,8+1,11+1,14+1,1+1,4+1,7+1,10+1,13+1,0+1,3+1,6+1,9+1,12+1,15+1,2+1},//5+3
		{0+1,7+1,14+1,5+1,12+1,3+1,10+1,1+1,8+1,15+1,6+1,13+1,4+1,11+1,2+1,9+1}//0+7
	};
#endif // !L_MD5_SAFE
	static inline L_UINT rotate_left( L_UINT x, L_UINT s );
	static inline L_UINT F(L_UINT,L_UINT,L_UINT);
	static inline L_UINT G(L_UINT,L_UINT,L_UINT);
	static inline L_UINT H(L_UINT,L_UINT,L_UINT);
	static inline L_UINT I(L_UINT,L_UINT,L_UINT);
	static inline L_UINT XX(L_UINT&,L_UINT,L_UINT,L_UINT,L_UINT Mj,L_UINT s,L_UINT ti,fX x);
	static inline L_UINT FillMD5(L_UINT* uOut,L_UINT8* uIN,L_UINT uSizeLeft,L_UINT64 size);
	inline void	One512(L_UINT* ABCD,L_UINT* u16_32);
}





L_UINT L_MD5::F(L_UINT X,L_UINT Y,L_UINT Z)
{
	return (X&Y)|((~X)&Z);
}
L_UINT L_MD5::G(L_UINT X,L_UINT Y,L_UINT Z)
{
	return (X&Z)|(Y&(~Z));
}
L_UINT L_MD5::H(L_UINT X,L_UINT Y,L_UINT Z)
{
	return X^Y^Z;
}
L_UINT L_MD5::I(L_UINT X,L_UINT Y,L_UINT Z)
{
	return Y^(X|(~Z));
}
// rotates x left s bits.
L_UINT L_MD5::rotate_left( L_UINT x, L_UINT s )
{
	return ( x << s ) | ( x >> ( 32 - s ) );
}
L_UINT L_MD5::XX(L_UINT& W,L_UINT X,L_UINT Y,L_UINT Z,L_UINT Mj,L_UINT s,L_UINT ti,fX x)
{
	return W = X + rotate_left((W + x(X,Y,Z) + Mj + ti), s);
}

void L_MD5::One512(L_UINT* ABCD,L_UINT* u16_32)
{
	L_UINT ABCDtmp[4];
	memmove(ABCDtmp,ABCD,sizeof(L_UINT) * 4);
	fX x[4]={F,G,H,I};
	for (int i = 0; i < 4; ++i)//FGHI
	{
		for (int j = 0; j < 4; ++j)
		{
#ifndef L_MD5_SAFE
			XX(ABCDtmp[0],ABCDtmp[1],ABCDtmp[2],ABCDtmp[3],u16_32[m_k[i][j * 4 + 0]],m_s[i][0],m_sin_n[i * 16 + j * 4  + 0],x[i]);
			XX(ABCDtmp[3],ABCDtmp[0],ABCDtmp[1],ABCDtmp[2],u16_32[m_k[i][j * 4 + 1]],m_s[i][1],m_sin_n[i * 16 + j * 4  + 1],x[i]);
			XX(ABCDtmp[2],ABCDtmp[3],ABCDtmp[0],ABCDtmp[1],u16_32[m_k[i][j * 4 + 2]],m_s[i][2],m_sin_n[i * 16 + j * 4  + 2],x[i]);
			XX(ABCDtmp[1],ABCDtmp[2],ABCDtmp[3],ABCDtmp[0],u16_32[m_k[i][j * 4 + 3]],m_s[i][3],m_sin_n[i * 16 + j * 4  + 3],x[i]);
#else
			XX(ABCDtmp[0],ABCDtmp[1],ABCDtmp[2],ABCDtmp[3],u16_32[m_k[i][j * 4 + 0]-1],m_s[i][0]+1,m_sin_n[i * 16 + j * 4  + 0]+1,x[i]);
			XX(ABCDtmp[3],ABCDtmp[0],ABCDtmp[1],ABCDtmp[2],u16_32[m_k[i][j * 4 + 1]-1],m_s[i][1]+1,m_sin_n[i * 16 + j * 4  + 1]+1,x[i]);
			XX(ABCDtmp[2],ABCDtmp[3],ABCDtmp[0],ABCDtmp[1],u16_32[m_k[i][j * 4 + 2]-1],m_s[i][2]+1,m_sin_n[i * 16 + j * 4  + 2]+1,x[i]);
			XX(ABCDtmp[1],ABCDtmp[2],ABCDtmp[3],ABCDtmp[0],u16_32[m_k[i][j * 4 + 3]-1],m_s[i][3]+1,m_sin_n[i * 16 + j * 4  + 3]+1,x[i]);
#endif // !L_MD5_SAFE

			

		}
	}
	ABCD[0] += ABCDtmp[0];
	ABCD[1] += ABCDtmp[1];
	ABCD[2] += ABCDtmp[2];
	ABCD[3] += ABCDtmp[3];
}
L_UINT L_MD5::FillMD5(L_UINT* uOut,L_UINT8* uIN,L_UINT uSizeLeft,L_UINT64 size)
{
	L_UINT8 * cOut = (L_UINT8*)uOut;
	memset(uOut, 0, sizeof(L_UINT) * 32);
	memmove(uOut, uIN, uSizeLeft);
	*(((L_UINT8*)uOut) + uSizeLeft) = (L_UINT8)0x80;
	if (uSizeLeft > 14 * 4)
	{
		*((L_UINT64*)(uOut + 14 + 16)) = size * sizeof(L_UINT8) * 8;
		return 64 * 2 / sizeof(L_UINT8);
	}
	else
	{
		*((L_UINT64*)(uOut + 14)) = size * sizeof(L_UINT8) * 8;
		return 64 / sizeof(L_UINT8);
	}
}
void L_MD5::calMD5(char* out, char * in, L_UINT64 size)
{
	L_UINT64 llsize = size;
	
#ifndef L_MD5_SAFE
	L_UINT ABCD[4] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};
#else
	L_UINT ABCD[4];
	volatile L_UINT ABCDv[4];
	ABCDv[0]= 0x67450000;ABCDv[0]+= 0x2301;
	ABCDv[1]= 0xefcd0000;ABCDv[1]+= 0xab89;
	ABCDv[2]= 0x98ba0000;ABCDv[2]+= 0xdcfe;
	ABCDv[3]= 0x10320000;ABCDv[3]+= 0x5476;
	ABCD[0] = ABCDv[0];ABCD[1] = ABCDv[1];ABCD[2] = ABCDv[2];ABCD[3] = ABCDv[3];
#endif
	L_UINT8 * cABCD = (L_UINT8 *)ABCD;
	L_UINT* uIN = (L_UINT*)in;
	while (llsize >= 64 / sizeof(L_UINT8))
	{
		One512(ABCD,uIN);
		uIN += 64 / sizeof(L_UINT);
		llsize -= 64 / sizeof(L_UINT8);
	}
	L_UINT uOut[32]={0};
	llsize = FillMD5(uOut, (L_UINT8*)uIN, (L_UINT)llsize, size);
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
	static inline L_UINT J(L_UINT,L_UINT,L_UINT);
	inline void	One512(L_UINT* ABCDE,L_UINT* u16_32);
	static const L_UINT K[] = {0x5A827999,0x6ED9EBA1,0x8F1BBCDC,0xCA62C1D6};
};

L_UINT L_SHA1::J(L_UINT X,L_UINT Y,L_UINT Z)
{
	return (X&Y)|(X&Z)|(Y&Z);
}

inline void	L_SHA1::One512(L_UINT* ABCDE,L_UINT* u16_32)
{
	L_UINT ABCDEtmp[5];
	memmove(ABCDEtmp,ABCDE,sizeof(L_UINT) * 5);
	L_UINT W[80];
	memmove(W,u16_32,sizeof(L_UINT) * 16);
	for (int i = 16; i < 80; ++i)
	{
		W[i] = L_MD5::rotate_left(1,(W[i-3] ^ W[i-8] ^ W[i-14] ^ W[i-16]));
	}
	fX x[4]={L_MD5::F,L_MD5::H,J,L_MD5::H};
	L_UINT TEMP;
	for (int i = 0; i < 4; ++i)//FGHI
	{
		for (int j = 0; j < 20; ++j)
		{
			TEMP = L_MD5::rotate_left(5,ABCDEtmp[0])
				+ x[i](ABCDEtmp[1],ABCDEtmp[2],ABCDEtmp[3]) + ABCDEtmp[4] + W[i * 20 + j] + K[i];
			ABCDEtmp[4] = ABCDEtmp[3];  ABCDEtmp[3] = ABCDEtmp[2];  ABCDEtmp[2] = L_MD5::rotate_left(30,ABCDEtmp[1]);
			ABCDEtmp[1] = ABCDEtmp[0];  ABCDEtmp[0] = TEMP;
		}
	}


	ABCDE[0] += ABCDEtmp[0];
	ABCDE[1] += ABCDEtmp[1];
	ABCDE[2] += ABCDEtmp[2];
	ABCDE[3] += ABCDEtmp[3];
	ABCDE[4] += ABCDEtmp[4];
}

void L_SHA1::calSHA1(char* out, char * in, unsigned long long size)
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
	llsize = L_MD5::FillMD5(uOut, (L_UINT8*)uIN, (L_UINT)llsize, size);
	uIN = uOut;
	while (llsize >= 64 / sizeof(L_UINT8))
	{
		One512(ABCDE,uIN);
		uIN += 64 / sizeof(L_UINT);
		llsize -= 64 / sizeof(L_UINT8);
	}

	//out = (char*)realloc(out,33);//传进来要保证长度大于32
	for (int i = 0; i < sizeof(L_UINT) * 5 / sizeof(char); ++i)
		sprintf_s (out + i * 2, 3, "%02x", cABCDE[i]);
}

