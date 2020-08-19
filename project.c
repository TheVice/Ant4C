/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "project.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "file_system.h"
#include "interpreter.h"
#include "listener.h"
#include "load_file.h"
#include "load_tasks.h"
#include "path.h"
#include "property.h"
#include "range.h"
#include "string_unit.h"
#include "target.h"
#include "text_encoding.h"
#include "version.h"
#include "xml.h"

#include <string.h>

static uint8_t project_is_init = 0;

static struct buffer project;

static const uint8_t* project_attributes[] =
{
	(const uint8_t*)"name",
	(const uint8_t*)"default",
	(const uint8_t*)"basedir",
	(const uint8_t*)"buildfile",
};

static const uint8_t project_attributes_lengths[] = { 4, 7, 7, 9 };

static const uint8_t project_attributes_count = 3;

#define PRIVATE_PROPERTIES		0

#define NAME_POSITION			0
#define DEFAULT_POSITION		1
#define BASE_DIR_POSITION		2
#define BUILD_FILE_POSITION		3

#define CONTENT_POSITION		1
#define PROPERTIES_POSITION		2
#define TARGETS_POSITION		3
#define MODULES_POSITION		4

#define COUNT_OF_POSITIONS		(MODULES_POSITION + 1)

uint8_t project_property_exists(const void* the_project,
								const uint8_t* property_name, uint8_t property_name_length,
								void** the_property, uint8_t verbose)
{
	if (NULL == the_project || NULL == property_name || 0 == property_name_length)
	{
		return 0;
	}

	(void)verbose;/*TODO*/
	const struct buffer* properties = buffer_buffer_data(the_project, PROPERTIES_POSITION);
	return property_exists(properties, property_name, property_name_length, the_property);
}

uint8_t project_private_property_exists(const void* the_project,
										const uint8_t* property_name, uint8_t property_name_length,
										void** the_property, uint8_t verbose)
{
	if (NULL == the_project || NULL == property_name || 0 == property_name_length)
	{
		return 0;
	}

	(void)verbose;/*TODO*/
	const struct buffer* private_properties = buffer_buffer_data(the_project, PRIVATE_PROPERTIES);
	return property_exists(private_properties, property_name, property_name_length, the_property);
}

uint8_t project_property_set_value(void* the_project,
								   const uint8_t* property_name, uint8_t property_name_length,
								   const uint8_t* property_value, ptrdiff_t property_value_length,
								   uint8_t dynamic, uint8_t over_write,
								   uint8_t read_only, uint8_t verbose)
{
	if (NULL == the_project || NULL == property_name || 0 == property_name_length)
	{
		return 0;
	}

	struct buffer* properties = buffer_buffer_data(the_project, PROPERTIES_POSITION);

	return property_set_by_name(properties, property_name, property_name_length,
								property_value, property_value_length, property_value_is_byte_array,
								dynamic, over_write, read_only, verbose);
}

uint8_t project_property_get_by_name(
	const void* the_project, const uint8_t* property_name, uint8_t property_name_length, struct buffer* output,
	uint8_t verbose)
{
	if (NULL == output)
	{
		return 0;
	}

	void* the_property = NULL;

	if (!project_property_exists(the_project, property_name, property_name_length, &the_property, verbose))
	{
		return 0;
	}

	return property_get_by_pointer(the_property, output);
}

uint8_t project_target_new(void* the_project, const uint8_t* target_name, uint8_t target_name_length,
						   const uint8_t* target_depends, uint16_t target_depends_length,
						   struct buffer* description,
						   const uint8_t* attributes_start, const uint8_t* attributes_finish,
						   const uint8_t* element_finish, uint8_t verbose)
{
	if (NULL == the_project ||
		NULL == target_name ||
		target_name_length < 1)
	{
		return 0;
	}

	struct buffer* targets = buffer_buffer_data(the_project, TARGETS_POSITION);

	if (target_exists(targets, target_name, target_name_length))
	{
		return 0;
	}

	return target_new(the_project,
					  target_name, target_name_length,
					  target_depends, target_depends_length,
					  description,
					  attributes_start, attributes_finish,
					  element_finish, targets, verbose);
}

uint8_t project_target_get(const void* the_project, const uint8_t* name, uint8_t name_length,
						   void** the_target,
						   uint8_t verbose)
{
	if (NULL == the_project || NULL == name || 0 == name_length || NULL == the_target)
	{
		return 0;
	}

	(void)verbose;/*TODO*/
	const struct buffer* targets = buffer_buffer_data(the_project, TARGETS_POSITION);
	return target_get(targets, name, name_length, the_target);
}

uint8_t project_target_exists(const void* the_project, const uint8_t* name, uint8_t name_length)
{
	if (NULL == the_project || NULL == name || 0 == name_length)
	{
		return 0;
	}

	const struct buffer* targets = buffer_buffer_data(the_project, TARGETS_POSITION);
	return target_exists(targets, name, name_length);
}

uint8_t project_target_has_executed(const void* the_project, const uint8_t* name, uint8_t name_length)
{
	if (NULL == the_project || NULL == name || 0 == name_length)
	{
		return 0;
	}

	const struct buffer* targets = buffer_buffer_data(the_project, TARGETS_POSITION);
	return target_has_executed(targets, name, name_length);
}

uint8_t project_add_module(void* the_project, const void* the_module, uint8_t length)
{
	if (NULL == the_project || NULL == the_module || 0 == length)
	{
		return 0;
	}

	struct buffer* modules = buffer_buffer_data(the_project, MODULES_POSITION);

	return buffer_append(modules, the_module, length);
}

const uint8_t* project_get_task_from_module(const void* the_project, const struct range* task_name,
		void** the_module_of_task)
{
	if (NULL == the_project || range_is_null_or_empty(task_name))
	{
		return 0;
	}

	const struct buffer* modules = buffer_buffer_data(the_project, MODULES_POSITION);
	return load_tasks_get_task(modules, task_name, the_module_of_task);
}

const uint8_t* project_get_function_from_module(const void* the_project,
		const struct range* name_space, const struct range* function_name,
		void** the_module_of_task, const uint8_t** name_space_at_module)
{
	if (NULL == the_project || range_is_null_or_empty(name_space) || range_is_null_or_empty(function_name))
	{
		return 0;
	}

	const struct buffer* modules = buffer_buffer_data(the_project, MODULES_POSITION);
	return load_tasks_get_function(modules, name_space, function_name, the_module_of_task, name_space_at_module);
}

uint8_t project_get_base_directory(const void* the_project, const void** the_property, uint8_t verbose)
{
	if (NULL == the_property)
	{
		return 0;
	}

	void* prop = NULL;
	const uint8_t returned = project_private_property_exists(the_project,
							 project_attributes[BASE_DIR_POSITION],
							 project_attributes_lengths[BASE_DIR_POSITION], &prop, verbose);
	(*the_property) = returned ? prop : NULL;
	return returned;
}

uint8_t project_get_buildfile_path(const void* the_project, const void** the_property, uint8_t verbose)
{
	if (NULL == the_property)
	{
		return 0;
	}

	void* prop = NULL;
	const uint8_t returned = project_private_property_exists(the_project,
							 project_attributes[BUILD_FILE_POSITION],
							 project_attributes_lengths[BUILD_FILE_POSITION], &prop, verbose);
	(*the_property) = returned ? prop : NULL;
	return returned;
}

uint8_t project_get_buildfile_uri(const void* the_property, struct buffer* build_file_uri, uint8_t verbose)
{
	if (NULL == the_property || NULL == build_file_uri)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(build_file_uri);

	if (!common_append_string_to_buffer((const uint8_t*)"file:///", build_file_uri))
	{
		return 0;
	}

	if (!property_get_by_pointer(the_property, build_file_uri))
	{
		return 0;
	}

	uint8_t* path_start = buffer_data(build_file_uri, size);
	uint8_t* path_finish = buffer_data(build_file_uri, 0) + buffer_size(build_file_uri);
#if defined(_WIN32)

	if (!cygpath_get_unix_path(path_start, path_finish))
	{
		return 0;
	}

#endif

	if (path_start + 8 < path_finish && '/' == *(path_start + 8))
	{
		path_start += 8;
		const ptrdiff_t length = path_finish - path_start;
		path_finish = path_start + 1;
		MEM_CPY(path_start, path_finish, length);
		return buffer_resize(build_file_uri, buffer_size(build_file_uri) - 1);
	}

	(void)verbose;/*TODO*/
	return 1;
}

uint8_t project_get_default_target(const void* the_project, const void** the_property, uint8_t verbose)
{
	if (NULL == the_property)
	{
		return 0;
	}

	void* prop = NULL;
	const uint8_t returned = project_private_property_exists(the_project,
							 project_attributes[DEFAULT_POSITION],
							 project_attributes_lengths[DEFAULT_POSITION], &prop, verbose);
	(*the_property) = returned ? prop : NULL;
	return returned;
}

uint8_t project_get_name(const void* the_project, const void** the_property, uint8_t verbose)
{
	if (NULL == the_property)
	{
		return 0;
	}

	void* prop = NULL;
	const uint8_t returned = project_private_property_exists(the_project,
							 project_attributes[NAME_POSITION],
							 project_attributes_lengths[NAME_POSITION], &prop, verbose);
	(*the_property) = returned ? prop : NULL;
	return returned;
}

uint8_t project_get_current_directory(const void* the_project, const void* the_target,
									  struct buffer* output, ptrdiff_t size, uint8_t verbose)
{
	if (!buffer_resize(output, size))
	{
		return 0;
	}

	const void* current_directory_property = NULL;

	if (!directory_get_current_directory(the_project, &current_directory_property, output, verbose))
	{
		return 0;
	}

	if (!interpreter_actualize_property_value(the_project, the_target,
			property_get_id_of_get_value_function(), current_directory_property,
			size, output, verbose))
	{
		return 0;
	}

	return 1;
}

uint8_t project_new(void** the_project)
{
	if (NULL == the_project)
	{
		return 0;
	}

	if (!project_is_init)
	{
		SET_NULL_TO_BUFFER(project);
		project_is_init = 1;
	}

	if (!common_get_attributes_and_arguments_for_task(NULL, NULL, COUNT_OF_POSITIONS, NULL, NULL, NULL, &project))
	{
		return 0;
	}

	*the_project = &project;
	return NULL != *the_project;
}

uint8_t project_load(void* the_project, uint8_t project_help, uint8_t verbose)
{
	if (NULL == the_project)
	{
		return 0;
	}

	struct range tag_name;

	const struct buffer* content = buffer_buffer_data(the_project, CONTENT_POSITION);

	BUFFER_TO_RANGE(tag_name, content);

	if (range_is_null_or_empty(&tag_name))
	{
		return 0;
	}

	struct buffer sub_nodes_names;

	SET_NULL_TO_BUFFER(sub_nodes_names);

	if (project_help &&
		!range_from_string((const uint8_t*)"project\0target\0description\0", 27, 3, &sub_nodes_names))
	{
		buffer_release(&sub_nodes_names);
		return 0;
	}

	struct buffer elements;

	SET_NULL_TO_BUFFER(elements);

	if (1 != xml_get_sub_nodes_elements(tag_name.start, tag_name.finish, &sub_nodes_names, &elements))
	{
		buffer_release(&elements);
		buffer_release(&sub_nodes_names);
		return 0;
	}

	struct range* element = NULL;

	if (NULL == (element = buffer_range_data(&elements, 0)))
	{
		buffer_release(&elements);
		buffer_release(&sub_nodes_names);
		return 0;
	}

	if (!xml_get_tag_name(element->start, element->finish, &tag_name))
	{
		buffer_release(&elements);
		buffer_release(&sub_nodes_names);
		return 0;
	}

	const uint8_t root_task_id = interpreter_get_task(tag_name.start, tag_name.finish);
	const uint8_t* element_finish = element->finish;
	const ptrdiff_t offset = project_get_source_offset(the_project, tag_name.finish);
	/**/
	listener_task_started(NULL, offset, the_project, NULL, root_task_id);
	uint8_t result = interpreter_evaluate_task(the_project, NULL,
					 root_task_id, &tag_name, element_finish, project_help, verbose);
	listener_task_finished(NULL, offset, the_project, NULL, root_task_id, result);

	if (!result ||
		!buffer_resize(&elements, 0))
	{
		buffer_release(&elements);
		buffer_release(&sub_nodes_names);
		return 0;
	}

	if (xml_get_sub_nodes_elements(tag_name.finish, element_finish, &sub_nodes_names, &elements))
	{
		result = interpreter_evaluate_tasks(the_project, NULL, &elements, project_help, verbose);
	}

	buffer_release(&elements);
	buffer_release(&sub_nodes_names);
	/**/
	return result;
}

uint8_t project_load_from_content(const uint8_t* content_start, const uint8_t* content_finish,
								  void* the_project, uint8_t project_help, uint8_t verbose)
{
	if (range_in_parts_is_null_or_empty(content_start, content_finish) ||
		NULL == the_project)
	{
		return 0;
	}

	struct buffer* content = buffer_buffer_data(the_project, CONTENT_POSITION);

	if (!buffer_resize(content, 0) ||
		!buffer_append(content, content_start, content_finish - content_start))
	{
		return 0;
	}

	return project_load(the_project, project_help, verbose);
}

uint8_t project_load_from_build_file(const struct range* path_to_build_file,
									 const struct range* current_directory,
									 uint16_t encoding, void* the_project,
									 uint8_t project_help, uint8_t verbose)
{
	if (range_is_null_or_empty(path_to_build_file) ||
		range_is_null_or_empty(current_directory) ||
		NULL == the_project)
	{
		return 0;
	}

	struct buffer* content = buffer_buffer_data(the_project, CONTENT_POSITION);;

	if (!path_get_full_path(
			current_directory->start, current_directory->finish,
			path_to_build_file->start, path_to_build_file->finish,
			content) ||
		!buffer_push_back(content, 0))
	{
		return 0;
	}

	const uint8_t* path = buffer_data(content, 0);

	if (!file_exists(path))
	{
		return 0;
	}

	if (!property_set_by_name(buffer_buffer_data(the_project, PRIVATE_PROPERTIES),
							  project_attributes[BUILD_FILE_POSITION],
							  project_attributes_lengths[BUILD_FILE_POSITION],
							  path, buffer_size(content) - 1,
							  property_value_is_byte_array,
							  0, 1, 1, verbose))
	{
		return 0;
	}

	if (!buffer_resize(content, 0) ||
		!load_file_to_buffer(path, encoding, content, verbose))
	{
		return 0;
	}

	if (!project_load(the_project, project_help, verbose))
	{
		return 0;
	}

	return 1;
}

uint8_t project_evaluate_default_target(void* the_project, uint8_t verbose)
{
	if (NULL == the_project)
	{
		return 0;
	}

	const void* the_property = NULL;

	if (project_get_default_target(the_project, &the_property, verbose))
	{
		struct buffer property_value;
		SET_NULL_TO_BUFFER(property_value);

		if (!property_get_by_pointer(the_property, &property_value))
		{
			buffer_release(&property_value);
			return 0;
		}

		if (!interpreter_actualize_property_value(
				the_project, NULL, property_get_id_of_get_value_function(),
				the_property, 0, &property_value, verbose))
		{
			buffer_release(&property_value);
			return 0;
		}

		struct range target_name;

		BUFFER_TO_RANGE(target_name, &property_value);

		if (!target_evaluate_by_name(the_project, &target_name, verbose))
		{
			buffer_release(&property_value);
			return 0;
		}

		buffer_release(&property_value);
	}

	return 1;
}

void project_clear(void* the_project)
{
	if (NULL == the_project)
	{
		return;
	}

	struct buffer* private_properties = buffer_buffer_data(the_project, PRIVATE_PROPERTIES);

	struct buffer* content = buffer_buffer_data(the_project, CONTENT_POSITION);

	struct buffer* properties = buffer_buffer_data(the_project, PROPERTIES_POSITION);

	struct buffer* targets = buffer_buffer_data(the_project, TARGETS_POSITION);

	struct buffer* modules = buffer_buffer_data(the_project, MODULES_POSITION);

	property_release_inner(private_properties);

	buffer_resize(private_properties, 0);

	buffer_resize(content, 0);

	property_release_inner(properties);

	buffer_resize(properties, 0);

	target_release_inner(targets);

	buffer_resize(targets, 0);

	load_tasks_unload(modules);

	buffer_resize(modules, 0);
}

void project_unload(void* the_project)
{
	if (NULL == the_project)
	{
		return;
	}

	struct buffer* private_properties = buffer_buffer_data(the_project, PRIVATE_PROPERTIES);

	struct buffer* content = buffer_buffer_data(the_project, CONTENT_POSITION);

	struct buffer* properties = buffer_buffer_data(the_project, PROPERTIES_POSITION);

	struct buffer* targets = buffer_buffer_data(the_project, TARGETS_POSITION);

	struct buffer* modules = buffer_buffer_data(the_project, MODULES_POSITION);

	property_release(private_properties);

	buffer_release(content);

	property_release(properties);

	target_release(targets);

	load_tasks_unload(modules);

	buffer_release(modules);

	buffer_release(the_project);
}

ptrdiff_t project_get_source_offset(const void* the_project, const uint8_t* cursor)
{
	if (NULL == the_project ||
		NULL == cursor)
	{
		return 0;
	}

	const struct buffer* content = buffer_buffer_data(the_project, CONTENT_POSITION);
	const uint8_t* start = buffer_data(content, 0);

	if (NULL == start)
	{
		return 0;
	}

	const uint8_t* finish = start + buffer_size(content);

	if (range_in_parts_is_null_or_empty(start, finish))
	{
		return 0;
	}

	if (start < cursor && cursor < finish)
	{
		return cursor - start;
	}

	return 0;
}

uint8_t project_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	return common_get_attributes_and_arguments_for_task(
			   project_attributes, project_attributes_lengths,
			   project_attributes_count, task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t project_evaluate_task(void* the_project, const struct buffer* task_arguments, uint8_t verbose)
{
	if (NULL == the_project || NULL == task_arguments)
	{
		return 0;
	}

	for (uint8_t i = 0, count = project_attributes_count; i < count; ++i)
	{
		const struct buffer* attribute = buffer_buffer_data(task_arguments, i);

		if (!attribute)
		{
			return 0;
		}

		if (buffer_size(attribute))
		{
			if (!property_set_by_name(buffer_buffer_data(the_project, PRIVATE_PROPERTIES),
									  project_attributes[i], project_attributes_lengths[i],
									  buffer_data(attribute, 0), buffer_size(attribute),
									  property_value_is_byte_array,
									  1, 1, 1, verbose))
			{
				return 0;
			}
		}
	}

	const struct buffer* base_dir = buffer_buffer_data(task_arguments, BASE_DIR_POSITION);

	if (!base_dir)
	{
		return 0;
	}

	if (!buffer_size(base_dir))
	{
		struct buffer build_file;
		SET_NULL_TO_BUFFER(build_file);
		void* the_property = NULL;

		if (project_private_property_exists(the_project,
											project_attributes[BUILD_FILE_POSITION],
											project_attributes_lengths[BUILD_FILE_POSITION],
											&the_property, verbose) &&
			property_get_by_pointer(the_property, &build_file) &&
			buffer_size(&build_file))
		{
			struct range base_directory;

			if (!path_get_directory_name(
					buffer_data(&build_file, 0),
					buffer_data(&build_file, 0) + buffer_size(&build_file), &base_directory))
			{
				buffer_release(&build_file);
				return 0;
			}

			if (!property_set_by_name(buffer_buffer_data(the_project, PRIVATE_PROPERTIES),
									  project_attributes[BASE_DIR_POSITION],
									  project_attributes_lengths[BASE_DIR_POSITION],
									  base_directory.start, range_size(&base_directory),
									  property_value_is_byte_array,
									  0, 1, 1, verbose))
			{
				buffer_release(&build_file);
				return 0;
			}
		}

		buffer_release(&build_file);
	}

	return 1;
}

static const uint8_t* project_function_str[] =
{
	(const uint8_t*)"get-base-directory",
	(const uint8_t*)"get-buildfile-path",
	(const uint8_t*)"get-buildfile-uri",
	(const uint8_t*)"get-default-target",
	(const uint8_t*)"get-name",
	(const uint8_t*)"version",
	(const uint8_t*)"current-directory"
};

enum project_function
{
	get_base_directory, get_buildfile_path,
	get_buildfile_uri, get_default_target,
	get_name, version_, current_directory_,
	UNKNOWN_PROJECT_FUNCTION
};

uint8_t project_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	return common_string_to_enum(name_start, name_finish, project_function_str, UNKNOWN_PROJECT_FUNCTION);
}

uint8_t project_exec_function(const void* the_project,
							  uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							  const void** the_property, struct buffer* output, uint8_t verbose)
{
	if (UNKNOWN_PROJECT_FUNCTION <= function ||
		NULL == arguments ||
		arguments_count ||
		NULL == output)
	{
		return 0;
	}

	switch (function)
	{
		case get_base_directory:
			return project_get_base_directory(the_project, the_property, verbose) ?
				   property_get_by_pointer(*the_property, output) : 1;

		case get_buildfile_path:
			return project_get_buildfile_path(the_project, the_property, verbose) ?
				   property_get_by_pointer(*the_property, output) : 1;

		case get_buildfile_uri:
			return project_get_buildfile_path(the_project, the_property, verbose) ?
				   project_get_buildfile_uri(*the_property, output, verbose) : 1;

		case get_default_target:
			return project_get_default_target(the_project, the_property, verbose) ?
				   property_get_by_pointer(*the_property, output) : 1;

		case get_name:
			return project_get_name(the_project, the_property, verbose) ?
				   property_get_by_pointer(*the_property, output) : 1;

		case UNKNOWN_PROJECT_FUNCTION:
		default:
			break;
	}

	return 0;
}

uint8_t program_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							  struct buffer* output)
{
	if (UNKNOWN_PROJECT_FUNCTION <= function ||
		NULL == arguments ||
		arguments_count ||
		NULL == output)
	{
		return 0;
	}

	switch (function)
	{
		case version_:
		{
			static const uint8_t* program_version = (const uint8_t*)(PROGRAM_VERSION);
			struct Version version;

			if (!version_parse(program_version, program_version + PROGRAM_VERSION_LENGTH, &version))
			{
				break;
			}

			return version_to_string(&version, output);
		}

		case current_directory_:
			return path_get_directory_for_current_process(output);

		case UNKNOWN_PROJECT_FUNCTION:
		default:
			break;
	}

	return 0;
}

#define PROGRAM_BUILD_FILE_POSITION		0
#define PROGRAM_ENCODING_POSITION		1
#define PROGRAM_INHERIT_ALL_POSITION	2
#define PROGRAM_TARGET_POSITION			3
#define PROGRAM_INHERIT_MODULES_POSITION	4

static const uint8_t* program_attributes[] =
{
	(const uint8_t*)"buildfile",
	(const uint8_t*)"encoding",
	(const uint8_t*)"inheritall",
	(const uint8_t*)"target",
	(const uint8_t*)"inheritmodules",
};

static const uint8_t program_attributes_lengths[] = { 9, 8, 10, 6, 14 };

uint8_t program_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	return common_get_attributes_and_arguments_for_task(
			   program_attributes, program_attributes_lengths,
			   COUNT_OF(program_attributes),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t program_get_property(
	const void* the_project, const void* the_target,
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	struct buffer* task_arguments,
	const uint8_t* attributes_start, const uint8_t* attributes_finish,
	struct buffer* properties, uint8_t verbose)
{
	if (NULL == the_project ||
		NULL == task_attributes ||
		NULL == task_attributes_lengths ||
		NULL == task_arguments ||
		NULL == properties)
	{
		return 0;
	}

	uint8_t task_attributes_count = 0;
	buffer_release_inner_buffers(task_arguments);
	/**/
	task_attributes_count = interpreter_prepare_attributes_and_arguments_for_property_task(
								the_project, the_target, task_attributes, task_attributes_lengths,
								&task_attributes_count, task_arguments, attributes_start, attributes_finish, verbose);

	if (task_attributes_count)
	{
		task_attributes_count = property_evaluate_task(NULL, properties, task_arguments, verbose);
	}

	buffer_release_inner_buffers(task_arguments);
	return task_attributes_count;
}

uint8_t program_get_properties(
	const void* the_project,
	const void* the_target,
	const struct buffer* properties_elements,
	struct buffer* properties,
	uint8_t is_root, uint8_t verbose)
{
	if (NULL == the_project ||
		NULL == properties_elements ||
		NULL == properties)
	{
		return 0;
	}

	if (!buffer_size(properties_elements))
	{
		return 1;
	}

	struct buffer property_range;

	SET_NULL_TO_BUFFER(property_range);

	if (is_root && !range_from_string((const uint8_t*)"property\0", 9, 1, &property_range))
	{
		buffer_release(&property_range);
		return 0;
	}

	const uint8_t** task_attributes = NULL;
	const uint8_t* task_attributes_lengths = NULL;
	/**/
	struct buffer attributes;
	SET_NULL_TO_BUFFER(attributes);
	/**/
	ptrdiff_t i = 0;
	struct range* element = NULL;
	/**/
	uint8_t returned = 1;

	while (NULL != (element = buffer_range_data(properties_elements, i++)))
	{
		uint8_t skip = 0;
		const uint8_t* finish_of_attributes = xml_get_tag_finish_pos(element->start, element->finish);
		buffer_release_inner_buffers(&attributes);

		if (!interpreter_is_xml_tag_should_be_skip_by_if_or_unless(
				the_project, the_target, element->start,
				finish_of_attributes, &skip, &attributes, verbose))
		{
			returned = 0;
			break;
		}

		if (skip)
		{
			continue;
		}

		buffer_release_inner_buffers(&attributes);
		uint8_t fail_on_error = 1;

		if (!interpreter_get_xml_tag_attribute_values(
				the_project, the_target, element->start,
				finish_of_attributes, &fail_on_error, &verbose, &attributes, verbose))
		{
			returned = 0;
			break;
		}

		buffer_release_inner_buffers(&attributes);

		if (!buffer_resize(&attributes, 0))
		{
			returned = 0;
			break;
		}

		if (is_root)
		{
			if (xml_get_sub_nodes_elements(element->start, element->finish, &property_range, &attributes))
			{
				if (!program_get_properties(the_project, the_target, &attributes, properties, 0, verbose))
				{
					if (!fail_on_error)
					{
						if (!buffer_resize(&attributes, 0))
						{
							returned = 0;
							break;
						}

						returned = FAIL_WITH_OUT_ERROR;
						continue;
					}

					returned = 0;
					break;
				}

				if (!buffer_resize(&attributes, 0))
				{
					returned = 0;
					break;
				}
			}
		}
		else
		{
			if (!program_get_property(
					the_project, the_target,
					&task_attributes, &task_attributes_lengths, &attributes,
					element->start, finish_of_attributes, properties, verbose))
			{
				if (!fail_on_error)
				{
					returned = FAIL_WITH_OUT_ERROR;
					continue;
				}

				returned = 0;
				break;
			}
		}
	}

	buffer_release(&property_range);
	buffer_release_with_inner_buffers(&attributes);
	/**/
	return returned;
}

uint8_t program_evaluate_task(const void* the_project, const void* the_target,
							  struct buffer* task_arguments, const uint8_t* attributes_finish,
							  const uint8_t* element_finish, uint8_t verbose)
{
	if (NULL == the_project ||
		NULL == task_arguments)
	{
		return 0;
	}

	const struct buffer* build_file_in_a_buffer = buffer_buffer_data(task_arguments, PROGRAM_BUILD_FILE_POSITION);

	if (!buffer_size(build_file_in_a_buffer))
	{
		return 1;
	}

	uint16_t encoding = UTF8;
	struct buffer* encoding_in_a_buffer = buffer_buffer_data(task_arguments, PROGRAM_ENCODING_POSITION);

	if (buffer_size(encoding_in_a_buffer))
	{
		encoding = load_file_get_encoding(encoding_in_a_buffer);

		if (FILE_ENCODING_UNKNOWN == encoding)
		{
			return 0;
		}
	}

	if (!buffer_resize(encoding_in_a_buffer, 0))
	{
		return 0;
	}

	uint8_t inherit_properties = 1;
	struct buffer* inherit_all_in_a_buffer = buffer_buffer_data(task_arguments, PROGRAM_INHERIT_ALL_POSITION);

	if (buffer_size(inherit_all_in_a_buffer))
	{
		if (!bool_parse(buffer_data(inherit_all_in_a_buffer, 0), buffer_size(inherit_all_in_a_buffer),
						&inherit_properties))
		{
			return 0;
		}
	}

	if (!buffer_resize(inherit_all_in_a_buffer, 0))
	{
		return 0;
	}

	uint8_t inherit_modules = 1;
	struct buffer* inherit_modules_in_a_buffer = buffer_buffer_data(task_arguments,
			PROGRAM_INHERIT_MODULES_POSITION);

	if (buffer_size(inherit_modules_in_a_buffer))
	{
		if (!bool_parse(buffer_data(inherit_modules_in_a_buffer, 0), buffer_size(inherit_modules_in_a_buffer),
						&inherit_modules))
		{
			return 0;
		}
	}

	struct buffer properties;

	SET_NULL_TO_BUFFER(properties);

	if (!range_from_string((const uint8_t*)"properties\0", 11, 1, encoding_in_a_buffer))
	{
		property_release(&properties);
		return 0;
	}

	if (xml_get_sub_nodes_elements(attributes_finish, element_finish, encoding_in_a_buffer,
								   inherit_all_in_a_buffer))
	{
		if (!program_get_properties(the_project, the_target, inherit_all_in_a_buffer, &properties, 1, verbose))
		{
			property_release(&properties);
			return 0;
		}
	}

	if (!buffer_resize(encoding_in_a_buffer, 0) ||
		!buffer_resize(inherit_all_in_a_buffer, 0))
	{
		property_release(&properties);
		return 0;
	}

	if (!common_get_attributes_and_arguments_for_task(NULL, NULL, COUNT_OF_POSITIONS, NULL, NULL, NULL,
			inherit_all_in_a_buffer))
	{
		property_release(&properties);
		return 0;
	}

	if (!property_add_at_project(inherit_all_in_a_buffer, &properties, verbose))
	{
		property_release(&properties);
		project_unload(inherit_all_in_a_buffer);
		return 0;
	}

	property_release(&properties);

	if (inherit_properties)
	{
		const struct buffer* current_project_properties = buffer_buffer_data(the_project, PROPERTIES_POSITION);

		if (!property_add_at_project(inherit_all_in_a_buffer, current_project_properties, verbose))
		{
			project_unload(inherit_all_in_a_buffer);
			return 0;
		}
	}

	if (inherit_modules)
	{
		const struct buffer* current_project_modules = buffer_buffer_data(the_project, MODULES_POSITION);
		struct buffer* new_project_modules = buffer_buffer_data(inherit_all_in_a_buffer, MODULES_POSITION);

		if (!load_tasks_copy_modules_with_out_objects(current_project_modules, new_project_modules))
		{
			project_unload(inherit_all_in_a_buffer);
			return 0;
		}
	}

	struct range build_file;

	BUFFER_TO_RANGE(build_file, build_file_in_a_buffer);

	if (!project_get_current_directory(the_project, the_target, encoding_in_a_buffer, 0, verbose))
	{
		project_unload(inherit_all_in_a_buffer);
		return 0;
	}

	struct range current_directory;

	BUFFER_TO_RANGE(current_directory, encoding_in_a_buffer);

	inherit_properties = project_load_from_build_file(
							 &build_file, &current_directory, encoding, inherit_all_in_a_buffer, 0, verbose);

	if (!inherit_properties)
	{
		project_unload(inherit_all_in_a_buffer);
		return 0;
	}

	const struct buffer* target_name_in_a_buffer = buffer_buffer_data(task_arguments, PROGRAM_TARGET_POSITION);

	if (buffer_size(target_name_in_a_buffer))
	{
		BUFFER_TO_RANGE(current_directory, target_name_in_a_buffer);
		inherit_properties = target_evaluate_by_name(inherit_all_in_a_buffer, &current_directory, verbose);
	}
	else
	{
		inherit_properties = project_evaluate_default_target(inherit_all_in_a_buffer, verbose);
	}

	project_unload(inherit_all_in_a_buffer);
	return inherit_properties;
}
