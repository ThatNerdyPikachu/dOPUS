#include "xciHelper.h"
#include "nspHelper.h"
#include "filehelper.h"

extern "C" {
#include <getopt.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cnmt.h"
#include "nsp.h"
#include "types.h"
#include "utils.h"
#include "settings.h"
#include "pki.h"
#include "xci.h"
#include "extkeys.h"
#include "version.h"
}

/* 4NXCI by The-4n
   Based on hactool by SciresM
   */


cnmt_xml_ctx_t application_cnmt_xml;
cnmt_xml_ctx_t patch_cnmt_xml;
cnmt_ctx_t application_cnmt;
cnmt_ctx_t patch_cnmt;
nsp_ctx_t application_nsp;
nsp_ctx_t patch_nsp;
nsp_ctx_t *addon_nsps;
cnmt_addons_ctx_t addons_cnmt_ctx;


namespace NXFramework
{

// Rename NCAs to NSP NCAids
void RenameNCAs(const xci_ctx_t& xci_ctx, const nsp_ctx_t& nsp_ctx)
{
    for (int i = 0; i < nsp_ctx.entry_count; i++)
    {
        filepath_t filepath;
        filepath_init(&filepath);
        filepath_copy(&filepath, &xci_ctx.secure_ctx.tool_ctx->settings.secure_dir_path);
        filepath_append(&filepath, "%s", nsp_ctx.nsp_entry[i].nsp_filename);

        // Rename nca file to nsp logical filename
        printf("Renaming %s to %s...\n", nsp_ctx.nsp_entry[i].filepath.char_path, filepath.char_path);
        if(rename(nsp_ctx.nsp_entry[i].filepath.char_path, filepath.char_path) != 0)
            printf("Error renaming file\n");
    }
}

void RenameNCAs(const xci_ctx_t& xci_ctx)
{
    RenameNCAs(xci_ctx, application_nsp);

    if (patch_cnmt.title_id != 0)
        RenameNCAs(xci_ctx, patch_nsp);

    if (addons_cnmt_ctx.count != 0)
        for (int i = 0; i < addons_cnmt_ctx.count; ++i)
            RenameNCAs(xci_ctx, addon_nsps[i]);
}

int ExtractXCI(const std::string& filename, const bool saveNSP, float* progress)
{
    nxci_ctx_t tool_ctx;
    char input_name[0x200];

    printf("4NXCI %s by The-4n\n", NXCI_VERSION);

    memset(&tool_ctx, 0, sizeof(tool_ctx));
    memset(input_name, 0, sizeof(input_name));
    memset(&application_cnmt, 0, sizeof(cnmt_ctx_t));
    memset(&patch_cnmt, 0, sizeof(cnmt_ctx_t));
    memset(&application_cnmt_xml, 0, sizeof(cnmt_xml_ctx_t));
    memset(&patch_cnmt_xml, 0, sizeof(cnmt_xml_ctx_t));
    memset(&application_nsp, 0, sizeof(nsp_ctx_t));
    memset(&patch_nsp, 0, sizeof(nsp_ctx_t));
    memset(&addons_cnmt_ctx, 0, sizeof(cnmt_addons_ctx_t));
    memset(&addon_nsps, 0, sizeof(addon_nsps));

    filepath_t keypath;

    filepath_init(&keypath);
    pki_initialize_keyset(&tool_ctx.settings.keyset, KEYSET_RETAIL);

	// Default keyset filepath
    filepath_set(&keypath, "/keys.dat");

    // Try to populate default keyfile.
    FILE *keyfile = NULL;
    keyfile = os_fopen(keypath.os_path, OS_MODE_READ);

    if (keyfile != NULL)
    {
        extkeys_initialize_keyset(&tool_ctx.settings.keyset, keyfile);
        pki_derive_keys(&tool_ctx.settings.keyset);
        fclose(keyfile);
    }
    else
    {
        fprintf(stderr, "Unable to open keyset '%s'\n", keypath.char_path);
		return 1;
    }

    // Copy input file
    strcpy(input_name, filename.c_str());
    if (!(tool_ctx.file = fopen(input_name, "rb")))
    {
        fprintf(stderr, "unable to open %s: %s\n", input_name, strerror(errno));
		return 1;
    }

    xci_ctx_t xci_ctx;
    memset(&xci_ctx, 0, sizeof(xci_ctx));
    xci_ctx.file = tool_ctx.file;
    xci_ctx.tool_ctx = &tool_ctx;

    // secure partition save path to "filename" directory
    char outputDir[MAX_PATH];
    GetFileBasename(outputDir, filename.c_str());
    if(saveNSP)
    {
        strcat(outputDir, "Temp");
    }
    filepath_init(&xci_ctx.tool_ctx->settings.secure_dir_path);
    filepath_set(&xci_ctx.tool_ctx->settings.secure_dir_path, outputDir);

    // Clean-up dir if it exists
    RmDirRecursive(outputDir);

    printf("\n");

    xci_process(&xci_ctx, progress);

    // Output NSP filename
    char nspFilename[MAX_PATH];
    GetFileBasename(nspFilename, filename.c_str());

    // Process ncas in cnmts
    printf("===> Processing Application Metadata:\n");
    cnmt_gamecard_process(xci_ctx.tool_ctx, &application_cnmt_xml, &application_cnmt, &application_nsp, saveNSP, nspFilename, progress);
    if (patch_cnmt.title_id != 0)
    {
        printf("===> Processing Patch Metadata:\n");
        std::string nspUPDFilename(std::string(nspFilename) + std::string(" [UPD]"));
        cnmt_download_process(xci_ctx.tool_ctx, &patch_cnmt_xml, &patch_cnmt, &patch_nsp, saveNSP, nspUPDFilename.c_str(), progress);
    }
    if (addons_cnmt_ctx.count != 0)
    {
        addon_nsps = (nsp_ctx_t *)calloc(1, sizeof(nsp_ctx_t) * addons_cnmt_ctx.count);
        printf("===> Processing %u Addon(s):\n", addons_cnmt_ctx.count);
        std::string nspDLCFilename(std::string(nspFilename) + std::string(" [DLC]"));
        for (int i = 0; i < addons_cnmt_ctx.count; i++)
        {
            printf("===> Processing AddOn %i Metadata:\n", i + 1);
            cnmt_gamecard_process(xci_ctx.tool_ctx, &addons_cnmt_ctx.addon_cnmt_xml[i], &addons_cnmt_ctx.addon_cnmt[i], &addon_nsps[i], saveNSP, nspDLCFilename.c_str(), progress);
        }
    }

    if(saveNSP)
    {
        printf("\nSummary:\n");
        printf("Game NSP: %s\n", application_nsp.filepath.char_path);
        if (patch_cnmt.title_id != 0)
            printf("Update NSP: %s\n", patch_nsp.filepath.char_path);
        if (addons_cnmt_ctx.count != 0)
        {
            for (int i2 = 0; i2 < addons_cnmt_ctx.count; i2++)
                printf("DLC NSP %i: %s\n", i2 + 1, addon_nsps[i2].filepath.char_path);
        }
    }
    fclose(tool_ctx.file);

    printf("\nClean-up:\n");

    if(saveNSP)
    {
        // Clean-up
        RmDirRecursive(xci_ctx.tool_ctx->settings.secure_dir_path.char_path);
    }
    else
    {
        // Rename NCAs using NCAids in XML
        RenameNCAs(xci_ctx);
    }
	printf("\n");
    printf("\nDone!\n");
    return 0;
}

int ConvertXCI(const std::string& filename, float* progress)
{
    return ExtractXCI(filename, true, progress);
}

int InstallXCI(const std::string& filename, const FsStorageId destStorageId, const bool ignoreReqFirmVersion, const bool deleteXCI, float* progress)
{
    // Extract
    ExtractXCI(filename, false, progress);

    // Delete source file if wanted, to avoid using 3 times the space necessary:
    // XCI + Extracted NCAs + Installed
    if(deleteXCI)
    {
        LOG("Deleting file %s...\n", filename.c_str());
        if(remove(filename.c_str()) != 0)
            printf("Error deleting file\n");
    }

    // Install
    char outputDir[MAX_PATH];
    GetFileBasename(outputDir, filename.c_str());
    LOG("\nInstalling folder %s...\n", outputDir);
    InstallExtracted(std::string(outputDir), destStorageId, ignoreReqFirmVersion, progress);

    // Clean-up
    RmDirRecursive(outputDir);
    return 0;
}

}
