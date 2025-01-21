#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <minizip/zip.h>
#include <minizip/unzip.h>

int get_windows_architecture()
{
    if ((size_t)-1 > 0xffffffffUL)
    {
        printf("64 bits\n");
        return 64;
    }
    else
    {
        printf("32 bits\n");
        return 32;
    }
}

int extract_zip_file()
{
    const char *zip_filename = "target.zip";
    const char *output_dir = "C:/sl/";

    // Create the output directory if it does not exist
    if (CreateDirectory(output_dir, NULL) == 0 && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        printf("Failed to create directory: %s\n", output_dir);
        return 1;
    }

    // Open the zip file
    unzFile zip = unzOpen(zip_filename);
    if (zip == NULL)
    {
        printf("Failed to open zip file\n");
        return 1;
    }

    // Get the number of files in the zip
    if (unzGoToFirstFile(zip) != UNZ_OK)
    {
        printf("Failed to locate the first file in zip\n");
        unzClose(zip);
        return 1;
    }

    do
    {
        char filename[256];
        unz_file_info file_info;

        // Get the file info
        if (unzGetCurrentFileInfo(zip, &file_info, filename, sizeof(filename), NULL, 0, NULL, 0) != UNZ_OK)
        {
            printf("Failed to get file info\n");
            break;
        }

        // Construct the full path for extraction
        char file_path[MAX_PATH];
        snprintf(file_path, sizeof(file_path), "%s%s", output_dir, filename);

        // Open the output file
        FILE *output_file = fopen(file_path, "wb");
        if (output_file == NULL)
        {
            printf("Failed to open file for writing: %s\n", file_path);
            break;
        }

        // Open the file from the zip
        if (unzOpenCurrentFile(zip) != UNZ_OK)
        {
            printf("Failed to open file from zip: %s\n", filename);
            fclose(output_file);
            break;
        }

        // Buffer for reading from the zip
        char buffer[8192];
        int bytes_read;
        while ((bytes_read = unzReadCurrentFile(zip, buffer, sizeof(buffer))) > 0)
        {
            fwrite(buffer, 1, bytes_read, output_file);
        }

        fclose(output_file);
        unzCloseCurrentFile(zip);

    } while (unzGoToNextFile(zip) == UNZ_OK);

    unzClose(zip);
    printf("Extraction complete.\n");
    return EXIT_SUCCESS;
}

int main()
{
    return 0;
}
