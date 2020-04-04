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
#include "load_file.h"
#include "path.h"
#include "property.h"
#include "range.h"
#include "string_unit.h"
#include "target.h"
#include "version.h"
#include "xml.h"

#include <string.h>

struct project
{
	struct buffer properties;
	struct buffer targets;
};

static struct project gProject;

static const uint8_t* project_attributes[] =
{
	(const uint8_t*)"name",
	(const uint8_t*)"default",
	(const uint8_t*)"basedir"
};

static const uint8_t* project_properties[] =
{
	(const uint8_t*)"project.name",
	(const uint8_t*)"project.default",
	(const uint8_t*)"project.basedir",
	(const uint8_t*)"project.buildfile"
};

static const uint8_t project_attributes_lengths[] = { 4, 7, 7 };
static const uint8_t project_properties_lengths[] = { 12, 15, 15, 17 };

#define NAME_POSITION			0
#define DEFAULT_POSITION		1
#define BASE_DIR_POSITION		2
#define BUILD_FILE_POSITION		3

uint8_t project_property_exists(const void* the_project,
								const uint8_t* property_name, uint8_t property_name_length,
								void** the_property, uint8_t verbose)
{
	if (NULL == the_project || NULL == property_name || 0 == property_name_length)
	{
		return 0;
	}

	(void)verbose;/*TODO*/
	/*TODO: validate project pointer.*/
	const struct project* pro = (const struct project*)the_project;
	return property_exists(&pro->properties, property_name, property_name_length, the_property);
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

	struct project* pro = (struct project*)the_project;

	return property_set_by_name(&pro->properties, property_name, property_name_length,
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

	struct project* pro = (struct project*)the_project;

	if (target_exists(&(pro->targets), target_name, target_name_length))
	{
		return 0;
	}

	return target_new(the_project,
					  target_name, target_name_length,
					  target_depends, target_depends_length,
					  description,
					  attributes_start, attributes_finish,
					  element_finish, &(pro->targets), verbose);
}

uint8_t project_target_get(const void* the_project, const uint8_t* name, uint8_t name_length,
						   void** the_target,
						   uint8_t verbose)
{
	(void)verbose;

	if (NULL == the_project || NULL == name || 0 == name_length || NULL == the_target)
	{
		return 0;
	}

	const struct project* pro = (const struct project*)the_project;
	return target_get(&pro->targets, name, name_length, the_target);
}

uint8_t project_target_exists(const void* the_project, const uint8_t* name, uint8_t name_length)
{
	if (NULL == the_project || NULL == name || 0 == name_length)
	{
		return 0;
	}

	const struct project* pro = (const struct project*)the_project;
	return target_exists(&pro->targets, name, name_length);
}

uint8_t project_target_has_executed(const void* the_project, const uint8_t* name, uint8_t name_length)
{
	if (NULL == the_project || NULL == name || 0 == name_length)
	{
		return 0;
	}

	const struct project* pro = (const struct project*)the_project;
	return target_has_executed(&pro->targets, name, name_length);
}

uint8_t project_get_base_directory(const void* the_project, const void** the_property)
{
	uint8_t verbose = 0;/*TODO*/

	if (NULL == the_property)
	{
		return 0;
	}

	void* prop = NULL;
	const uint8_t returned = project_property_exists(the_project,
							 project_properties[BASE_DIR_POSITION],
							 project_properties_lengths[BASE_DIR_POSITION], &prop, verbose);
	(*the_property) = returned ? prop : NULL;
	return returned;
}

uint8_t project_get_buildfile_path(const void* the_project, const void** the_property)
{
	uint8_t verbose = 0;/*TODO*/

	if (NULL == the_property)
	{
		return 0;
	}

	void* prop = NULL;
	const uint8_t returned = project_property_exists(the_project,
							 project_properties[BUILD_FILE_POSITION],
							 project_properties_lengths[BUILD_FILE_POSITION], &prop, verbose);
	(*the_property) = returned ? prop : NULL;
	return returned;
}

uint8_t project_get_buildfile_uri(const void* the_property, struct buffer* build_file_uri)
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

	return 1;
}

uint8_t project_get_default_target(const void* the_project, const void** the_property)
{
	uint8_t verbose = 0;/*TODO*/

	if (NULL == the_property)
	{
		return 0;
	}

	void* prop = NULL;
	const uint8_t returned = project_property_exists(the_project,
							 project_properties[DEFAULT_POSITION],
							 project_properties_lengths[DEFAULT_POSITION], &prop, verbose);
	(*the_property) = returned ? prop : NULL;
	return returned;
}

uint8_t project_get_name(const void* the_project, const void** the_property)
{
	uint8_t verbose = 0;/*TODO*/

	if (NULL == the_property)
	{
		return 0;
	}

	void* prop = NULL;
	const uint8_t returned = project_property_exists(the_project,
							 project_properties[NAME_POSITION],
							 project_properties_lengths[NAME_POSITION], &prop, verbose);
	(*the_property) = returned ? prop : NULL;
	return returned;
}

uint8_t project_new(void** the_project)
{
	/*NOTE: provide multi project support on a future release.*/
	if (NULL == the_project || NULL != (*the_project))
	{
		return 0;
	}

	SET_NULL_TO_BUFFER(gProject.properties);
	SET_NULL_TO_BUFFER(gProject.targets);
	(*the_project) = &gProject;
	return 1;
}

uint8_t project_load_from_content(const uint8_t* content_start, const uint8_t* content_finish,
								  void* the_project, uint8_t project_help, uint8_t verbose)
{
	if (range_in_parts_is_null_or_empty(content_start, content_finish) ||
		NULL == the_project)
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

	if (1 != xml_get_sub_nodes_elements(content_start, content_finish, &sub_nodes_names, &elements))
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

	struct range tag_name;

	if (!xml_get_tag_name(element->start, element->finish, &tag_name))
	{
		buffer_release(&elements);
		buffer_release(&sub_nodes_names);
		return 0;
	}

	const uint8_t root_task_id = interpreter_get_task(tag_name.start, tag_name.finish);
	const uint8_t* element_finish = element->finish;

	if (!interpreter_evaluate_task(the_project, NULL, root_task_id,
								   tag_name.finish, element_finish,
								   project_help, verbose))
	{
		buffer_release(&elements);
		buffer_release(&sub_nodes_names);
		return 0;
	}

	if (!buffer_resize(&elements, 0))
	{
		buffer_release(&elements);
		buffer_release(&sub_nodes_names);
		return 0;
	}

	if (xml_get_sub_nodes_elements(tag_name.finish, element_finish, &sub_nodes_names, &elements) &&
		!interpreter_evaluate_tasks(the_project, NULL, &elements, project_help, verbose))
	{
		buffer_release(&elements);
		buffer_release(&sub_nodes_names);
		return 0;
	}

	const void* the_property = NULL;

	if (project_get_default_target(the_project, &the_property))
	{
		if (!buffer_resize(&sub_nodes_names, 0))
		{
			buffer_release(&elements);
			buffer_release(&sub_nodes_names);
			return 0;
		}

		if (!property_get_by_pointer(the_property, &sub_nodes_names))
		{
			buffer_release(&elements);
			buffer_release(&sub_nodes_names);
			return 0;
		}

		if (!interpreter_actualize_property_value(
				the_project, NULL, property_get_id_of_get_value_function(),
				the_property, 0, &sub_nodes_names, verbose))
		{
			buffer_release(&elements);
			buffer_release(&sub_nodes_names);
			return 0;
		}

		void* the_target = NULL;

		if (!project_target_get(the_project,
								buffer_data(&sub_nodes_names, 0),
								(uint8_t)buffer_size(&sub_nodes_names), &the_target, verbose))
		{
			buffer_release(&elements);
			buffer_release(&sub_nodes_names);
			return 0;
		}

		if (!buffer_resize(&sub_nodes_names, 0))
		{
			buffer_release(&elements);
			buffer_release(&sub_nodes_names);
			return 0;
		}

		if (!target_evaluate(the_project, the_target, &sub_nodes_names, project_help, 1, verbose))
		{
			buffer_release(&elements);
			buffer_release(&sub_nodes_names);
			return 0;
		}
	}

	buffer_release(&elements);
	buffer_release(&sub_nodes_names);
	return 1;
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

	struct buffer content;

	SET_NULL_TO_BUFFER(content);

	if (!path_get_full_path(
			current_directory->start, current_directory->finish,
			path_to_build_file->start, path_to_build_file->finish,
			&content) ||
		!buffer_push_back(&content, 0))
	{
		buffer_release(&content);
		return 0;
	}

	const uint8_t* path = buffer_data(&content, 0);

	if (!file_exists(path))
	{
		buffer_release(&content);
		return 0;
	}

	if (!project_property_set_value(the_project,
									project_properties[BUILD_FILE_POSITION],
									project_properties_lengths[BUILD_FILE_POSITION],
									path, buffer_size(&content) - 1,
									0, 1, 1, verbose))
	{
		buffer_release(&content);
		return 0;
	}

	if (!buffer_resize(&content, 0) ||
		!load_file_to_buffer(path, encoding, &content, verbose))
	{
		buffer_release(&content);
		return 0;
	}

	path = buffer_data(&content, 0);

	if (!project_load_from_content(path, path + buffer_size(&content), the_project, project_help, verbose))
	{
		buffer_release(&content);
		return 0;
	}

	buffer_release(&content);
	return 1;
}

void project_unload(void* the_project)
{
	if (NULL == the_project || &gProject != the_project)
	{
		return;
	}

	struct project* pro = (struct project*)the_project;

	property_clear(&pro->properties);

	target_clear(&pro->targets);
}

uint8_t project_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	return common_get_attributes_and_arguments_for_task(
			   project_attributes, project_attributes_lengths,
			   COUNT_OF(project_attributes_lengths),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t project_evaluate_task(void* the_project, const struct buffer* task_arguments, uint8_t verbose)
{
	if (NULL == the_project || NULL == task_arguments)
	{
		return 0;
	}

	for (uint8_t i = 0, count = COUNT_OF(project_attributes_lengths); i < count; ++i)
	{
		const struct buffer* attribute = buffer_buffer_data(task_arguments, i);

		if (!attribute)
		{
			return 0;
		}

		if (buffer_size(attribute))
		{
			if (!project_property_set_value(the_project,
											project_properties[i], project_properties_lengths[i],
											buffer_data(attribute, 0), buffer_size(attribute),
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

		if (project_property_get_by_name(the_project,
										 project_properties[BUILD_FILE_POSITION],
										 project_properties_lengths[BUILD_FILE_POSITION], &build_file, verbose) &&
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

			if (!project_property_set_value(the_project,
											project_properties[BASE_DIR_POSITION],
											project_properties_lengths[BASE_DIR_POSITION],
											base_directory.start, range_size(&base_directory),
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
	get_name, version_, current_directory,
	UNKNOWN_PROJECT_FUNCTION
};

uint8_t project_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	return common_string_to_enum(name_start, name_finish, project_function_str, UNKNOWN_PROJECT_FUNCTION);
}

uint8_t project_exec_function(const void* the_project,
							  uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							  const void** the_property, struct buffer* output)
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
			return project_get_base_directory(the_project, the_property) &&
				   property_get_by_pointer(*the_property, output);

		case get_buildfile_path:
			return project_get_buildfile_path(the_project, the_property) &&
				   property_get_by_pointer(*the_property, output);

		case get_buildfile_uri:
			return project_get_buildfile_path(the_project, the_property) &&
				   project_get_buildfile_uri(*the_property, output);

		case get_default_target:
			return project_get_default_target(the_project, the_property) &&
				   property_get_by_pointer(*the_property, output);

		case get_name:
			return project_get_name(the_project, the_property) && property_get_by_pointer(*the_property, output);

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

		case current_directory:
			return path_get_directory_for_current_process(output);

		case UNKNOWN_PROJECT_FUNCTION:
		default:
			break;
	}

	return 0;
}
