#include <stdio.h>
#include <string.h>

/* include naemon */
#include "naemon/naemon.h"

// specify event broker API version (required)
NEB_API_VERSION(CURRENT_NEB_API_VERSION);

void *statusengine_module_handle = NULL;

//Broker initialize function
int nebmodule_init(int flags, char *args, nebmodule *handle){

	//Save handle
	statusengine_module_handle = handle;

	//I guess nagios don't use this?
	neb_set_module_info(statusengine_module_handle, NEBMODULE_MODINFO_TITLE,   "Statusengine - the missing event broker");
	neb_set_module_info(statusengine_module_handle, NEBMODULE_MODINFO_AUTHOR,  "Daniel Ziegler");
	neb_set_module_info(statusengine_module_handle, NEBMODULE_MODINFO_TITLE,   "Copyright (c) 2014 - present Daniel Ziegler");
	neb_set_module_info(statusengine_module_handle, NEBMODULE_MODINFO_VERSION, "3.0.2");
	neb_set_module_info(statusengine_module_handle, NEBMODULE_MODINFO_LICENSE, "GPL v2");
	neb_set_module_info(statusengine_module_handle, NEBMODULE_MODINFO_DESC,    "A powerful and flexible event broker");

	//Welcome messages
    nm_log(NSLOG_INFO_MESSAGE, "HALLO TEST 123");


	//Register callbacks
	//neb_register_callback(NEBCALLBACK_HOST_STATUS_DATA,                 statusengine_module_handle, 0, statusengine_handle_data);
	

	return 0;
}


//Broker deinitialize function
int nebmodule_deinit(int flags, int reason){

	// Deregister all callbacks
	//neb_deregister_callback(NEBCALLBACK_HOST_STATUS_DATA,                 statusengine_handle_data);

	return 0;
}