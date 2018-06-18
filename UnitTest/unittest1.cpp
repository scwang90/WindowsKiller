#include "stdafx.h"
#include "tchar.h"
#include "CppUnitTest.h"
#include "../FileEngine/IFileEngine.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// {F3EBADDA-2200-4d05-9D7F-8B7A14714D76}
static const GUID CLSID_IFileEngine =
{ 0xf3ebadda, 0x2200, 0x4d05,{ 0x9d, 0x7f, 0x8b, 0x7a, 0x14, 0x71, 0x4d, 0x76 } };

// {8B474B04-4940-46c7-9102-8312511EA11B}
static const GUID IID_IFileEngine =
{ 0x8b474b04, 0x4940, 0x46c7,{ 0x91, 0x2, 0x83, 0x12, 0x51, 0x1e, 0xa1, 0x1b } };


namespace UnitTest
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			// TODO: 在此输入测试代码
			char testChar = TEXT('C');
			BYTE testByte = 55;
			bool testBool = true;
			short testShort = 256;
			WORD testWord = 260;
			int testInt = 2356;
			long testLong = 456789;
			float testFloat = 9.9f;
			double testDouble = 99.99;
			TCHAR testString[] = TEXT("Test String");
			TCHAR testString2[] = TEXT("Test String 2");
			RECT testStruct = { 1,2,3,4 };
			RECT testStruct2 = { 4,3,2,1 };

			TYPEDATA cTypeData[] = {
			{ IFileEngine::VT_CHAR	,TEXT("TD_VALUE_VT_CHAR	 ")	,LPCVOID(testChar),TD_VALUE },
			{ IFileEngine::VT_BYTE	,TEXT("TD_VALUE_VT_BYTE	 ")	,LPCVOID(testByte),TD_VALUE },
			{ IFileEngine::VT_BOOL	,TEXT("TD_VALUE_VT_BOOL	 ")	,LPCVOID(testBool),TD_VALUE },
			{ IFileEngine::VT_SHORT ,TEXT("TD_VALUE_VT_SHORT ")	,LPCVOID(testShort),TD_VALUE },
			{ IFileEngine::VT_WORD	,TEXT("TD_VALUE_VT_WORD	 ")	,LPCVOID(testWord),TD_VALUE },
			{ IFileEngine::VT_INT	,TEXT("TD_VALUE_VT_INT	 ")	,LPCVOID(testInt),TD_VALUE },
			{ IFileEngine::VT_LONG	,TEXT("TD_VALUE_VT_LONG	 ")	,LPCVOID(testLong),TD_VALUE },
			{ IFileEngine::VT_FLOAT ,TEXT("TD_VALUE_VT_FLOAT ")	,LPCVOID(&testFloat),TD_ADDRESS },
			{ IFileEngine::VT_DOUBLE,TEXT("TD_VALUE_VT_DOUBLE")	,LPCVOID(&testDouble),TD_ADDRESS },
			{ IFileEngine::VT_STRING,TEXT("TD_VALUE_VT_STRING")	,LPCVOID(testString), lstrlen(testString) + 1 },
			{ IFileEngine::VT_STRING,TEXT("TD_VALUE_VT_STRING2")	,LPCVOID(testString2), lstrlen(testString2) + 1 },
			{ IFileEngine::VT_STRUCT,TEXT("TD_VALUE_VT_STRUCT")	,LPCVOID(&testStruct),sizeof(RECT) },
			{ IFileEngine::VT_STRUCT,TEXT("TD_VALUE_VT_STRUCT2")	,LPCVOID(&testStruct2),sizeof(RECT) },
			};

			TCHAR szFileName[] = TEXT("UnitTestFile.data");
			DeleteFile(szFileName);
			IFileEngine *ptrFileEngine = nullptr;
			DllFileEngine::DllCoCreateObject(CLSID_IFileEngine, IID_IFileEngine, (void**)(&ptrFileEngine));
			ptrFileEngine->OpenFile(szFileName);
			ptrFileEngine->AddVarValue(cTypeData, sizeof(cTypeData)/sizeof(TYPEDATA));
			ptrFileEngine->OutPutDataInfo();
			//ptrFileEngine->SaveFile();
			ptrFileEngine->CloseFile();
			ptrFileEngine->Release();

			char readChar = TEXT('\0');
			BYTE readByte = 0;
			bool readBool = false;
			short readShort = 0;
			WORD readWord = 0;
			int readInt = 0;
			long readLong = 0;
			float readFloat = 0;
			double readDouble = 0;
			TCHAR readString[MAX_PATH] = { 0 };
			TCHAR readString2[MAX_PATH] = { 0 };
			RECT readStruct = { 0 };
			RECT readStruct2 = { 0 };
			DllFileEngine::DllCoCreateObject(CLSID_IFileEngine, IID_IFileEngine, (void**)(&ptrFileEngine));
			ptrFileEngine->OpenFile(szFileName);

			TCHAR szLog[MAX_PATH] = TEXT("");
			TCHAR szTemp[MAX_PATH] = TEXT("");
			wsprintf(szTemp, TEXT("\r\nVT_CHAR - %d"), ptrFileEngine->GetVarValue(IFileEngine::VT_CHAR, TEXT("TD_VALUE_VT_CHAR	 "), &readChar));
			lstrcat(szLog, szTemp);
			wsprintf(szTemp, TEXT("\r\nVT_BYTE - %d"), ptrFileEngine->GetVarValue(IFileEngine::VT_BYTE, TEXT("TD_VALUE_VT_BYTE	 "), &readByte));
			lstrcat(szLog, szTemp);
			wsprintf(szTemp, TEXT("\r\nVT_BOOL - %d"), ptrFileEngine->GetVarValue(IFileEngine::VT_BOOL, TEXT("TD_VALUE_VT_BOOL	 "), &readBool));
			lstrcat(szLog, szTemp);
			wsprintf(szTemp, TEXT("\r\nVT_SHORT - %d"), ptrFileEngine->GetVarValue(IFileEngine::VT_SHORT, TEXT("TD_VALUE_VT_SHORT "), &readShort));
			lstrcat(szLog, szTemp);
			wsprintf(szTemp, TEXT("\r\nVT_WORD - %d"), ptrFileEngine->GetVarValue(IFileEngine::VT_WORD, TEXT("TD_VALUE_VT_WORD	 "), &readWord));
			lstrcat(szLog, szTemp);
			wsprintf(szTemp, TEXT("\r\nVT_INT - %d"), ptrFileEngine->GetVarValue(IFileEngine::VT_INT, TEXT("TD_VALUE_VT_INT	 "), &readInt));
			lstrcat(szLog, szTemp);
			wsprintf(szTemp, TEXT("\r\nVT_LONG - %d"), ptrFileEngine->GetVarValue(IFileEngine::VT_LONG, TEXT("TD_VALUE_VT_LONG	 "), &readLong));
			lstrcat(szLog, szTemp);
			wsprintf(szTemp, TEXT("\r\nVT_FLOAT - %d"), ptrFileEngine->GetVarValue(IFileEngine::VT_FLOAT, TEXT("TD_VALUE_VT_FLOAT "), &readFloat));
			lstrcat(szLog, szTemp);
			wsprintf(szTemp, TEXT("\r\nVT_DOUBLE - %d"), ptrFileEngine->GetVarValue(IFileEngine::VT_DOUBLE, TEXT("TD_VALUE_VT_DOUBLE"), &readDouble));
			lstrcat(szLog, szTemp);
			wsprintf(szTemp, TEXT("\r\nTD_VALUE_VT_STRING - %d"), ptrFileEngine->GetVarString(TEXT("TD_VALUE_VT_STRING"), readString, MAX_PATH));
			lstrcat(szLog, szTemp);
			wsprintf(szTemp, TEXT("\r\nTD_VALUE_VT_STRING2 - %d"), ptrFileEngine->GetVarString(TEXT("TD_VALUE_VT_STRING2"), readString2, MAX_PATH));
			lstrcat(szLog, szTemp);
			wsprintf(szTemp, TEXT("\r\nTD_VALUE_VT_STRUCT - %d"), ptrFileEngine->GetVarStruct(TEXT("TD_VALUE_VT_STRUCT"), &readStruct));
			lstrcat(szLog, szTemp);
			wsprintf(szTemp, TEXT("\r\nTD_VALUE_VT_STRUCT1 - %d"), ptrFileEngine->GetVarStruct(TEXT("TD_VALUE_VT_STRUCT2"), &readStruct2));
			lstrcat(szLog, szTemp);

			wsprintf(szTemp,_T("readChar = %c, %d\n"), readChar, readChar == testChar);
			wsprintf(szTemp,_T("readByte = %d, %d\n"), readByte, readByte == testByte);
			wsprintf(szTemp,_T("readBool = %d, %d\n"), readBool, readBool == testBool);
			wsprintf(szTemp,_T("readShort = %d, %d\n"), readShort, readShort == testShort);
			wsprintf(szTemp,_T("readWord = %d, %d\n"), readWord, readWord == testWord);
			wsprintf(szTemp,_T("readInt = %d, %d\n"), readInt, readInt == testInt);
			wsprintf(szTemp,_T("readLong = %d, %d\n"), readLong, readLong == testLong);
			wsprintf(szTemp,_T("readFloat = %f, %d\n"), readFloat, readFloat == testFloat);
			wsprintf(szTemp, _T("readDouble = %lf, %d\n"), readDouble, readDouble == testDouble);
			wsprintf(szTemp, _T("readString = %s, %d\n"), readString, lstrcmp(readString, testString));
			wsprintf(szTemp, _T("readString2 = %s, %d\n"), readString2, lstrcmp(readString2, testString2));
			//wsprintf(szTemp, _T("testStruct = %ld, %d"), readChar, readChar == testChar);
			Assert::IsTrue( readChar == testChar);
			Assert::IsTrue( readByte == testByte);
			Assert::IsTrue( readBool == testBool);
			Assert::IsTrue( readShort == testShort);
			Assert::IsTrue( readWord == testWord);
			Assert::IsTrue( readInt == testInt);
			Assert::IsTrue( readLong == testLong);
			Assert::IsTrue( readFloat == testFloat);
			Assert::IsTrue( readDouble == testDouble);
			Assert::IsTrue(lstrcmp(readString, testString));
			Assert::IsTrue(lstrcmp(readString2, testString2));
			//Assert::IsTrue(readStruct == testStruct);

			

		}

	};
}