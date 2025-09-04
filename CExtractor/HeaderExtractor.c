#include <stdio.h>
#include <windows.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <exe/dll file>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("Error opening file");
        return 1;
    }

    // Read DOS Header
    IMAGE_DOS_HEADER dosHeader;
    fread(&dosHeader, sizeof(IMAGE_DOS_HEADER), 1, fp);

    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) { // "MZ"
        printf("Not a valid PE file (missing MZ)\n");
        fclose(fp);
        return 1;
    }

    // Move to PE header
    fseek(fp, dosHeader.e_lfanew, SEEK_SET);

    DWORD peSignature;
    fread(&peSignature, sizeof(DWORD), 1, fp);
    if (peSignature != IMAGE_NT_SIGNATURE) { // "PE\0\0"
        printf("Invalid PE signature\n");
        fclose(fp);
        return 1;
    }

    // Read File Header
    IMAGE_FILE_HEADER fileHeader;
    fread(&fileHeader, sizeof(IMAGE_FILE_HEADER), 1, fp);

    printf("Machine: 0x%x\n", fileHeader.Machine);
    printf("Number of Sections: %d\n", fileHeader.NumberOfSections);
    printf("TimeDateStamp: 0x%x\n", fileHeader.TimeDateStamp);

    // Read Optional Header
    IMAGE_OPTIONAL_HEADER optHeader;
    fread(&optHeader, sizeof(IMAGE_OPTIONAL_HEADER), 1, fp);

    printf("Entry Point: 0x%x\n", optHeader.AddressOfEntryPoint);
    printf("Image Base: 0x%llx\n", (unsigned long long)optHeader.ImageBase);
    printf("Subsystem: %d\n", optHeader.Subsystem);

    fclose(fp);
    return 0;
}
