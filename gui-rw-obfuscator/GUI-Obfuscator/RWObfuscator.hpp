#pragma once
#include "BinaryIOHelpers.h"

int PARTS_NUM, VARIANT_KEY;
#define XR_SWAP(x,y) temp = *x; *x = *y; *y = temp

void runBuild(const wchar_t* input, const wchar_t* output);
void startMission(int& lPARTS_NUM, int& lVARIANT_KEY)
{
	PARTS_NUM = lPARTS_NUM;
	VARIANT_KEY = lVARIANT_KEY;

	wchar_t dir1[100];
	GetWindowTextW(OriginIMGEdit, dir1, sizeof(dir1));
	swprintf(dir1, sizeof(dir1) / sizeof(wchar_t), L"%s", dir1);
	wchar_t dir2[100];
	GetWindowTextW(EncryptedIMGEdit, dir2, sizeof(dir2));
	swprintf(dir2, sizeof(dir2) / sizeof(wchar_t), L"%s", dir2);
	runBuild(dir1, dir2);
}

long double fact(int N)
{
	if (N < 0)
		return 0;
	if (N == 0)
		return 1;
	else
		return N * fact(N - 1);
}

void permute(unsigned* a, unsigned** lookup, int l, int r)
{
	int i;
	if (l == r)
	{
		memcpy(*lookup, a, sizeof(unsigned) * PARTS_NUM);
		(*lookup) += PARTS_NUM;
	}
	else
	{
		char temp;
		for (i = l; i <= r; i++)
		{
			XR_SWAP((a + l), (a + i));
			permute(a, lookup, l + 1, r);
			XR_SWAP((a + l), (a + i));
		}
	}
}

void toClipboard(HWND hwnd, const std::string& s) {
	OpenClipboard(hwnd);
	EmptyClipboard();
	HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size() + 1);
	if (!hg) {
		CloseClipboard();
		return;
	}
	memcpy(GlobalLock(hg), s.c_str(), s.size() + 1);
	GlobalUnlock(hg);
	SetClipboardData(CF_TEXT, hg);
	CloseClipboard();
	GlobalFree(hg);
}

void runBuild(const wchar_t* input, const wchar_t* output)
{
	ifstream in(input, ios::binary | ios::ate);
	if (in.fail()) {
		wchar_t errorMessage[50];
		swprintf(errorMessage, sizeof(errorMessage) / sizeof(wchar_t), L"%s", input);
		MessageBoxW(MainWindow, errorMessage, L"Opening error", MB_OK | MB_ICONERROR);
	}

	fpos_t fileSize = in.tellg();
	in.seekg(0, ios_base::beg);

	unsigned partSize = floor(fileSize / PARTS_NUM);
	unsigned totalPartsSize = partSize * PARTS_NUM;
	unsigned tailSize = fileSize - totalPartsSize;

	// Calculate a random permutation key
	unsigned totalVariants = fact(PARTS_NUM);
	srand((unsigned)time(NULL) + fileSize);
	unsigned variant = rand() % totalVariants;

	// Build an auxiliary array
	unsigned* perm = new unsigned[PARTS_NUM];
	for (unsigned i = 0; i < PARTS_NUM; i++)
	{
		perm[i] = i;
	}
	unsigned* permLookup = new unsigned[totalVariants * PARTS_NUM];
	unsigned* tempPtr = permLookup;

	permute(perm, &tempPtr, 0, PARTS_NUM - 1);

	/*
		Build
	*/
	ofstream out(output, ios::binary);
	if (out.fail()) {
		wchar_t errorMessage[50];
		swprintf(errorMessage, sizeof(errorMessage) / sizeof(wchar_t), L"%s", input);
		MessageBoxW(MainWindow, errorMessage, L"cannot open", MB_OK | MB_ICONERROR);
	}

	writeUInt32(0x18D5E82, out); // Our flag for the deobfuscator
	writeUInt32(fileSize, out); // File size
	writeUInt32(variant * VARIANT_KEY, out); // Permutation key

	char* buf = (char*)malloc(partSize);
	unsigned* permPtr = permLookup + variant * PARTS_NUM;
	for (unsigned i = 0; i < PARTS_NUM; i++)
	{
		// Move to a chunk in an default asset
		unsigned chunkIndex = permPtr[i];
		in.seekg(chunkIndex * partSize, ios_base::beg);

		in.read(buf, partSize);
		out.write(buf, partSize);
	}

	// Write a tail
	in.seekg(totalPartsSize, ios_base::beg);
	in.read(buf, tailSize);
	out.write(buf, tailSize);

	delete buf;
	delete[] perm;
    delete[] permLookup;
	out.close();
	in.close();

	std::string variantStr = std::to_string(variant);
	const char* variantCStr = variantStr.c_str();
	toClipboard(MainWindow, variantCStr);

	wstring SuccessfulMessage = L" Variant copied to the clipboard [" + to_wstring(variant) + L"]";
	MessageBoxW(MainWindow, SuccessfulMessage.c_str(), L"The Operation Is Successful", MB_OK | MB_ICONINFORMATION);



}