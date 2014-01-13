/*
 * mod_simpleamd for FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2014, Christopher M. Rienzo <chris@rienzo.com>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Christopher M. Rienzo <chris@rienzo.com>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Christopher M. Rienzo <chris@rienzo.com>
 *
 * Maintainer: Christopher M. Rienzo <chris@rienzo.com>
 *
 * mod_simpleamd.c -- Crude answering machine detector using a simple energy threshold for VAD
 *
 */
#include <switch.h>

#include "simpleamd/simpleamd.h"

/* Defines module interface to FreeSWITCH */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_simpleamd_shutdown);
SWITCH_MODULE_LOAD_FUNCTION(mod_simpleamd_load);
SWITCH_MODULE_DEFINITION(mod_simpleamd, mod_simpleamd_load, mod_simpleamd_shutdown, NULL);

#define SIMPLEVAD_START_SYNTAX ""
#define SIMPLEVAD_STOP_SYNTAX ""

#define SIMPLEAMD_START_SYNTAX ""
#define SIMPLEAMD_STOP_SYNTAX ""

/**
 * module data
 */
static struct {
	switch_memory_pool_t *pool;
} globals;

/**
 * Forward logs
 * @param level
 * @param file that sent the log
 * @param line number that sent the log
 * @param user_data
 * @param message log message
 */
static void log_handler(samd_log_level_t level, void *user_data, const char *file, int line, const char *message)
{
	switch_log_level_t slevel = SWITCH_LOG_DEBUG;
	switch(level) {
		case SAMD_LOG_DEBUG:
			slevel = SWITCH_LOG_DEBUG;
			break;
		case SAMD_LOG_INFO:
			slevel = SWITCH_LOG_INFO;
			break;
		case SAMD_LOG_WARNING:
			slevel = SWITCH_LOG_WARNING;
			break;
		case SAMD_LOG_ERROR:
			slevel = SWITCH_LOG_ERROR;
			break;
	}
	switch_log_printf(SWITCH_CHANNEL_ID_LOG, file, "", line, switch_core_session_get_uuid((switch_core_session_t *)user_data), slevel, "%s\n", message);
}

/**
 * Forward VAD events
 *
 * @param vad_event
 * @param samples
 * @param user_data
 */
static void vad_event_handler(samd_vad_event_t vad_event, uint32_t samples, void *user_data)
{
	if (vad_event == SAMD_VAD_VOICE_BEGIN || vad_event == SAMD_VAD_SILENCE_BEGIN) {
		switch_event_t *event = NULL;
		switch_core_session_t *session = (switch_core_session_t *)user_data;
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_INFO, "Processing VAD event\n");
		if (switch_event_create_subclass(&event, SWITCH_EVENT_CUSTOM, "simpleamd::vad") == SWITCH_STATUS_SUCCESS) {
			if (vad_event == SAMD_VAD_VOICE_BEGIN) {
				switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "Value", "speaking");
			} else {
				switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "Value", "silent");
			}
			switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "Unique-ID", switch_core_session_get_uuid(session));
			switch_channel_event_set_data(switch_core_session_get_channel(session), event);
			switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_INFO, "Firing VAD event\n");
			switch_event_fire(&event);
		}
	}
}

/**
 * Process a buffer of audio data for VAD events
 *
 * @param bug the session's media bug
 * @param user_data the detector
 * @param type the type of data available from the bug
 * @return SWITCH_TRUE
 */
static switch_bool_t vad_process_buffer(switch_media_bug_t *bug, void *user_data, switch_abc_type_t type)
{
	samd_vad_t *vad = (samd_vad_t *)user_data;

	switch(type) {
	case SWITCH_ABC_TYPE_INIT:
		break;
	case SWITCH_ABC_TYPE_READ_REPLACE:
	{
		switch_frame_t *frame;
		if ((frame = switch_core_media_bug_get_read_replace_frame(bug))) {
			samd_vad_process_buffer(vad, frame->data, frame->samples);
			switch_core_media_bug_set_read_replace_frame(bug, frame);
		}
		break;
	}
	case SWITCH_ABC_TYPE_CLOSE:
		samd_vad_destroy(&vad);
		break;
	default:
		break;
	}

	return SWITCH_TRUE;
}

/**
 * APP interface to start VAD
 */
SWITCH_STANDARD_APP(simplevad_start_app)
{
	switch_channel_t *channel = switch_core_session_get_channel(session);
	switch_media_bug_t *bug = NULL;
	samd_vad_t *vad = NULL;

	/* are we already running? */
	bug = switch_channel_get_private(channel, "_mod_simpleamd_vad");
	if (bug) {
		switch_channel_set_variable(channel, SWITCH_CURRENT_APPLICATION_RESPONSE_VARIABLE, "-ERR already running");
		return;
	}

	/* Create detector */
	samd_vad_init(&vad, vad_event_handler, session);
	samd_vad_set_log_handler(vad, log_handler);
	/* TODO detector params */

	/* Add media bug */
	switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_DEBUG, "Starting VAD\n");
	switch_core_media_bug_add(session, "_mod_simpleamd_vad", NULL, vad_process_buffer, vad, 0, SMBF_READ_REPLACE | SMBF_NO_PAUSE, &bug);
	if (!bug) {
		samd_vad_destroy(&vad);
		switch_channel_set_variable(channel, SWITCH_CURRENT_APPLICATION_RESPONSE_VARIABLE, "-ERR can't create media bug");
		return;
	}
	switch_channel_set_private(channel, "_mod_simpleamd_vad", bug);

	switch_channel_set_variable(channel, SWITCH_CURRENT_APPLICATION_RESPONSE_VARIABLE, "+OK started");
}

/**
 * APP interface to stop VAD
 */
SWITCH_STANDARD_APP(simplevad_stop_app)
{
	switch_channel_t *channel = switch_core_session_get_channel(session);
	switch_media_bug_t *bug = switch_channel_get_private(channel, "_mod_simpleamd_vad");
	if (bug) {
		switch_core_media_bug_remove(session, &bug);
		switch_channel_set_private(channel, "_mod_simpleamd_vad", NULL);
		switch_channel_set_variable(channel, SWITCH_CURRENT_APPLICATION_RESPONSE_VARIABLE, "+OK stopped");
	} else {
		switch_channel_set_variable(channel, SWITCH_CURRENT_APPLICATION_RESPONSE_VARIABLE, "-ERR not running");
	}
}

/**
 * Forward AMD events
 *
 * @param vad_event
 * @param samples
 * @param user_data
 */
static void amd_event_handler(samd_event_t amd_event, uint32_t samples, void *user_data)
{
	switch_event_t *event = NULL;
	switch_core_session_t *session = (switch_core_session_t *)user_data;
	switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_INFO, "Processing AMD event\n");
	if (switch_event_create_subclass(&event, SWITCH_EVENT_CUSTOM, "simpleamd::amd") == SWITCH_STATUS_SUCCESS) {
		switch (amd_event) {
			case SAMD_DEAD_AIR:
				switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "Value", "dead-air");
				break;
			case SAMD_MACHINE_VOICE:
				switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "Value", "machine-speaking");
				break;
			case SAMD_MACHINE_SILENCE:
				switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "Value", "machine-silent");
				break;
			case SAMD_HUMAN_VOICE:
				switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "Value", "human-speaking");
				break;
			case SAMD_HUMAN_SILENCE:
				switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "Value", "human-silent");
				break;
		}
		switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "Unique-ID", switch_core_session_get_uuid(session));
		switch_channel_event_set_data(switch_core_session_get_channel(session), event);
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_INFO, "Firing AMD event\n");
		switch_event_fire(&event);
	}
}

/**
 * Process a buffer of audio data for AMD events
 *
 * @param bug the session's media bug
 * @param user_data the detector
 * @param type the type of data available from the bug
 * @return SWITCH_TRUE
 */
static switch_bool_t amd_process_buffer(switch_media_bug_t *bug, void *user_data, switch_abc_type_t type)
{
	samd_t *amd = (samd_t *)user_data;

	switch(type) {
	case SWITCH_ABC_TYPE_INIT:
		break;
	case SWITCH_ABC_TYPE_READ_REPLACE:
	{
		switch_frame_t *frame;
		if ((frame = switch_core_media_bug_get_read_replace_frame(bug))) {
			samd_process_buffer(amd, frame->data, frame->samples);
			switch_core_media_bug_set_read_replace_frame(bug, frame);
		}
		break;
	}
	case SWITCH_ABC_TYPE_CLOSE:
		samd_destroy(&amd);
		break;
	default:
		break;
	}

	return SWITCH_TRUE;
}

/**
 * APP interface to start AMD
 */
SWITCH_STANDARD_APP(simpleamd_start_app)
{
	switch_channel_t *channel = switch_core_session_get_channel(session);
	switch_media_bug_t *bug = NULL;
	samd_t *amd = NULL;

	/* are we already running? */
	bug = switch_channel_get_private(channel, "_mod_simpleamd_amd");
	if (bug) {
		switch_channel_set_variable(channel, SWITCH_CURRENT_APPLICATION_RESPONSE_VARIABLE, "-ERR already running");
		return;
	}

	/* Create detector */
	samd_init(&amd, amd_event_handler, session);
	samd_set_log_handler(amd, log_handler);
	/* TODO detector params */

	/* Add media bug */
	switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_DEBUG, "Starting AMD\n");
	switch_core_media_bug_add(session, "_mod_simpleamd_amd", NULL, amd_process_buffer, amd, 0, SMBF_READ_REPLACE | SMBF_NO_PAUSE, &bug);
	if (!bug) {
		samd_destroy(&amd);
		switch_channel_set_variable(channel, SWITCH_CURRENT_APPLICATION_RESPONSE_VARIABLE, "-ERR can't create media bug");
		return;
	}
	switch_channel_set_private(channel, "_mod_simpleamd_amd", bug);

	switch_channel_set_variable(channel, SWITCH_CURRENT_APPLICATION_RESPONSE_VARIABLE, "+OK started");
}

/**
 * APP interface to stop AMD
 */
SWITCH_STANDARD_APP(simpleamd_stop_app)
{
	switch_channel_t *channel = switch_core_session_get_channel(session);
	switch_media_bug_t *bug = switch_channel_get_private(channel, "_mod_simpleamd_amd");
	if (bug) {
		switch_core_media_bug_remove(session, &bug);
		switch_channel_set_private(channel, "_mod_simpleamd_amd", NULL);
		switch_channel_set_variable(channel, SWITCH_CURRENT_APPLICATION_RESPONSE_VARIABLE, "+OK stopped");
	} else {
		switch_channel_set_variable(channel, SWITCH_CURRENT_APPLICATION_RESPONSE_VARIABLE, "-ERR not running");
	}
}

/**
 * Configure the module
 * @return SWITCH_STATUS_SUCCESS if successful
 */
static switch_status_t do_config(switch_memory_pool_t *pool)
{
	switch_status_t status = SWITCH_STATUS_SUCCESS;
#if 0
	char *cf = "simpleamd.conf";
	switch_xml_t cfg, xml, settings;

	if (!(xml = switch_xml_open_cfg(cf, &cfg, NULL))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "open of %s failed\n", cf);
		return SWITCH_STATUS_TERM;
	}

	/* get params */
	settings = switch_xml_child(cfg, "settings");
	if (settings) {
		switch_xml_t param;
		for (param = switch_xml_child(settings, "param"); param; param = param->next) {
			char *var = (char *) switch_xml_attr_soft(param, "name");
			//char *val = (char *) switch_xml_attr_soft(param, "value");
			if (!strcasecmp(var, "foo")) {
				/* TODO */
			} else {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Unsupported param: %s\n", var);
			}
		}
	}

	switch_xml_free(xml);
#endif
	return status;
}

/**
 * Called when FreeSWITCH loads the module
 */
SWITCH_MODULE_LOAD_FUNCTION(mod_simpleamd_load)
{
	switch_application_interface_t *app;

	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	globals.pool = pool;

	SWITCH_ADD_APP(app, "simplevad_start", "Start VAD", "Start VAD", simplevad_start_app, SIMPLEVAD_START_SYNTAX, SAF_MEDIA_TAP);
	SWITCH_ADD_APP(app, "simplevad_stop", "Stop VAD", "Stop VAD", simplevad_stop_app, SIMPLEVAD_STOP_SYNTAX, SAF_NONE);

	SWITCH_ADD_APP(app, "simpleamd_start", "Start AMD", "Start AMD", simpleamd_start_app, SIMPLEAMD_START_SYNTAX, SAF_MEDIA_TAP);
	SWITCH_ADD_APP(app, "simpleamd_stop", "Stop AMD", "Stop AMD", simpleamd_stop_app, SIMPLEAMD_STOP_SYNTAX, SAF_NONE);

	if (do_config(pool) != SWITCH_STATUS_SUCCESS) {
		return SWITCH_STATUS_TERM;
	}

	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}

/**
 * Called when FreeSWITCH stops the module
 */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_simpleamd_shutdown)
{
	return SWITCH_STATUS_SUCCESS;
}


/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4 noet:
 */
