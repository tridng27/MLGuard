#include <stdio.h>
#include <windows.h>
#include <shlobj.h>
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Ole32.lib")
#include <strsafe.h>
#include <math.h>

int firstItem = 1; // để quản lý dấu phẩy trong JSON

// =======================
// Tính entropy Shannon
// =======================
double calculateEntropy(unsigned char *data, size_t size) {
    if (size == 0) return 0.0;

    unsigned int freq[256] = {0};
    for (size_t i = 0; i < size; i++) {
        freq[data[i]]++;
    }

    double entropy = 0.0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] == 0) continue;
        double p = (double)freq[i] / size;
        entropy -= p * log2(p);
    }
    return entropy;
}

// =======================
// Parse 1 PE file
// =======================
void parsePE(const char *filepath) {
    FILE *fp = fopen(filepath, "rb");
    if (!fp) return;

    IMAGE_DOS_HEADER dosHeader;
    if (fread(&dosHeader, sizeof(dosHeader), 1, fp) != 1) { fclose(fp); return; }
    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) { fclose(fp); return; }

    fseek(fp, dosHeader.e_lfanew, SEEK_SET);
    DWORD peSignature;
    if (fread(&peSignature, sizeof(peSignature), 1, fp) != 1) { fclose(fp); return; }
    if (peSignature != IMAGE_NT_SIGNATURE) { fclose(fp); return; }

    IMAGE_FILE_HEADER fileHeader;
    if (fread(&fileHeader, sizeof(fileHeader), 1, fp) != 1) { fclose(fp); return; }

    WORD magic;
    if (fread(&magic, sizeof(magic), 1, fp) != 1) { fclose(fp); return; }
    fseek(fp, -sizeof(magic), SEEK_CUR);

    DWORD entryPoint = 0;
    unsigned long long imageBase = 0;
    WORD subsystem = 0;
    WORD numberOfSections = fileHeader.NumberOfSections;
    size_t sectionOffset = ftell(fp);

    if (magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        IMAGE_OPTIONAL_HEADER32 opt32;
        if (fread(&opt32, sizeof(opt32), 1, fp) != 1) { fclose(fp); return; }
        entryPoint = opt32.AddressOfEntryPoint;
        imageBase = opt32.ImageBase;
        subsystem = opt32.Subsystem;
        sectionOffset += sizeof(IMAGE_OPTIONAL_HEADER32);
    } else if (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        IMAGE_OPTIONAL_HEADER64 opt64;
        if (fread(&opt64, sizeof(opt64), 1, fp) != 1) { fclose(fp); return; }
        entryPoint = opt64.AddressOfEntryPoint;
        imageBase = opt64.ImageBase;
        subsystem = opt64.Subsystem;
        sectionOffset += sizeof(IMAGE_OPTIONAL_HEADER64);
    } else {
        fclose(fp);
        return;
    }

    // JSON output
    if (!firstItem) {
        printf(",\n");
    } else {
        firstItem = 0;
    }

    printf("  {\n");
    printf("    \"FilePath\": \"%s\",\n", filepath);
    printf("    \"Machine\": \"0x%x\",\n", fileHeader.Machine);
    printf("    \"Sections\": %d,\n", numberOfSections);
    printf("    \"EntryPoint\": \"0x%x\",\n", entryPoint);
    printf("    \"ImageBase\": \"0x%llx\",\n", imageBase);
    printf("    \"Subsystem\": %d,\n", subsystem);

    // Parse sections
    fseek(fp, sectionOffset, SEEK_SET);
    printf("    \"SectionTable\": [\n");

    for (int i = 0; i < numberOfSections; i++) {
        IMAGE_SECTION_HEADER sh;
        if (fread(&sh, sizeof(sh), 1, fp) != 1) break;

        // Read section data for entropy
        double entropy = 0.0;
        if (sh.SizeOfRawData > 0) {
            unsigned char *buffer = (unsigned char*)malloc(sh.SizeOfRawData);
            if (buffer) {
                long curPos = ftell(fp);
                fseek(fp, sh.PointerToRawData, SEEK_SET);
                fread(buffer, 1, sh.SizeOfRawData, fp);
                entropy = calculateEntropy(buffer, sh.SizeOfRawData);
                free(buffer);
                fseek(fp, curPos, SEEK_SET);
            }
        }

        // Print JSON for section
        printf("      {\n");
        printf("        \"name\": \"%.8s\",\n", sh.Name);
        printf("        \"virtual_size\": %u,\n", sh.Misc.VirtualSize);
        printf("        \"virtual_address\": \"0x%x\",\n", sh.VirtualAddress);
        printf("        \"raw_size\": %u,\n", sh.SizeOfRawData);
        printf("        \"raw_pointer\": \"0x%x\",\n", sh.PointerToRawData);
        printf("        \"entropy\": %.4f\n", entropy);
        printf("      }%s\n", (i == numberOfSections - 1) ? "" : ",");
    }

    printf("    ]\n");
    printf("  }");

    fclose(fp);
}

// =======================
// Scan given folder for .exe
// =======================
void scanFolder(const char *folderPath) {
    char searchPath[MAX_PATH];
    _snprintf_s(searchPath, MAX_PATH, _TRUNCATE, "%s\\*.exe", folderPath);

    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(searchPath, &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            char fullPath[MAX_PATH];
            _snprintf_s(fullPath, MAX_PATH, _TRUNCATE, "%s\\%s", folderPath, findData.cFileName);
            parsePE(fullPath);
        }
    } while (FindNextFile(hFind, &findData));

    FindClose(hFind);
}

// =======================
// Get Downloads path
// =======================
void getDownloadsPath(char *outPath, size_t size) {
    PWSTR downloadsPath = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_Downloads, 0, NULL, &downloadsPath))) {
        wcstombs(outPath, downloadsPath, size);
        CoTaskMemFree(downloadsPath);
    } else {
        strcpy_s(outPath, size, ".");
    }
}

// =======================
// Main
// =======================
int main(int argc, char *argv[]) {
    printf("[\n");

    if (argc > 1) {
        scanFolder(argv[1]);
    } else {
        char downloads[MAX_PATH];
        getDownloadsPath(downloads, MAX_PATH);
        scanFolder(downloads);
    }

    printf("\n]\n");
    return 0;
}
