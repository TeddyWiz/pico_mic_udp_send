// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/xlogging.h"

#include "hsm_client_data.h"
#include "tpm_msr.h"
#include "symm_key.h"

#ifdef HSM_TYPE_RIOT
#include "riot_info.h"
#endif

#define DATA_SIG_LENGTH     1024

typedef struct IOTHUB_HSM_IMPL_TAG
{
#ifdef HSM_TYPE_RIOT    
    X509_INFO_HANDLE x509_info;
#endif
    TPM_INFO_HANDLE tpm_info;
    SYMM_KEY_INFO_HANDLE key_info;
} IOTHUB_HSM_IMPL;

int hsm_client_tpm_init(void)
{
    return 0;
}

void hsm_client_tpm_deinit(void)
{
}

int hsm_client_key_init(void)
{
    initialize_symm_key();
    return 0;
}

void hsm_client_key_deinit(void)
{
}

HSM_CLIENT_HANDLE iothub_hsm_tpm_create(void)
{
    IOTHUB_HSM_IMPL* result;
    result = malloc(sizeof(IOTHUB_HSM_IMPL));
    if (result == NULL)
    {
        (void)printf("Failure: malloc IOTHUB_HSM_IMPL.");
    }
    else
    {
        memset(result, 0, sizeof(IOTHUB_HSM_IMPL));
        if ((result->tpm_info = tpm_msr_create()) == NULL)
        {
            (void)printf("Failure: tpm_msr_create.");
#ifdef HSM_TYPE_RIOT
            x509_info_destroy(result->x509_info);
#endif
            free(result);
            result = NULL;
        }
    }
    return (HSM_CLIENT_HANDLE)result;
}

HSM_CLIENT_HANDLE iothub_hsm_key_create(void)
{
    IOTHUB_HSM_IMPL* result;
    result = malloc(sizeof(IOTHUB_HSM_IMPL));
    if (result == NULL)
    {
        (void)printf("Failure: malloc IOTHUB_HSM_IMPL.");
    }
    else
    {
        memset(result, 0, sizeof(IOTHUB_HSM_IMPL));
        if ((result->key_info = symm_key_info_create()) == NULL)
        {
            (void)printf("Failure: symm_key_create.");
            free(result);
            result = NULL;
        }
    }
    return (HSM_CLIENT_HANDLE)result;
}

void iothub_hsm_destroy(HSM_CLIENT_HANDLE handle)
{
    if (handle != NULL)
    {
        IOTHUB_HSM_IMPL* hsm_impl = (IOTHUB_HSM_IMPL*)handle;
#ifdef HSM_TYPE_RIOT
        x509_info_destroy(hsm_impl->x509_info);
#endif
        tpm_msr_destroy(hsm_impl->tpm_info);
        symm_key_info_destroy(hsm_impl->key_info);
        free(hsm_impl);
    }
}

int iothub_tpm_hsm_get_endorsement_key(HSM_CLIENT_HANDLE handle, unsigned char** key, size_t* key_len)
{
    int result;
    if (handle == NULL || key == NULL || key_len == NULL)
    {
        (void)printf("Invalid parameter specified");
        result = __LINE__;
    }
    else
    {
        unsigned char data_sig[DATA_SIG_LENGTH];
        size_t length = DATA_SIG_LENGTH;

        IOTHUB_HSM_IMPL* hsm_impl = (IOTHUB_HSM_IMPL*)handle;
        if (tpm_msr_get_ek(hsm_impl->tpm_info, data_sig, &length) != 0)
        {
            (void)printf("Invalid handle value specified");
            result = __LINE__;
        }
        else
        {
            if ((*key = (unsigned char*)malloc(length)) == NULL)
            {
                printf("Failure creating buffer handle");
                result = __LINE__;
            }
            else
            {
                memcpy(*key, data_sig, length);
                *key_len = (size_t)length;
                result = 0;
            }
        }
    }
    return result;
}

int iothub_tpm_hsm_get_storage_root_key(HSM_CLIENT_HANDLE handle, unsigned char** key, size_t* key_len)
{
    int result;
    if (handle == NULL || key == NULL || key_len == NULL)
    {
        (void)printf("Invalid handle value specified");
        result = __LINE__;
    }
    else
    {
        unsigned char data_sig[DATA_SIG_LENGTH];
        size_t length = DATA_SIG_LENGTH;

        IOTHUB_HSM_IMPL* hsm_impl = (IOTHUB_HSM_IMPL*)handle;
        if (tpm_msr_get_srk(hsm_impl->tpm_info, data_sig, &length) != 0)
        {
            (void)printf("Invalid handle value specified");
            result = __LINE__;
        }
        else
        {
            if ((*key = (unsigned char*)malloc(length)) == NULL)
            {
                printf("Failure creating buffer handle");
                result = __LINE__;
            }
            else
            {
                memcpy(*key, data_sig, length);
                *key_len = (size_t)length;
                result = 0;
            }
        }
    }
    return result;
}

int iothub_tpm_hsm_sign_with_identity(HSM_CLIENT_HANDLE handle, const unsigned char* data, size_t data_len, unsigned char** key, size_t* key_len)
{
    int result;
    if (handle == NULL || data == NULL || key == NULL || key_len == NULL)
    {
        (void)printf("Invalid handle value specified");
        result = __LINE__;
    }
    else
    {
        unsigned char data_signature[DATA_SIG_LENGTH];
        size_t length = DATA_SIG_LENGTH;

        IOTHUB_HSM_IMPL* hsm_impl = (IOTHUB_HSM_IMPL*)handle;
        if (tpm_msr_sign_data(hsm_impl->tpm_info, data, data_len, data_signature, &length) != 0 && length != 0)
        {
            (void)printf("Invalid handle value specified");
            result = __LINE__;
        }
        else
        {
            if ((*key = (unsigned char*)malloc(length)) == NULL)
            {
                printf("Failure creating buffer handle");
                result = __LINE__;
            }
            else
            {
                memcpy(*key, data_signature, length);
                *key_len = length;
                result = 0;
            }
        }
    }
    return result;
}

int iothub_tpm_hsm_activate_identity_key(HSM_CLIENT_HANDLE handle, const unsigned char* key, size_t key_len)
{
    int result;
    if (handle == NULL || key == NULL || key_len == 0)
    {
        (void)printf("Invalid handle value specified");
        result = __LINE__;
    }
    else
    {
        IOTHUB_HSM_IMPL* hsm_impl = (IOTHUB_HSM_IMPL*)handle;
        result = tpm_msr_import_key(hsm_impl->tpm_info, key, key_len);
    }
    return result;
}

char* iothub_hsm_key_symm_key(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        (void)printf("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        IOTHUB_HSM_IMPL* hsm_impl = (IOTHUB_HSM_IMPL*)handle;
        const char* key = symm_key_info_get_key(hsm_impl->key_info);
        if (key == NULL)
        {
            LogError("Failed retrieving key");
            result = NULL;
        }
        else
        {
            if (mallocAndStrcpy_s(&result, key) != 0)
            {
                LogError("Failed to allocate key buffer.");
                result = NULL;
            }
        }
    }
    return result;
}

char* iothub_hsm_get_registry_id(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        (void)printf("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        IOTHUB_HSM_IMPL* hsm_impl = (IOTHUB_HSM_IMPL*)handle;
        const char* reg_id = symm_key_info_get_reg_id(hsm_impl->key_info);
        if (reg_id == NULL)
        {
            LogError("Failed retrieving register Id");
            result = NULL;
        }
        else
        {
            if (mallocAndStrcpy_s(&result, reg_id) != 0)
            {
                LogError("Failed to allocate register Id.");
                result = NULL;
            }
        }
    }
    return result;
}

#ifdef HSM_TYPE_RIOT

int hsm_client_x509_init(void)
{
    int result = 0;
    initialize_device();
    return result;
}

void hsm_client_x509_deinit(void)
{
    // no-op.
}

HSM_CLIENT_HANDLE iothub_hsm_x509_create(void)
{
    IOTHUB_HSM_IMPL* result;
    result = malloc(sizeof(IOTHUB_HSM_IMPL));
    if (result == NULL)
    {
        (void)printf("Failure: malloc IOTHUB_HSM_IMPL.");
    }
    else
    {
        memset(result, 0, sizeof(IOTHUB_HSM_IMPL));
        if ((result->x509_info = x509_info_create()) == NULL)
        {
            (void)printf("Failure: x509_info_create.");
        }
    }
    return (HSM_CLIENT_HANDLE)result;
}

char* iothub_x509_hsm_get_certificate(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        (void)printf("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        IOTHUB_HSM_IMPL* hsm_impl = (IOTHUB_HSM_IMPL*)handle;
        const char* certificate = x509_info_get_cert(hsm_impl->x509_info);
        if (certificate == NULL)
        {
            LogError("Failed retrieving certificate");
            result = NULL;
        }
        else
        {
            if (mallocAndStrcpy_s(&result, certificate) != 0)
            {
                LogError("Failed to allocate cert buffer.");
                result = NULL;
            }
        }
    }
    return result;
}

char* iothub_x509_hsm_get_alias_key(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        (void)printf("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        IOTHUB_HSM_IMPL* hsm_impl = (IOTHUB_HSM_IMPL*)handle;
        const char* key = x509_info_get_key(hsm_impl->x509_info);
        if (key == NULL)
        {
            LogError("Failed retrieving certificate key");
            result = NULL;
        }
        else
        {
            if (mallocAndStrcpy_s(&result, key) != 0)
            {
                LogError("Failed to allocate cert buffer.");
                result = NULL;
            }
        }
    }
    return result;
}

char* iothub_hsm_get_common_name(HSM_CLIENT_HANDLE handle)
{
    char* result;
    if (handle == NULL)
    {
        (void)printf("Invalid handle value specified");
        result = NULL;
    }
    else
    {
        IOTHUB_HSM_IMPL* hsm_impl = (IOTHUB_HSM_IMPL*)handle;
        const char* cn = x509_info_get_cn(hsm_impl->x509_info);
        if (cn == NULL)
        {
            LogError("Failed retrieving certificate CN");
            result = NULL;
        }
        else
        {
            if (mallocAndStrcpy_s(&result, cn) != 0)
            {
                LogError("Failed to allocate cert buffer.");
                result = NULL;
            }
        }
    }
    return result;
}

// Defining the v-table for the x509 hsm calls
static const HSM_CLIENT_X509_INTERFACE x509_interface =
{
    iothub_hsm_x509_create,
    iothub_hsm_destroy,
    iothub_x509_hsm_get_certificate,
    iothub_x509_hsm_get_alias_key,
    iothub_hsm_get_common_name
};

const HSM_CLIENT_X509_INTERFACE* hsm_client_x509_interface(void)
{
    return &x509_interface;
}

#endif // HSM_TYPE_RIOT

static const HSM_CLIENT_TPM_INTERFACE tpm_interface =
{
    iothub_hsm_tpm_create,
    iothub_hsm_destroy,
    iothub_tpm_hsm_activate_identity_key,
    iothub_tpm_hsm_get_endorsement_key,
    iothub_tpm_hsm_get_storage_root_key,
    iothub_tpm_hsm_sign_with_identity
};

const HSM_CLIENT_TPM_INTERFACE* hsm_client_tpm_interface(void)
{
    // tpm interface pointer
    return &tpm_interface;
}

static const HSM_CLIENT_KEY_INTERFACE key_interface =
{
    iothub_hsm_tpm_create,
    iothub_hsm_destroy,
    iothub_hsm_key_symm_key,
    iothub_hsm_get_registry_id
};

const HSM_CLIENT_KEY_INTERFACE* hsm_client_key_interface(void)
{
    // tpm interface pointer
    return &key_interface;
}
