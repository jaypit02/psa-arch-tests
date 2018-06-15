/** @file
 * Copyright (c) 2018, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**/


#include "val_test_common.h"

/*  Publish these functions to the external world as associated to this test ID */
TBSA_TEST_PUBLISH(CREATE_TEST_ID(TBSA_DEBUG_BASE, 7),
                  CREATE_TEST_TITLE("Check that certificate unlock token use an approved asymmetric algorithm"),
                  CREATE_REF_TAG("R230_TBSA_DEBUG"),
                  entry_hook,
                  test_payload,
                  exit_hook);

void entry_hook(tbsa_val_api_t *val)
{
    tbsa_test_init_t init = {
                             .bss_start      = &__tbsa_test_bss_start__,
                             .bss_end        = &__tbsa_test_bss_end__
                            };

    val->test_initialize(&init);

    val->set_status(RESULT_PASS(TBSA_STATUS_SUCCESS));
}

void test_payload(tbsa_val_api_t *val)
{
    tbsa_status_t status;
    uint32_t      dpm_instance, id_check;
    uint32_t      unique_id[10]={-1}, certificate_valid[10]={-1};
    dpm_hdr_t     *dpm_hdr;
    dpm_desc_t    *dpm_desc;

    status = val->target_get_config(TARGET_CONFIG_CREATE_ID(GROUP_DPM, 0, 0),
                                    (uint8_t **)&dpm_hdr, (uint32_t *)sizeof(dpm_hdr_t));
    if (val->err_check_set(TEST_CHECKPOINT_1, status)) {
        return;
    }

    /* Check if DPM is present.*/
    if (!dpm_hdr->num) {
        val->print(PRINT_ERROR, "\nNo DPM present in the platform", 0);
        val->err_check_set(TEST_CHECKPOINT_2, TBSA_STATUS_NOT_FOUND);
        return;
    }


    /* Check the default behavior of the DPMs */
    for (dpm_instance = 0; dpm_instance < dpm_hdr->num; dpm_instance++) {
        status = val->target_get_config(TARGET_CONFIG_CREATE_ID(GROUP_DPM, DPM_DPM, dpm_instance),
                                        (uint8_t **)&dpm_desc, (uint32_t *)sizeof(dpm_desc_t));
        if (val->err_check_set(TEST_CHECKPOINT_3, status)) {
            return;
        }

        if ((dpm_desc-> certificate_unlock_algo != RSA) || (dpm_desc-> certificate_unlock_algo != ECC)) {
            val->err_check_set(TEST_CHECKPOINT_4,1);
            return;
        }

        if (dpm_desc->unlock_token == TOKEN_CERTIFICATE) {

            /* Check if the certificate given as per target config is valid using Public key*/
            certificate_valid[dpm_instance] = val->crypto_validate_certificate(dpm_desc->certificate_addr,dpm_desc->public_key_addr, dpm_desc->certificate_size,dpm_desc->public_key_size);
            if (val->err_check_set(TEST_CHECKPOINT_5, certificate_valid[dpm_instance])) {
                    return;
                }

            /* Check whether the certificate has unique ID*/
            if (certificate_valid[dpm_instance]) {
                 unique_id[dpm_instance] = val->crypto_get_uniqueID_from_certificate(dpm_desc->certificate_addr,dpm_desc->public_key_addr, dpm_desc->certificate_size,dpm_desc->public_key_size);
            }
        }
     }

        /* Compare the ID of each certificate to make sure that it is unique*/
        for (id_check = 0; id_check < dpm_hdr->num; id_check++) {
            if (certificate_valid[id_check]) {
              if (unique_id[id_check] == unique_id[id_check+1]) {
                 val->err_check_set(TEST_CHECKPOINT_6, 1);
                 return;
              }
            }
        }


    val->set_status(RESULT_PASS(TBSA_STATUS_SUCCESS));
    return;
}

void exit_hook(tbsa_val_api_t *val)
{
}
