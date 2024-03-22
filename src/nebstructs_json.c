#include <stdio.h>
#include <string.h>
#include <json-c/json.h>

/* include naemon */
#include "naemon/naemon.h"

#include "nebstructs_json.h"

// This function converts host_check_data into a json_object
// Remember to call json_object_put() an pass the json_object to free up the memory
json_object *nebstruct_encode_host_check_as_json(nebstruct_host_check_data *ds)
{
    json_object *my_object;
    json_object *hostcheck_object;
    nebstruct_host_check_data *current_hostcheck;

    char *raw_command;
    host *current_host;

    my_object = json_object_new_object();
    hostcheck_object = json_object_new_object();

    json_object_object_add(my_object, "nebstruct", json_object_new_string("nebstruct_host_check_data"));
    json_object_object_add(my_object, "type", json_object_new_int(ds->type));
    json_object_object_add(my_object, "flags", json_object_new_int(ds->flags));
    json_object_object_add(my_object, "attr", json_object_new_int(ds->attr));
    json_object_object_add(my_object, "timestamp", json_object_new_int(ds->timestamp.tv_sec));

    current_host = (host *)ds->object_ptr;
    current_hostcheck = ds;

    HOSTCHECKFIELD_STRING(host_name);

    raw_command = NULL;
    get_raw_command_line_r(get_global_macros(), current_host->check_command_ptr, current_host->check_command, &raw_command, 0);

    json_object_object_add(hostcheck_object, "command_line", (raw_command != NULL ? json_object_new_string(raw_command) : NULL));
    json_object_object_add(hostcheck_object, "command_name", (current_host->check_command != NULL ? json_object_new_string(current_host->check_command) : NULL));

    HOSTCHECKFIELD_STRING(output);
    HOSTCHECKFIELD_STRING(long_output);
    HOSTCHECKFIELD_STRING(perf_data);
    HOSTCHECKFIELD_INT(check_type);
    HOSTCHECKFIELD_INT(current_attempt);
    HOSTCHECKFIELD_INT(max_attempts);
    HOSTCHECKFIELD_INT(state_type);
    HOSTCHECKFIELD_INT(state);
    HOSTCHECKFIELD_INT(timeout);
    json_object_object_add(hostcheck_object, "start_time", json_object_new_int64(ds->start_time.tv_sec));
    json_object_object_add(hostcheck_object, "end_time", json_object_new_int64(ds->end_time.tv_sec));
    HOSTCHECKFIELD_INT(early_timeout);
    HOSTCHECKFIELD_DOUBLE(execution_time);
    HOSTCHECKFIELD_DOUBLE(latency);
    HOSTCHECKFIELD_INT(return_code);

    json_object_object_add(my_object, "hostcheck", hostcheck_object);

    free(raw_command);

    return my_object;
}

// This function converts service_check_data into a json_object
// Remember to call json_object_put() an pass the json_object to free up the memory
json_object *nebstruct_encode_service_check_as_json(nebstruct_service_check_data *ds)
{
    json_object *my_object;
    json_object *servicecheck_object;
    nebstruct_service_check_data *current_servicecheck;

    char *raw_command;
    service *current_service;

    my_object = json_object_new_object();
    servicecheck_object = json_object_new_object();

    json_object_object_add(my_object, "nebstruct", json_object_new_string("nebstruct_service_check_data"));
    json_object_object_add(my_object, "type", json_object_new_int(ds->type));
    json_object_object_add(my_object, "flags", json_object_new_int(ds->flags));
    json_object_object_add(my_object, "attr", json_object_new_int(ds->attr));
    json_object_object_add(my_object, "timestamp", json_object_new_int(ds->timestamp.tv_sec));

    current_service = (service *)ds->object_ptr;
    current_servicecheck = ds;

    SERVICECHECKFIELD_STRING(host_name);
    SERVICECHECKFIELD_STRING(service_description);

    raw_command = NULL;
    get_raw_command_line_r(get_global_macros(), current_service->check_command_ptr, current_service->check_command, &raw_command, 0);

    json_object_object_add(servicecheck_object, "command_line", (raw_command != NULL ? json_object_new_string(raw_command) : NULL));
    json_object_object_add(servicecheck_object, "command_name", (current_service->check_command != NULL ? json_object_new_string(current_service->check_command) : NULL));

    SERVICECHECKFIELD_STRING(output);
    SERVICECHECKFIELD_STRING(long_output);
    SERVICECHECKFIELD_STRING(perf_data);
    SERVICECHECKFIELD_INT(check_type);
    SERVICECHECKFIELD_INT(current_attempt);
    SERVICECHECKFIELD_INT(max_attempts);
    SERVICECHECKFIELD_INT(state_type);
    SERVICECHECKFIELD_INT(state);
    SERVICECHECKFIELD_INT(timeout);
    json_object_object_add(servicecheck_object, "start_time", json_object_new_int64(ds->start_time.tv_sec));
    json_object_object_add(servicecheck_object, "end_time", json_object_new_int64(ds->end_time.tv_sec));
    SERVICECHECKFIELD_INT(early_timeout);
    SERVICECHECKFIELD_DOUBLE(execution_time);
    SERVICECHECKFIELD_DOUBLE(latency);
    SERVICECHECKFIELD_INT(return_code);

    json_object_object_add(my_object, "servicecheck", servicecheck_object);

    free(raw_command);

    return my_object;
}

// This function converts nebstruct_host_status_data into a json_object
// Remember to call json_object_put() an pass the json_object to free up the memory
json_object *nebstruct_encode_host_status_as_json(nebstruct_host_status_data *ds)
{
    json_object *my_object;
    json_object *hoststatus_object;
    host *current_hoststatus;

    my_object = json_object_new_object();
    hoststatus_object = json_object_new_object();

    json_object_object_add(my_object, "nebstruct", json_object_new_string("nebstruct_host_status_data"));
    json_object_object_add(my_object, "type", json_object_new_int(ds->type));
    json_object_object_add(my_object, "flags", json_object_new_int(ds->flags));
    json_object_object_add(my_object, "attr", json_object_new_int(ds->attr));
    json_object_object_add(my_object, "timestamp", json_object_new_int(ds->timestamp.tv_sec));

    current_hoststatus = (host *)ds->object_ptr;

    HOSTSTATUSFIELD_STRING(name);
    HOSTSTATUSFIELD_STRING(plugin_output);
    HOSTSTATUSFIELD_STRING(long_plugin_output);
    HOSTSTATUSFIELD_STRING(event_handler);
    HOSTSTATUSFIELD_STRING(perf_data);
    HOSTSTATUSFIELD_STRING(check_command);
    HOSTSTATUSFIELD_STRING(check_period);
    HOSTSTATUSFIELD_INT(current_state);
    HOSTSTATUSFIELD_INT(has_been_checked);
    json_object_object_add(hoststatus_object, "should_be_scheduled", json_object_new_int64(1));
    HOSTSTATUSFIELD_INT(current_attempt);
    HOSTSTATUSFIELD_INT(max_attempts);
    HOSTSTATUSFIELD_INT(last_check);
    HOSTSTATUSFIELD_INT(next_check);
    HOSTSTATUSFIELD_INT(check_type);
    HOSTSTATUSFIELD_INT(last_state_change);
    HOSTSTATUSFIELD_INT(last_hard_state_change);
    HOSTSTATUSFIELD_INT(last_hard_state);
    HOSTSTATUSFIELD_INT(last_time_up);
    HOSTSTATUSFIELD_INT(last_time_down);
    HOSTSTATUSFIELD_INT(last_time_unreachable);
    HOSTSTATUSFIELD_INT(state_type);
    HOSTSTATUSFIELD_INT(last_notification);
    HOSTSTATUSFIELD_INT(next_notification);
    HOSTSTATUSFIELD_INT(no_more_notifications);
    HOSTSTATUSFIELD_INT(notifications_enabled);
    HOSTSTATUSFIELD_INT(problem_has_been_acknowledged);
    HOSTSTATUSFIELD_INT(acknowledgement_type);
    HOSTSTATUSFIELD_INT(current_notification_number);
    HOSTSTATUSFIELD_INT(accept_passive_checks);
    HOSTSTATUSFIELD_INT(event_handler_enabled);
    HOSTSTATUSFIELD_INT(checks_enabled);
    HOSTSTATUSFIELD_INT(flap_detection_enabled);
    HOSTSTATUSFIELD_INT(is_flapping);
    HOSTSTATUSFIELD_DOUBLE(percent_state_change);
    HOSTSTATUSFIELD_DOUBLE(latency);
    HOSTSTATUSFIELD_DOUBLE(execution_time);
    HOSTSTATUSFIELD_INT(scheduled_downtime_depth);
    HOSTSTATUSFIELD_INT(process_performance_data);
    HOSTSTATUSFIELD_INT(obsess);
    HOSTSTATUSFIELD_INT(modified_attributes);
    HOSTSTATUSFIELD_DOUBLE(check_interval);
    HOSTSTATUSFIELD_DOUBLE(retry_interval);

    json_object_object_add(my_object, "hoststatus", hoststatus_object);

    return my_object;
}

// This function converts nebstruct_service_status_data into a json_object
// Remember to call json_object_put() an pass the json_object to free up the memory
json_object *nebstruct_encode_service_status_as_json(nebstruct_service_status_data *ds)
{
    json_object *my_object;
    json_object *servicestatus_object;
    service *current_servicestatus;

    my_object = json_object_new_object();
    servicestatus_object = json_object_new_object();

    json_object_object_add(my_object, "nebstruct", json_object_new_string("nebstruct_service_status_data"));
    json_object_object_add(my_object, "type", json_object_new_int(ds->type));
    json_object_object_add(my_object, "flags", json_object_new_int(ds->flags));
    json_object_object_add(my_object, "attr", json_object_new_int(ds->attr));
    json_object_object_add(my_object, "timestamp", json_object_new_int(ds->timestamp.tv_sec));

    current_servicestatus = (service *)ds->object_ptr;


    SERVICESTATUSFIELD_STRING(host_name);
    SERVICESTATUSFIELD_STRING(description);
    SERVICESTATUSFIELD_STRING(plugin_output);
    SERVICESTATUSFIELD_STRING(long_plugin_output);
    SERVICESTATUSFIELD_STRING(event_handler);
    SERVICESTATUSFIELD_STRING(perf_data);
    SERVICESTATUSFIELD_STRING(check_command);
    SERVICESTATUSFIELD_STRING(check_period);
    SERVICESTATUSFIELD_INT(current_state);
    SERVICESTATUSFIELD_INT(has_been_checked);
    json_object_object_add(servicestatus_object, "should_be_scheduled", json_object_new_int64(1));
    SERVICESTATUSFIELD_INT(current_attempt);
    SERVICESTATUSFIELD_INT(max_attempts);
    SERVICESTATUSFIELD_INT(last_check);
    SERVICESTATUSFIELD_INT(next_check);
    SERVICESTATUSFIELD_INT(check_type);
    SERVICESTATUSFIELD_INT(last_state_change);
    SERVICESTATUSFIELD_INT(last_hard_state_change);
    SERVICESTATUSFIELD_INT(last_hard_state);
    SERVICESTATUSFIELD_INT(last_time_ok);
    SERVICESTATUSFIELD_INT(last_time_warning);
    SERVICESTATUSFIELD_INT(last_time_critical);
    SERVICESTATUSFIELD_INT(last_time_unknown);
    SERVICESTATUSFIELD_INT(state_type);
    SERVICESTATUSFIELD_INT(last_notification);
    SERVICESTATUSFIELD_INT(next_notification);
    SERVICESTATUSFIELD_INT(no_more_notifications);
    SERVICESTATUSFIELD_INT(notifications_enabled);
    SERVICESTATUSFIELD_INT(problem_has_been_acknowledged);
    SERVICESTATUSFIELD_INT(acknowledgement_type);
    SERVICESTATUSFIELD_INT(current_notification_number);
    SERVICESTATUSFIELD_INT(accept_passive_checks);
    SERVICESTATUSFIELD_INT(event_handler_enabled);
    SERVICESTATUSFIELD_INT(checks_enabled);
    SERVICESTATUSFIELD_INT(flap_detection_enabled);
    SERVICESTATUSFIELD_INT(is_flapping);
    SERVICESTATUSFIELD_DOUBLE(percent_state_change);
    SERVICESTATUSFIELD_DOUBLE(latency);
    SERVICESTATUSFIELD_DOUBLE(execution_time);
    SERVICESTATUSFIELD_INT(scheduled_downtime_depth);
    SERVICESTATUSFIELD_INT(process_performance_data);
    SERVICESTATUSFIELD_INT(obsess);
    SERVICESTATUSFIELD_INT(modified_attributes);
    SERVICESTATUSFIELD_DOUBLE(check_interval);
    SERVICESTATUSFIELD_DOUBLE(retry_interval);

    json_object_object_add(my_object, "servicestatus", servicestatus_object);

    return my_object;
}