/*
 * Copyright (C) 2014 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include<stdio.h>
#include<ctype.h>

#include<hardware/bluetooth.h>
#include<hardware/bt_hl.h>

#include "if-main.h"
#include "pollhandler.h"
#include "../hal-utils.h"

SINTMAP(bthl_mdep_role_t, -1, "(unknown)")
	DELEMENT(BTHL_MDEP_ROLE_SOURCE),
	DELEMENT(BTHL_MDEP_ROLE_SINK),
ENDMAP

SINTMAP(bthl_channel_type_t, -1, "(unknown)")
	DELEMENT(BTHL_CHANNEL_TYPE_RELIABLE),
	DELEMENT(BTHL_CHANNEL_TYPE_STREAMING),
	DELEMENT(BTHL_CHANNEL_TYPE_ANY),
ENDMAP

const bthl_interface_t *if_hl = NULL;

static bthl_callbacks_t hl_cbacks = {
	.size = sizeof(hl_cbacks),
	.app_reg_state_cb = NULL,
	.channel_state_cb = NULL,
};

/* init */

static void init_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_hl);

	EXEC(if_hl->init, &hl_cbacks);
}

/* register_application */

static void register_application_p(int argc, const char **argv)
{
	bthl_reg_param_t reg;
	uint16_t mdep_argc_init, mdep_argc_off;
	int app_id = -1;
	int i;

	RETURN_IF_NULL(if_hl);

	if (argc <= 2) {
		haltest_error("No app name is specified\n");
		return;
	}

	if (argc <= 3) {
		haltest_error("No provider is specified\n");
		return;
	}

	if (argc <= 4) {
		haltest_error("No service name is specified\n");
		return;
	}

	if (argc <= 5) {
		haltest_error("No service description is specified\n");
		return;
	}

	if (argc <= 6) {
		haltest_error("No num of mdeps is specified\n");
		return;
	}

	memset(&reg, 0, sizeof(reg));

	if (argc != ((atoi(argv[6]) * 4) + 7)) {
		haltest_error("mdep cfg argumetns are not proper\n");
		return;
	}

	reg.application_name = argv[2];

	if (strcmp("-", argv[3]))
		reg.provider_name = argv[3];

	if (strcmp("-", argv[4]))
		reg.srv_name = argv[4];

	if (strcmp("-", argv[5]))
		reg.srv_desp = argv[5];

	reg.number_of_mdeps = atoi(argv[6]);

	reg.mdep_cfg = malloc(reg.number_of_mdeps * sizeof(bthl_mdep_cfg_t));
	mdep_argc_init = 7;

	for (i = 0; i < reg.number_of_mdeps; i++) {
		mdep_argc_off = mdep_argc_init + (4 * i);
		reg.mdep_cfg[i].mdep_role =
				str2bthl_mdep_role_t(argv[mdep_argc_off]);
		reg.mdep_cfg[i].data_type = atoi(argv[mdep_argc_off + 1]);
		reg.mdep_cfg[i].channel_type =
			str2bthl_channel_type_t(argv[mdep_argc_off + 2]);

		if (!strcmp("-", argv[mdep_argc_off + 3])) {
			reg.mdep_cfg[i].mdep_description = NULL;
			continue;
		}

		reg.mdep_cfg[i].mdep_description = argv[mdep_argc_off + 3];
	}

	EXEC(if_hl->register_application, &reg, &app_id);

	free(reg.mdep_cfg);
}

/* unregister_application */

static void unregister_application_p(int argc, const char **argv)
{
	uint32_t app_id;

	RETURN_IF_NULL(if_hl);

	if (argc <= 2) {
		haltest_error("No app id is specified");
		return;
	}

	app_id = (uint32_t) atoi(argv[2]);

	EXEC(if_hl->unregister_application, app_id);
}

/* cleanup */

static void cleanup_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_hl);

	EXECV(if_hl->cleanup);
	if_hl = NULL;
}

static struct method methods[] = {
	STD_METHOD(init),
	STD_METHODH(register_application,
		"<app_name> <provider_name> <srv_name> <srv_descr>\n"
		"<num_of_mdeps>\n"
		"[[<mdep_role>] [<data_type>] [<channel_type>] [<mdep_descr>]]"
		"..."),
	STD_METHODH(unregister_application, "<app_id>"),
	STD_METHOD(cleanup),
	END_METHOD
};

const struct interface hl_if = {
	.name = "hl",
	.methods = methods
};
