/* include naemon */
#include "naemon/naemon.h"
#include <json-c/json.h>

/* For processing nebstruct_host_check_data data */
#define HOSTCHECKFIELD_STRING(FIELD) \
	json_object_object_add(hostcheck_object, #FIELD, (current_hostcheck->FIELD != NULL ? json_object_new_string(current_hostcheck->FIELD) : NULL))

#define HOSTCHECKFIELD_INT(FIELD) \
	json_object_object_add(hostcheck_object, #FIELD, json_object_new_int64(current_hostcheck->FIELD))

#define HOSTCHECKFIELD_DOUBLE(FIELD) \
	json_object_object_add(hostcheck_object, #FIELD, json_object_new_double(current_hostcheck->FIELD))

json_object *nebstruct_encode_host_check_as_json(nebstruct_host_check_data *ds);

/* For processing nebstruct_servicecheck_data data */
#define SERVICECHECKFIELD_STRING(FIELD) \
	json_object_object_add(servicecheck_object, #FIELD, (current_servicecheck->FIELD != NULL ? json_object_new_string(current_servicecheck->FIELD) : NULL))

#define SERVICECHECKFIELD_INT(FIELD) \
	json_object_object_add(servicecheck_object, #FIELD, json_object_new_int64(current_servicecheck->FIELD))

#define SERVICECHECKFIELD_DOUBLE(FIELD) \
	json_object_object_add(servicecheck_object, #FIELD, json_object_new_double(current_servicecheck->FIELD))

json_object *nebstruct_encode_service_check_as_json(nebstruct_service_check_data *ds);

/* For processing nebstruct_host_status_data data */
#define HOSTSTATUSFIELD_STRING(FIELD) \
	json_object_object_add(hoststatus_object, #FIELD, (current_hoststatus->FIELD != NULL ? json_object_new_string(current_hoststatus->FIELD) : NULL))

#define HOSTSTATUSFIELD_INT(FIELD) \
	json_object_object_add(hoststatus_object, #FIELD, json_object_new_int64(current_hoststatus->FIELD))

#define HOSTSTATUSFIELD_DOUBLE(FIELD) \
	json_object_object_add(hoststatus_object, #FIELD, json_object_new_double(current_hoststatus->FIELD))

json_object *nebstruct_encode_host_status_as_json(nebstruct_host_status_data *ds);

/* For processing nebstruct_service_status_data data */
#define SERVICESTATUSFIELD_STRING(FIELD) \
	json_object_object_add(servicestatus_object, #FIELD, (current_servicestatus->FIELD != NULL ? json_object_new_string(current_servicestatus->FIELD) : NULL))

#define SERVICESTATUSFIELD_INT(FIELD) \
	json_object_object_add(servicestatus_object, #FIELD, json_object_new_int64(current_servicestatus->FIELD))

#define SERVICESTATUSFIELD_DOUBLE(FIELD) \
	json_object_object_add(servicestatus_object, #FIELD, json_object_new_double(current_servicestatus->FIELD))

json_object *nebstruct_encode_service_status_as_json(nebstruct_service_status_data *ds);

