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
static int ws_connected_clients = 0;

struct mg_mgr mgr;

// Web server thread
pthread_t tid;

// Hash tables
GHashTable *map_hoststatus;

// HTTP request callback
static void fn(struct mg_connection *c, int ev, void *ev_data)
{

	if (ev == MG_EV_OPEN && c->is_listening)
	{
		// empty
	}
	else if (ev == MG_EV_HTTP_MSG)
	{
		struct mg_http_message *hm = (struct mg_http_message *)ev_data;

		struct mg_str caps[3];
		if (mg_match(hm->uri, mg_str("/hoststatus/#/"), caps))
		{

			// https://mongoose.ws/documentation/#mg_match
			char hostname[256]; // Enough for any valid hostname

			mg_http_get_var(&hm->uri, "#", hostname, sizeof(hostname));
			printf("Hostname: '%s'\n", caps[0]);

			// A hostname is set, return the host status for this host
			const char *host_status = (const char *)g_hash_table_lookup(map_hoststatus, (gpointer)hostname);
			if (host_status != NULL)
			{
				mg_http_reply(c, 200, "Content-Type: application/json\r\n", host_status);
			}
			else
			{
				mg_http_reply(c, 404, "Content-Type: text/plain\r\n", "Host not found");
			}
		}
		else if (mg_http_match_uri(hm, "/hoststatus"))
		{
			// Return the latest host status for all hosts

			GList *values = g_hash_table_get_values(map_hoststatus);
			GString *json_array = g_string_new("[");

			// Iterate through the list and append the strings
			for (GList *iter = values; iter != NULL; iter = iter->next)
			{
				const char *current_value = (const char *)iter->data;
				g_string_append(json_array, current_value);

				// If there is a next element, append a comma
				if (iter->next != NULL)
				{
					g_string_append(json_array, ",");
				}
			}

			g_string_append(json_array, "]");
			mg_http_reply(c, 200, "Content-Type: application/json\r\n", json_array->str);

			g_string_free(json_array, TRUE);
			g_list_free(values);
		}

		if (mg_http_match_uri(hm, "/websocket"))
		{
			// New WebSocket client connected
			ws_connected_clients++;
			mg_ws_upgrade(c, hm, NULL); // Upgrade HTTP to Websocket

			// Set some unique mark on a connection
			// c->data[0] = 'W';
			strncpy(c->data, g_uuid_string_random(), sizeof(c->data) - 1);
			c->data[sizeof(c->data) - 1] = '\0';
		}
		else
		{
			// Serve static files
			// struct mg_http_serve_opts opts = {.root_dir = s_web_root};
			// mg_http_serve_dir(c, ev_data, &opts);
		}
	}
	else if (ev == MG_EV_CLOSE)
	{
		// WebSocket Client disconnected
		ws_connected_clients--;
	}
	else if (ev == MG_EV_WS_MSG)
	{
		// Got websocket frame. Received data is wm->data. Echo it back!
		struct mg_ws_message *wm = (struct mg_ws_message *)ev_data;
		mg_ws_send(c, wm->data.ptr, wm->data.len, WEBSOCKET_OP_TEXT);
		mg_iobuf_del(&c->recv, 0, c->recv.len);
	}
	else if (ev == MG_EV_WAKEUP)
	{
		struct mg_str *data = (struct mg_str *)ev_data;
		// Broadcast message to all connected websocket clients.
		// Traverse over all connections
		for (struct mg_connection *wc = c->mgr->conns; wc != NULL; wc = wc->next)
		{
			// Send only to marked connections
			// if (wc->data[0] == 'W') {
			mg_ws_send(wc, data->ptr, data->len, WEBSOCKET_OP_TEXT);
			// }
		}
	}

	/*
		if (ev == MG_EV_HTTP_MSG)
		{
			struct mg_http_message *hm = (struct mg_http_message *)ev_data;
			if (mg_http_match_uri(hm, "/hoststatus"))
			{
				// Return the latest host status for all hosts

				GList *values = g_hash_table_get_values(map_hoststatus);
				// Iterate through the list and print the strings
				for (GList *iter = values; iter != NULL; iter = iter->next)
				{
					const char *current_value = (const char *)iter->data;
					mg_http_reply(c, 200, "Host: foo.com\r\n", current_value);
				}

				g_list_free(values);
			}

			else if (mg_http_match_uri(hm, "/websocket"))
			{
				// New WebSocket client
				mg_ws_upgrade(c, hm, NULL); // Upgrade HTTP to Websocket
				c->data[0] = 'W';			// Set some unique mark on the connection
			}
		}

		else if (ev == MG_EV_WAKEUP)
		{
			struct mg_str *data = (struct mg_str *)ev_data;
			mg_http_reply(c, 200, "", "Result: %.*s\n", data->len, data->ptr);
		}
		*/
}

// Web server thread function
void *web_server_thread(void *data)
{

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
		mg_wakeup(mgr, c->id, buf, strlen(buf)); // Send to parent
												 // mg_ws_send(c, buf, strlen(buf), WEBSOCKET_OP_TEXT);
	}
}

static int have_push_clients(struct mg_mgr *mgr)
{
	return ws_connected_clients > 0;
}

// Naemon callback for host checks
static int cb_host_check_data(int cb, void *data)
{
	json_object *hostcheck_object;
	const char *json_string;

	nebstruct_host_check_data *ds = (nebstruct_host_check_data *)data;

	if (ds->type != NEBTYPE_HOSTCHECK_PROCESSED)
		return NEB_OK;

	// Encode the current host check event as JSON object
	hostcheck_object = nebstruct_encode_host_check_as_json(ds);

	// Convert the JSON object to a string
	json_string = json_object_to_json_string(hostcheck_object);

	// have_push_clients(&mgr);

	// Push host check to web sockets clients (async)
	push_to_clients(&mgr, json_string);

	// Release resources
	json_object_put(hostcheck_object);

	return NEB_OK;
}

// Naemon callback for service checks
static int cb_service_check_data(int cb, void *data)
{
	json_object *servicecheck_object;
	const char *json_string;

	nebstruct_service_check_data *ds = (nebstruct_service_check_data *)data;

	if (ds->type != NEBTYPE_SERVICECHECK_PROCESSED)
		return NEB_OK;

	// Encode the current service check event as JSON object
	servicecheck_object = nebstruct_encode_service_check_as_json(ds);

	// Convert the JSON object to a string
	json_string = json_object_to_json_string(servicecheck_object);

	// Push service check to web sockets clients (async)
	push_to_clients(&mgr, json_string);

	// Release resources
	json_object_put(servicecheck_object);

	return NEB_OK;
}

// Naemon callback for host status
static int cb_host_status_data(int cb, void *data)
{
	json_object *hoststatus_object;
	const char *json_string;

	nebstruct_host_status_data *ds = (nebstruct_host_status_data *)data;
	if (ds == NULL)
	{
		return NEB_OK;
	}

	host *current_host = (host *)ds->object_ptr;

	// Encode the current host status event as JSON object
	hoststatus_object = nebstruct_encode_host_status_as_json(ds);

	// Convert the JSON object to a string
	json_string = json_object_to_json_string(hoststatus_object);

	// Push host status to web sockets clients (async)
	push_to_clients(&mgr, json_string);

	// Push latest host status into hashmap
	g_hash_table_insert(map_hoststatus, (gpointer)current_host->name, g_strdup(json_string));

	// Release resources
	json_object_put(hoststatus_object);

	return NEB_OK;
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
	map_hoststatus = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);

	// Register callbacks
	neb_register_callback(NEBCALLBACK_HOST_CHECK_DATA, nebapi_module_handle, 0, cb_host_check_data);
	// neb_register_callback(NEBCALLBACK_SERVICE_CHECK_DATA, nebapi_module_handle, 0, cb_service_check_data);
	neb_register_callback(NEBCALLBACK_HOST_STATUS_DATA, nebapi_module_handle, 0, cb_host_status_data);

	return NEB_OK;
}

// Broker deinitialize function
int nebmodule_deinit(int flags, int reason)
{

	// Deregister all callbacks
	neb_deregister_callback(NEBCALLBACK_HOST_CHECK_DATA, cb_host_check_data);
	// neb_deregister_callback(NEBCALLBACK_SERVICE_CHECK_DATA, cb_service_check_data);
	neb_deregister_callback(NEBCALLBACK_HOST_STATUS_DATA, cb_host_status_data);

	// Wait for the web server thread to terminate
	nm_log(NSLOG_INFO_MESSAGE, "Wait for the web server thread to terminate");
	stop_web_server_thread();
	pthread_join(tid, NULL);

	// Cleanup hashmaps
	g_hash_table_destroy(map_hoststatus);

	return NEB_OK;
}