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

uint8_t project_property_exists(const void* project,
								const uint8_t* property_name, uint8_t property_name_length,
								void** the_property, uint8_t verbose)
{
	if (NULL == project || NULL == property_name || 0 == property_name_length)
	{
		return 0;
	}

	(void)verbose;/*TODO*/
	/*TODO: validate project pointer.*/
	const struct project* pro = (const struct project*)project;
	return property_exists(&pro->properties, property_name, property_name_length, the_property);
}

uint8_t project_property_set_value(void* project,
								   const uint8_t* property_name, uint8_t property_name_length,
								   const uint8_t* property_value, ptrdiff_t property_value_length,
								   uint8_t dynamic, uint8_t over_write,
								   uint8_t read_only, uint8_t verbose)
{
	if (NULL == project || NULL == property_name || 0 == property_name_length)
	{
		return 0;
	}

	struct project* pro = (struct project*)project;

	return property_set_by_name(&pro->properties, property_name, property_name_length,
								property_value, property_value_length, property_value_is_byte_array,
								dynamic, over_write, read_only, verbose);
}

uint8_t project_property_get_by_name(
	const void* project, const uint8_t* property_name, uint8_t property_name_length, struct buffer* output,
	uint8_t verbose)
{
	if (NULL == output)
	{
		return 0;
	}

	void* the_property = NULL;

	if (!project_property_exists(project, property_name, property_name_length, &the_property, verbose))
	{
		return 0;
	}

	return property_get_by_pointer(the_property, output);
}

uint8_t project_target_new(void* project,
						   const struct range* name, const struct range* depends, const struct range* content)
{
	if (NULL == project ||
		range_is_null_or_empty(name))
	{
		return 0;
	}

	struct project* pro = (struct project*)project;

	if (target_exists(project, name->start, (uint8_t)range_size(name)))
	{
		return 0;
	}

	return target_new(name, depends, content, &(pro->targets));
}

uint8_t project_target_exists(const void* project, const uint8_t* name, uint8_t name_length)
{
	if (NULL == project || NULL == name || 0 == name_length)
	{
		return 0;
	}

	const struct project* pro = (const struct project*)project;
	return target_exists(&pro->targets, name, name_length);
}

uint8_t project_target_has_executed(const void* project, const uint8_t* name, uint8_t name_length)
{
	if (NULL == project || NULL == name || 0 == name_length)
	{
		return 0;
	}

	const struct project* pro = (const struct project*)project;
	return target_has_executed(&pro->targets, name, name_length);
}

uint8_t project_get_base_directory(const void* project, const void** the_property)
{
	uint8_t verbose = 0;/*TODO*/

	if (NULL == the_property)
	{
		return 0;
	}

	void* prop = NULL;
	const uint8_t returned = project_property_exists(project,
							 project_properties[BASE_DIR_POSITION],
							 project_properties_lengths[BASE_DIR_POSITION], &prop, verbose);
	(*the_property) = returned ? prop : NULL;
	return returned;
}

uint8_t project_get_buildfile_path(const void* project, const void** the_property)
{
	uint8_t verbose = 0;/*TODO*/

	if (NULL == the_property)
	{
		return 0;
	}

	void* prop = NULL;
	const uint8_t returned = project_property_exists(project,
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

uint8_t project_get_default_target(const void* project, const void** the_property)
{
	uint8_t verbose = 0;/*TODO*/

	if (NULL == the_property)
	{
		return 0;
	}

	void* prop = NULL;
	const uint8_t returned = project_property_exists(project,
							 project_properties[DEFAULT_POSITION],
							 project_properties_lengths[DEFAULT_POSITION], &prop, verbose);
	(*the_property) = returned ? prop : NULL;
	return returned;
}

uint8_t project_get_name(const void* project, const void** the_property)
{
	uint8_t verbose = 0;/*TODO*/

	if (NULL == the_property)
	{
		return 0;
	}

	void* prop = NULL;
	const uint8_t returned = project_property_exists(project,
							 project_properties[NAME_POSITION],
							 project_properties_lengths[NAME_POSITION], &prop, verbose);
	(*the_property) = returned ? prop : NULL;
	return returned;
}

uint8_t project_new(void** project)
{
	/*NOTE: provide multi project support on a future release.*/
	if (NULL == project || NULL != (*project))
	{
		return 0;
	}

	SET_NULL_TO_BUFFER(gProject.properties);
	SET_NULL_TO_BUFFER(gProject.targets);
	(*project) = &gProject;
	return 1;
}

uint8_t project_load_from_content(const uint8_t* content_start, const uint8_t* content_finish,
								  void* project, uint8_t verbose)
{
	if (range_in_parts_is_null_or_empty(content_start, content_finish) ||
		NULL == project)
	{
		return 0;
	}

	struct buffer elements;

	SET_NULL_TO_BUFFER(elements);

	if (1 != xml_get_sub_nodes_elements(content_start, content_finish, NULL, &elements))
	{
		buffer_release(&elements);
		return 0;
	}

	struct range* element = NULL;

	if (NULL == (element = buffer_range_data(&elements, 0)))
	{
		buffer_release(&elements);
		return 0;
	}

	struct range tag_name;

	if (!xml_get_tag_name(element->start, element->finish, &tag_name))
	{
		buffer_release(&elements);
		return 0;
	}

	const uint8_t root_task_id = interpreter_get_task(tag_name.start, tag_name.finish);
	const uint8_t* element_finish = element->finish;

	if (!interpreter_evaluate_task(project, NULL, root_task_id,
								   tag_name.finish, element_finish, verbose))
	{
		buffer_release(&elements);
		return 0;
	}

	if (!buffer_resize(&elements, 0))
	{
		buffer_release(&elements);
		return 0;
	}

	if (xml_get_sub_nodes_elements(tag_name.finish, element_finish, NULL, &elements) &&
		!interpreter_evaluate_tasks(project, NULL, &elements, verbose))
	{
		buffer_release(&elements);
		return 0;
	}

	buffer_release(&elements);
	return 1;
}

uint8_t project_load_from_build_file(const struct range* path_to_build_file,
									 const struct range* current_directory,
									 uint16_t encoding, void* project, uint8_t verbose)
{
	if (range_is_null_or_empty(path_to_build_file) ||
		range_is_null_or_empty(current_directory) ||
		NULL == project)
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

	if (!project_property_set_value(project,
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

	if (!project_load_from_content(path, path + buffer_size(&content), project, verbose))
	{
		buffer_release(&content);
		return 0;
	}

	buffer_release(&content);
	return 1;
}

void project_unload(void* project)
{
	if (NULL == project || &gProject != project)
	{
		return;
	}

	struct project* pro = (struct project*)project;

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

uint8_t project_evaluate_task(void* project, const struct buffer* task_arguments, uint8_t verbose)
{
	if (NULL == project || NULL == task_arguments)
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
			if (!project_property_set_value(project,
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

		if (project_property_get_by_name(project,
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

			if (!project_property_set_value(project,
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

uint8_t project_exec_function(const void* project,
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
			return project_get_base_directory(project, the_property) && property_get_by_pointer(*the_property, output);

		case get_buildfile_path:
			return project_get_buildfile_path(project, the_property) && property_get_by_pointer(*the_property, output);

		case get_buildfile_uri:
			return project_get_buildfile_path(project, the_property) && project_get_buildfile_uri(*the_property, output);

		case get_default_target:
			return project_get_default_target(project, the_property) && property_get_by_pointer(*the_property, output);

		case get_name:
			return project_get_name(project, the_property) && property_get_by_pointer(*the_property, output);

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
