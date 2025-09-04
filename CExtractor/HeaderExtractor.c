#include <stdio.h>
#include <windows.h>
#include <string.h>

// =======================
// Function: Parse 1 PE file
// =======================
void parsePE(const char *filepath) {
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        printf("Error opening file: %s\n", filepath);
        return;
    }

    IMAGE_DOS_HEADER dosHeader;
    fread(&dosHeader, sizeof(IMAGE_DOS_HEADER), 1, fp);

    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) { // "MZ"
        fclose(fp);
        return; // Not a valid PE
    }

    fseek(fp, dosHeader.e_lfanew, SEEK_SET);

    DWORD peSignature;
    fread(&peSignature, sizeof(DWORD), 1, fp);
    if (peSignature != IMAGE_NT_SIGNATURE) { // "PE\0\0"
        fclose(fp);
        return;
    }

    IMAGE_FILE_HEADER fileHeader;
    fread(&fileHeader, sizeof(IMAGE_FILE_HEADER), 1, fp);

    IMAGE_OPTIONAL_HEADER optHeader;
    fread(&optHeader, sizeof(IMAGE_OPTIONAL_HEADER), 1, fp);

    printf("\n--- %s ---\n", filepath);
    printf("Machine: 0x%x\n", fileHeader.Machine);
    printf("Number of Sections: %d\n", fileHeader.NumberOfSections);
    printf("TimeDateStamp: 0x%x\n", fileHeader.TimeDateStamp);
    printf("Entry Point: 0x%x\n", optHeader.AddressOfEntryPoint);
    printf("Image Base: 0x%llx\n", (unsigned long long)optHeader.ImageBase);
    printf("Subsystem: %d\n", optHeader.Subsystem);

    fclose(fp);
}

// =======================
// Function: Scan Downloads folder for .exe
// =======================
void scanDownloads() {
    char searchPath[MAX_PATH];
    ExpandEnvironmentStrings("%USERPROFILE%\\Downloads\\*.exe", searchPath, MAX_PATH);

    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(searchPath, &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("No .exe files found in Downloads.\n");
        return;
    }

    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            char fullPath[MAX_PATH];
            ExpandEnvironmentStrings("%USERPROFILE%\\Downloads", fullPath, MAX_PATH);

            strcat(fullPath, "\\");
            strcat(fullPath, findData.cFileName);

            parsePE(fullPath);
        }
    } while (FindNextFile(hFind, &findData));

    FindClose(hFind);
}

// =======================
// Main
// =======================
int main(int argc, char *argv[]) {
    if (argc > 1) {
        // If user passes a file, parse just that
        parsePE(argv[1]);
    } else {
        // Otherwise, scan Downloads
        printf("Scanning Downloads folder for .exe files...\n");
        scanDownloads();
    }
    return 0;
}
