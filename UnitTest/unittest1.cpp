#include "stdafx.h"
#include "tchar.h"
#include "CppUnitTest.h"
#include "../FileEngine/IFileEngine.h"

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
			RECT testStruct = {1,2,3,4};

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
			{ IFileEngine::VT_STRUCT,TEXT("TD_VALUE_VT_STRUCT")	,LPCVOID(&testStruct),sizeof(RECT) },
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
			TCHAR readString[MAX_PATH] = {0};
			RECT readStuct = {0};
			DllFileEngine::DllCoCreateObject(CLSID_IFileEngine, IID_IFileEngine, (void**)(&ptrFileEngine));
			ptrFileEngine->OpenFile(szFileName);
			ptrFileEngine->GetVarValue(IFileEngine::VT_CHAR, TEXT("TD_VALUE_VT_CHAR	 "), &readChar);
			ptrFileEngine->GetVarValue(IFileEngine::VT_BYTE, TEXT("TD_VALUE_VT_BYTE	 "), &readByte);
			ptrFileEngine->GetVarValue(IFileEngine::VT_BOOL, TEXT("TD_VALUE_VT_BOOL	 "), &readBool);
			ptrFileEngine->GetVarValue(IFileEngine::VT_SHORT, TEXT("TD_VALUE_VT_SHORT "), &readShort);
			ptrFileEngine->GetVarValue(IFileEngine::VT_WORD, TEXT("TD_VALUE_VT_WORD	 "), &readWord);
			ptrFileEngine->GetVarValue(IFileEngine::VT_INT, TEXT("TD_VALUE_VT_INT	 "), &readInt);
			ptrFileEngine->GetVarValue(IFileEngine::VT_LONG, TEXT("TD_VALUE_VT_LONG	 "), &readLong);
			ptrFileEngine->GetVarValue(IFileEngine::VT_FLOAT, TEXT("TD_VALUE_VT_FLOAT "), &readFloat);
			ptrFileEngine->GetVarValue(IFileEngine::VT_DOUBLE, TEXT("TD_VALUE_VT_DOUBLE"), &readDouble);
			ptrFileEngine->GetVarString(TEXT("TD_VALUE_VT_STRING"), readString, MAX_PATH);
			ptrFileEngine->GetVarStruct(TEXT("TD_VALUE_VT_STRUCT"), &testStruct);

			_tprintf(_T("readChar = %c, %d"), readChar, readChar == testChar);
			_tprintf(_T("readByte = %d, %d"), readByte, readByte == testByte);
			_tprintf(_T("readBool = %d, %d"), readBool, readBool == testBool);
			_tprintf(_T("readShort = %d, %d"), readShort, readShort == testShort);
			_tprintf(_T("readWord = %d, %d"), readWord, readWord == testWord);
			_tprintf(_T("readInt = %d, %d"), readInt, readInt == testInt);
			_tprintf(_T("readLong = %d, %d"), readLong, readLong == testLong);
			_tprintf(_T("readFloat = %f, %d"), readFloat, readFloat == testFloat);
			_tprintf(_T("readDouble = %lf, %d"), readDouble, readDouble == testDouble);
			_tprintf(_T("readString = %s, %d"), readString, lstrcmp(readString,testString));
			//_tprintf(_T("testStruct = %ld, %d"), readChar, readChar == testChar);

		}

	};
}