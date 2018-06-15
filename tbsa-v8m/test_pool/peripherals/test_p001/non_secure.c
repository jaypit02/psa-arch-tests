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

/**
  Publish these functions to the external world as associated to this test ID
**/
TBSA_TEST_PUBLISH(CREATE_TEST_ID(TBSA_PERIPHERALS_BASE, 1),
                  CREATE_TEST_TITLE("Check direct access to clock and power functionality from NT"),
                  CREATE_REF_TAG("R190_TBSA_INFRA"),
                  entry_hook,
                  test_payload,
                  exit_hook);

clocks_desc_t         *clocks_desc;

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
    uint32_t      data;

    status = val->target_get_config(TARGET_CONFIG_CREATE_ID(GROUP_CLOCKS, CLOCKS_SYS_FREQ, 0),
                                    (uint8_t **)&clocks_desc,
                                    (uint32_t *)sizeof(clocks_desc_t));
    if (val->err_check_set(TEST_CHECKPOINT_2, status)) {
        return;
    }

    val->set_status(RESULT_PENDING(status));

    val->print(PRINT_ALWAYS, "\n clock: %x", clocks_desc->pll_base);
    /* Trying to read the trusted PLL base address, expect secure fault? */
    val_mem_read_wide((uint32_t *)(clocks_desc->pll_base + clocks_desc->offset), &data);

    /* wait here till pending status is cleared by secure fault */
    while (IS_TEST_PENDING(val->get_status()));

    /* Restoring default Handler */
    status = val->interrupt_restore_handler(EXCP_NUM_SF);
    if (val->err_check_set(TEST_CHECKPOINT_3, status)) {
        return;
    }
}

void exit_hook(tbsa_val_api_t *val)
{
}
