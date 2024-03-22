#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <json-c/json.h>
#include <glib.h>
#include <unistd.h>

#include "nebstructs_json.h"

/* include naemon */
#include "naemon/naemon.h"

#include "mongoose.h"

// specify event broker API version (required)
NEB_API_VERSION(CURRENT_NEB_API_VERSION);

void *nebapi_module_handle = NULL;

// Web server configuration
static int ws_debug_level = MG_LL_INFO;
static const char *ws_listening_address = "http://0.0.0.0:8000";
static int ws_exit_flag = false;

struct mg_mgr mgr;

// Web server thread
pthread_t tid;

// Hash tables
GHashTable *hostchecks;

// HTTP request callback
static void fn(struct mg_connection *c, int ev, void *ev_data)
{
	if (ev == MG_EV_HTTP_MSG)
	{
		struct mg_http_message *hm = (struct mg_http_message *)ev_data;
		if (mg_http_match_uri(hm, "/fast"))
		{

			GList *values = g_hash_table_get_values(hostchecks);
			// Iterate through the list and print the strings
			for (GList *iter = values; iter != NULL; iter = iter->next)
			{
				const char *current_value = (const char *)iter->data;
				mg_http_reply(c, 200, "Host: foo.com\r\n", current_value);
			}

			g_list_free(values);
			// Single-threaded code path, for performance comparison
			// The /fast URI responds immediately
			// mg_http_reply(c, 200, "Host: foo.com\r\n", json_string_dub);
		}
		else if (mg_http_match_uri(hm, "/websocket/hostchecks"))
		{
			mg_ws_upgrade(c, hm, NULL); // Upgrade HTTP to Websocket
			c->data[0] = 'W';			// Set some unique mark on the connection
		}
	}
	else if (ev == MG_EV_WAKEUP)
	{
		struct mg_str *data = (struct mg_str *)ev_data;
		mg_http_reply(c, 200, "", "Result: %.*s\n", data->len, data->ptr);
	}
}

// Web server thread function
void *web_server_thread(void *data)
{
	// struct mg_mgr mgr;
	//  struct mg_connection *nc;

	// Initialize Mongoose event manager
	mg_mgr_init(&mgr);
	mg_log_set(ws_debug_level);							  // Set debug log level
	mg_http_listen(&mgr, ws_listening_address, fn, NULL); // Create listener

	mg_wakeup_init(&mgr); // Initialise wakeup socket pair
	while (!ws_exit_flag)
	{
		mg_mgr_poll(&mgr, 1000);
	}
	mg_mgr_free(&mgr);

	return NULL;
}

// Function to stop the web server thread
void stop_web_server_thread()
{
	ws_exit_flag = true;
}

// asynchronous push method that will not block the Naemon core.
static void push_to_clients(struct mg_mgr *mgr, const char *buf)
{
	struct mg_connection *c;
	for (c = mgr->conns; c != NULL; c = c->next)
	{
		mg_ws_send(c, buf, strlen(buf), WEBSOCKET_OP_TEXT);
	}
}

static int cb_host_check_data(int cb, void *data)
{
	json_object *hostcheck_object;
	const char *json_string;

	nebstruct_host_check_data *ds = (nebstruct_host_check_data *)data;

	if (ds->type != NEBTYPE_HOSTCHECK_PROCESSED)
		return 0;

	// Encode the current host check event as JSON object
	hostcheck_object = nebstruct_encode_host_check_as_json(ds);

	// Convert the JSON object to a string
	json_string = json_object_to_json_string(hostcheck_object);

	// Push host check to web sockets clients (async)
	push_to_clients(&mgr, json_string);

	// Release resources
	json_object_put(hostcheck_object);

	return 0;
}

// Broker initialize function
int nebmodule_init(int flags, char *args, nebmodule *handle)
{

	// Save handle
	nebapi_module_handle = handle;

	// I guess nagios don't use this?
	neb_set_module_info(nebapi_module_handle, NEBMODULE_MODINFO_TITLE, "NEB-API - HTTP API for Naemon");
	neb_set_module_info(nebapi_module_handle, NEBMODULE_MODINFO_AUTHOR, "Daniel Ziegler");
	neb_set_module_info(nebapi_module_handle, NEBMODULE_MODINFO_TITLE, "Copyright (c) 2024 - present Daniel Ziegler");
	neb_set_module_info(nebapi_module_handle, NEBMODULE_MODINFO_VERSION, "1.0.0");
	neb_set_module_info(nebapi_module_handle, NEBMODULE_MODINFO_LICENSE, "GPL v2");
	neb_set_module_info(nebapi_module_handle, NEBMODULE_MODINFO_DESC, "This Event Broker Module launches a Web Server within the Naemon process and exposes an HTTP API");

	// Welcome messages
	nm_log(NSLOG_INFO_MESSAGE, "NEB-API: Hi :)");

	// Start the web server in a separate thread
	if (pthread_create(&tid, NULL, web_server_thread, NULL) != 0)
	{
		nm_log(NSLOG_CONFIG_ERROR, "NEB-API: Failed to create web server thread");
		return NEB_ERROR;
	}

	// Create hashtabls
	hostchecks = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);

	// Register callbacks
	neb_register_callback(NEBCALLBACK_HOST_CHECK_DATA, nebapi_module_handle, 0, cb_host_check_data);

	return NEB_OK;
}

// Broker deinitialize function
int nebmodule_deinit(int flags, int reason)
{

	// Deregister all callbacks
	neb_deregister_callback(NEBCALLBACK_HOST_CHECK_DATA, cb_host_check_data);

	// Wait for the web server thread to terminate
	nm_log(NSLOG_INFO_MESSAGE, "Wait for the web server thread to terminate");
	stop_web_server_thread();
	pthread_join(tid, NULL);

	// Cleanup hashmaps
	g_hash_table_destroy(hostchecks);

	return NEB_OK;
}