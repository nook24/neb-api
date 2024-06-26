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

// Mutex for hashmaps
pthread_mutex_t status_mutex;

// Hash tables
GHashTable *map_hoststatus;
GHashTable *map_servicestatus;

// HTTP request callback
static void fn(struct mg_connection *c, int ev, void *ev_data)
{

	if (ev == MG_EV_OPEN && c->is_listening)
	{
		// empty
	}
	else if (ev == MG_EV_HTTP_MSG)
	{
		int err;
		char errmsg[256] = "Unknown error";

		struct mg_http_message *hm = (struct mg_http_message *)ev_data;

		if (mg_http_match_uri(hm, "/hoststatus"))
		{
			char hostname[1024];
			mg_http_get_var(&hm->query, "hostname", hostname, sizeof(hostname));

			// nm_log(NSLOG_PROCESS_INFO, "NEB-API: Requested host status for '%s'", hostname);

			err = pthread_mutex_lock(&status_mutex);
			if (err)
			{
				strerror_r(err, errmsg, 256);
				nm_log(NSLOG_RUNTIME_ERROR, "Error locking status_mutex: %s (%d)", errmsg, err);
			}

			if (strlen(hostname) == 0)
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
			else
			{
				// Return the latest host status for a specific host
				const char *hoststatus = g_hash_table_lookup(map_hoststatus, hostname);
				if (hoststatus != NULL)
				{
					mg_http_reply(c, 200, "Content-Type: application/json\r\n", hoststatus);
				}
				else
				{
					mg_http_reply(c, 404, "Content-Type: application/json\r\n", "{\"error\": \"Host not found\"}");
				}
			}

			err = pthread_mutex_unlock(&status_mutex);
			if (err)
			{
				strerror_r(err, errmsg, 256);
				nm_log(NSLOG_RUNTIME_ERROR, "Error unlocking status_mutex: %s (%d)", errmsg, err);
			}
		}

		if (mg_http_match_uri(hm, "/servicestatus"))
		{
			char hostname[1024];
			char service_description[1024];
			mg_http_get_var(&hm->query, "hostname", hostname, sizeof(hostname));
			mg_http_get_var(&hm->query, "service_description", service_description, sizeof(service_description));

			nm_log(NSLOG_PROCESS_INFO, "NEB-API: Requested service status for '%s/%s'", hostname, service_description);

			err = pthread_mutex_lock(&status_mutex);
			if (err)
			{
				strerror_r(err, errmsg, 256);
				nm_log(NSLOG_RUNTIME_ERROR, "Error locking status_mutex: %s (%d)", errmsg, err);
			}

			if (strlen(hostname) == 0 || strlen(service_description) == 0)
			{
				// Return the latest service status for all services

				GList *values = g_hash_table_get_values(map_servicestatus);
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
			else
			{
				// Return the latest service status for a specific service
				char *key = nm_malloc(strlen(hostname) + strlen(service_description) + 2); // +2 for the null-terminator and separator
				sprintf(key, "%s_%s", hostname, service_description);

				const char *servicestatus = g_hash_table_lookup(map_servicestatus, key);
				if (servicestatus != NULL)
				{
					mg_http_reply(c, 200, "Content-Type: application/json\r\n", servicestatus);
				}
				else
				{
					mg_http_reply(c, 404, "Content-Type: application/json\r\n", "{\"error\": \"Service not found\"}");
				}

				free(key);
			}

			err = pthread_mutex_unlock(&status_mutex);
			if (err)
			{
				strerror_r(err, errmsg, 256);
				nm_log(NSLOG_RUNTIME_ERROR, "Error unlocking status_mutex: %s (%d)", errmsg, err);
			}
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
	int err;
	char errmsg[256] = "Unknown error";

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
	err = pthread_mutex_lock(&status_mutex);
	if (err)
	{
		strerror_r(err, errmsg, 256);
		nm_log(NSLOG_RUNTIME_ERROR, "Error locking status_mutex: %s (%d)", errmsg, err);
	}

	g_hash_table_insert(map_hoststatus, g_strdup(current_host->name), g_strdup(json_string));

	err = pthread_mutex_unlock(&status_mutex);
	if (err)
	{
		strerror_r(err, errmsg, 256);
		nm_log(NSLOG_RUNTIME_ERROR, "Error unlocking status_mutex: %s (%d)", errmsg, err);
	}

	// Release resources
	json_object_put(hoststatus_object);

	return NEB_OK;
}

// Naemon callback for service status
static int cb_service_status_data(int cb, void *data)
{
	json_object *servicestatus_object;
	const char *json_string;
	int err;
	char errmsg[256] = "Unknown error";

	nebstruct_service_status_data *ds = (nebstruct_service_status_data *)data;
	if (ds == NULL)
	{
		return NEB_OK;
	}

	service *current_service = (service *)ds->object_ptr;

	// Encode the current service status event as JSON object
	servicestatus_object = nebstruct_encode_service_status_as_json(ds);

	// Convert the JSON object to a string
	json_string = json_object_to_json_string(servicestatus_object);

	// Push service status to web sockets clients (async)
	push_to_clients(&mgr, json_string);

	// Push latest service status into hashmap
	char *key = nm_malloc(strlen(current_service->host_name) + strlen(current_service->description) + 2); // +2 for the null-terminator and separator
	sprintf(key, "%s_%s", current_service->host_name, current_service->description);

	printf("Key: '%s'\n", key);

	err = pthread_mutex_lock(&status_mutex);
	if (err)
	{
		strerror_r(err, errmsg, 256);
		nm_log(NSLOG_RUNTIME_ERROR, "Error locking status_mutex: %s (%d)", errmsg, err);
	}

	g_hash_table_insert(map_servicestatus, g_strdup(key), g_strdup(json_string));

	guint size = g_hash_table_size(map_servicestatus);
    g_print("The hash table contains %u elements.\n", size);

	err = pthread_mutex_unlock(&status_mutex);
	if (err)
	{
		strerror_r(err, errmsg, 256);
		nm_log(NSLOG_RUNTIME_ERROR, "Error unlocking status_mutex: %s (%d)", errmsg, err);
	}

	// Release resources
	json_object_put(servicestatus_object);

	free(key);

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

	if (pthread_mutex_init(&status_mutex, NULL) != 0)
	{
		nm_log(NSLOG_RUNTIME_ERROR, "NEB-API: Failed to init status_mutex");
		return NEB_ERROR;
	}

	// Start the web server in a separate thread
	if (pthread_create(&tid, NULL, web_server_thread, NULL) != 0)
	{
		nm_log(NSLOG_CONFIG_ERROR, "NEB-API: Failed to create web server thread");
		return NEB_ERROR;
	}

	// Create hashtabls
	map_hoststatus = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
	map_servicestatus = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);

	// Register callbacks
	neb_register_callback(NEBCALLBACK_HOST_CHECK_DATA, nebapi_module_handle, 0, cb_host_check_data);
	neb_register_callback(NEBCALLBACK_SERVICE_CHECK_DATA, nebapi_module_handle, 0, cb_service_check_data);
	neb_register_callback(NEBCALLBACK_HOST_STATUS_DATA, nebapi_module_handle, 0, cb_host_status_data);
	neb_register_callback(NEBCALLBACK_SERVICE_STATUS_DATA, nebapi_module_handle, 0, cb_service_status_data);

	return NEB_OK;
}

// Broker deinitialize function
int nebmodule_deinit(int flags, int reason)
{

	// Deregister all callbacks
	neb_deregister_callback(NEBCALLBACK_HOST_CHECK_DATA, cb_host_check_data);
	neb_deregister_callback(NEBCALLBACK_SERVICE_CHECK_DATA, cb_service_check_data);
	neb_deregister_callback(NEBCALLBACK_HOST_STATUS_DATA, cb_host_status_data);
	neb_deregister_callback(NEBCALLBACK_SERVICE_STATUS_DATA, cb_service_status_data);

	// Wait for the web server thread to terminate
	nm_log(NSLOG_INFO_MESSAGE, "Wait for the web server thread to terminate");
	stop_web_server_thread();
	pthread_join(tid, NULL);

	// Cleanup hashmaps
	g_hash_table_destroy(map_hoststatus);
	g_hash_table_destroy(map_servicestatus);

	return NEB_OK;
}