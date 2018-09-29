#include "xciHelper.h"
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


void CleanUp(const xci_ctx_t& xci_ctx)
{
    // Clean-up temp files //
    printf("Deleting temp files...\n");
    for (uint32_t i = 0; i < xci_ctx.secure_ctx.header->num_files; i++)
    {
        filepath_t filepath;
        filepath_init(&filepath);
        filepath_copy(&filepath, &xci_ctx.secure_ctx.tool_ctx->settings.secure_dir_path);
        filepath_append(&filepath, "%s", hfs0_get_file_name(xci_ctx.secure_ctx.header, i));
        printf("Deleting %s...\n", filepath.os_path);
        if(remove(filepath.os_path) != 0)
            printf("Error deleting file\n");
    }
    // Clean-up XML files //
    {
        // Application
        printf("Deleting %s...\n", application_cnmt_xml.filepath.os_path);
        if(remove(application_cnmt_xml.filepath.char_path) != 0)
            printf("Error deleting file\n");

        // Patches
        if (patch_cnmt.title_id != 0)
        {
            printf("Deleting %s...\n", patch_cnmt_xml.filepath.os_path);
            if(remove(patch_cnmt_xml.filepath.char_path) != 0)
                printf("Error deleting file\n");
        }

        // Addons
        if (addons_cnmt_ctx.count != 0)
        {
            for (int i = 0; i < addons_cnmt_ctx.count; i++)
            {
                printf("Deleting %s...\n", addons_cnmt_ctx.addon_cnmt_xml[i].filepath.os_path);
                if(remove(addons_cnmt_ctx.addon_cnmt_xml[i].filepath.char_path) != 0)
                    printf("Error deleting file\n");
            }
        }
    }
    // Delete temp folder //
    printf("Deleting folder %s...\n", xci_ctx.tool_ctx->settings.secure_dir_path.os_path);
    if(rmdir(xci_ctx.tool_ctx->settings.secure_dir_path.char_path)!=0)
        printf("Error deleting file\n");
}

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

int ConvertXCI(const std::string& filename)
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
		return 0;
    }

    // Copy input file
    strncpy(input_name, filename.c_str(), sizeof(filename.length()));

    if (!(tool_ctx.file = fopen(input_name, "rb")))
    {
        fprintf(stderr, "unable to open %s: %s\n", input_name, strerror(errno));
		return 0;
    }

    xci_ctx_t xci_ctx;
    memset(&xci_ctx, 0, sizeof(xci_ctx));
    xci_ctx.file = tool_ctx.file;
    xci_ctx.tool_ctx = &tool_ctx;

    // Hardcode secure partition save path to "4nxci_extracted_nsp" directory
    filepath_init(&xci_ctx.tool_ctx->settings.secure_dir_path);
    filepath_set(&xci_ctx.tool_ctx->settings.secure_dir_path, "/4nxci_extracted_xci");

    printf("\n");

    xci_process(&xci_ctx);

    // Output NSP filename
    char nspFilename[MAX_PATH];
    NXFramework::GetFileBasename(nspFilename, filename.c_str());

    // Process ncas in cnmts
    printf("===> Processing Application Metadata:\n");
    cnmt_gamecard_process(xci_ctx.tool_ctx, &application_cnmt_xml, &application_cnmt, &application_nsp, nspFilename);
    if (patch_cnmt.title_id != 0)
    {
        printf("===> Processing Patch Metadata:\n");
        std::string nspUPDFilename(std::string(nspFilename) + std::string(" [UPD]"));
        cnmt_download_process(xci_ctx.tool_ctx, &patch_cnmt_xml, &patch_cnmt, &patch_nsp, nspUPDFilename.c_str());
    }
    if (addons_cnmt_ctx.count != 0)
    {
        addon_nsps = (nsp_ctx_t *)calloc(1, sizeof(nsp_ctx_t) * addons_cnmt_ctx.count);
        printf("===> Processing %u Addon(s):\n", addons_cnmt_ctx.count);
        std::string nspDLCFilename(std::string(nspFilename) + std::string(" [DLC]"));
        for (int i = 0; i < addons_cnmt_ctx.count; i++)
        {
            printf("===> Processing AddOn %i Metadata:\n", i + 1);
            cnmt_gamecard_process(xci_ctx.tool_ctx, &addons_cnmt_ctx.addon_cnmt_xml[i], &addons_cnmt_ctx.addon_cnmt[i], &addon_nsps[i], nspDLCFilename.c_str());
        }
    }

    printf("\nSummary:\n");
    printf("Game NSP: %s\n", application_nsp.filepath.char_path);
    if (patch_cnmt.title_id != 0)
        printf("Update NSP: %s\n", patch_nsp.filepath.char_path);
    if (addons_cnmt_ctx.count != 0)
    {
        for (int i2 = 0; i2 < addons_cnmt_ctx.count; i2++)
            printf("DLC NSP %i: %s\n", i2 + 1, addon_nsps[i2].filepath.char_path);
    }
    fclose(tool_ctx.file);

    printf("\nClean-up:\n");

    // Clean-up
    CleanUp(xci_ctx);

    // Rename NCAs to NSP NCAids
    //RenameNCAs(xci_ctx);

	printf("\n");
    printf("\nDone!\n");
    return 1;
}
