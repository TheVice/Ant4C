/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "project.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "file_system.h"
#include "interpreter.h"
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

static const uint8_t* project_str = (const uint8_t*)"project";
#define PROJECT_STR_LENGTH		7

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

uint8_t project_property_get_pointer(const void* project,
									 const uint8_t* property_name, uint8_t property_name_length,
									 void** the_property)
{
	if (NULL == project || NULL == property_name || 0 == property_name_length)
	{
		return 0;
	}

	/*TODO: validate project pointer.*/
	const struct project* pro = (const struct project*)project;
	return property_exists(&pro->properties, property_name, property_name_length, the_property);
}

uint8_t project_property_set_value(void* project,
								   const uint8_t* property_name, uint8_t property_name_length,
								   const uint8_t* property_value, ptrdiff_t property_value_length,
								   uint8_t dynamic, uint8_t overwrite,
								   uint8_t readonly, uint8_t verbose)
{
	if (NULL == project || NULL == property_name || 0 == property_name_length)
	{
		return 0;
	}

	struct project* pro = (struct project*)project;

	return property_set_by_name(&pro->properties, property_name, property_name_length,
								property_value, property_value_length, property_value_is_byte_array,
								dynamic, overwrite, readonly, verbose);
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

uint8_t project_get_base_directory(const void* project, const void** the_property)
{
	if (NULL == the_property)
	{
		return 0;
	}

	void* prop = NULL;
	const uint8_t returned = project_property_get_pointer(project,
							 project_properties[BASE_DIR_POSITION],
							 project_properties_lengths[BASE_DIR_POSITION], &prop);
	(*the_property) = returned ? prop : NULL;
	return returned;
}

uint8_t project_get_buildfile_path(const void* project, const void** the_property)
{
	if (NULL == the_property)
	{
		return 0;
	}

	void* prop = NULL;
	const uint8_t returned = project_property_get_pointer(project,
							 project_properties[BUILD_FILE_POSITION],
							 project_properties_lengths[BUILD_FILE_POSITION], &prop);
	(*the_property) = returned ? prop : NULL;
	return returned;
}
#if 0
uint8_t project_get_buildfile_uri(const void* project, const void* target, struct buffer* build_file_uri)
{
	if (NULL == project || NULL == build_file_uri)
	{
		return 0;
	}

#if defined(_WIN32)
	const ptrdiff_t size = buffer_size(build_file_uri);
#endif

	if (!common_append_string_to_buffer((const uint8_t*)"file:///", build_file_uri))
	{
		return 0;
	}

	if (!project_get_buildfile_path(project, target, build_file_uri))
	{
		return 0;
	}

#if defined(_WIN32)
	uint8_t* path_start = buffer_data(build_file_uri, size);
	uint8_t* path_finish = (buffer_data(build_file_uri, 0) + buffer_size(build_file_uri));
	return cygpath_get_unix_path(path_start, path_finish);
#else
	return 1;
#endif
}
#endif
uint8_t project_get_default_target(const void* project, const void** the_property)
{
	if (NULL == the_property)
	{
		return 0;
	}

	void* prop = NULL;
	const uint8_t returned = project_property_get_pointer(project,
							 project_properties[DEFAULT_POSITION],
							 project_properties_lengths[DEFAULT_POSITION], &prop);
	(*the_property) = returned ? prop : NULL;
	return returned;
}

uint8_t project_get_name(const void* project, const void** the_property)
{
	if (NULL == the_property)
	{
		return 0;
	}

	void* prop = NULL;
	const uint8_t returned = project_property_get_pointer(project,
							 project_properties[NAME_POSITION],
							 project_properties_lengths[NAME_POSITION], &prop);
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

const uint8_t* project_is_in_element(const struct range* element)
{
	struct range tag_name;
	tag_name.start = tag_name.finish = NULL;

	if (!xml_get_tag_name(element->start, element->finish, &tag_name) ||
		!string_equal(tag_name.start, tag_name.finish, project_str, project_str + PROJECT_STR_LENGTH))
	{
		return NULL;
	}

	return tag_name.finish;
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

	if (1 != xml_get_sub_nodes_elements(content_start, content_finish, &elements))
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

	const uint8_t* tag_name_finish = NULL;

	if (NULL == (tag_name_finish = project_is_in_element(element)))
	{
		buffer_release(&elements);
		return 0;
	}

	const uint8_t project_task_id = interpreter_get_task(project_str, project_str + PROJECT_STR_LENGTH);
	const uint8_t* element_finish = element->finish;

	if (!interpreter_evaluate_task(project, NULL, project_task_id,
								   tag_name_finish, element_finish))
	{
		buffer_release(&elements);
		return 0;
	}

	if (!buffer_resize(&elements, 0))
	{
		buffer_release(&elements);
		return 0;
	}

	if (xml_get_sub_nodes_elements(tag_name_finish, element_finish, &elements) &&
		!interpreter_evaluate_tasks(project, NULL, &elements, verbose))
	{
		buffer_release(&elements);
		return 0;
	}

	buffer_release(&elements);
	return 1;
}

uint8_t project_load_from_build_file(const uint8_t* path_to_build_file, void* project, uint8_t verbose)
{
	ptrdiff_t path_length = 0;

	if (NULL == path_to_build_file ||
		NULL == project ||
		0 == (path_length = common_count_bytes_until(path_to_build_file, 0)))
	{
		return 0;
	}

	struct buffer content;

	SET_NULL_TO_BUFFER(content);

	if (!path_get_directory_for_current_process(&content))
	{
		buffer_release(&content);
		return 0;
	}

	ptrdiff_t addition_path_length = buffer_size(&content);

	if (!buffer_append(&content, NULL, addition_path_length + path_length + 2) ||
		!buffer_resize(&content, addition_path_length))
	{
		buffer_release(&content);
		return 0;
	}

	if (!path_get_full_path(buffer_data(&content, 0), buffer_data(&content, 0) + addition_path_length,
							path_to_build_file, path_to_build_file + path_length, &content))
	{
		buffer_release(&content);
		return 0;
	}

	path_length = buffer_size(&content) - addition_path_length;
	uint8_t* dst = buffer_data(&content, 0);
	const uint8_t* src = buffer_data(&content, addition_path_length);
	MEM_CPY(dst, src, path_length);

	if (!buffer_resize(&content, path_length) ||
		!buffer_push_back(&content, 0))
	{
		buffer_release(&content);
		return 0;
	}

	if (!file_exists(buffer_data(&content, 0)))
	{
		buffer_release(&content);
		return 0;
	}

	path_length = buffer_size(&content);

	if (!project_property_set_value(project,
									project_properties[BUILD_FILE_POSITION],
									project_properties_lengths[BUILD_FILE_POSITION],
									buffer_data(&content, 0), path_length,
									0, 1, 1, verbose))
	{
		buffer_release(&content);
		return 0;
	}

	if (!read_file(buffer_data(&content, 0), &content))
	{
		buffer_release(&content);
		return 0;
	}

	if (!project_load_from_content(buffer_data(&content, 0) + path_length,
								   buffer_data(&content, 0) + buffer_size(&content),
								   project, verbose))
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
	if (NULL == task_attributes ||
		NULL == task_attributes_lengths ||
		NULL == task_attributes_count ||
		NULL == task_arguments)
	{
		return 0;
	}

	*task_attributes = project_attributes;
	*task_attributes_lengths = project_attributes_lengths;
	*task_attributes_count = COUNT_OF(project_attributes_lengths);

	if (!buffer_resize(task_arguments, 0) ||
		!buffer_append_buffer(task_arguments, NULL, *task_attributes_count))
	{
		return 0;
	}

	for (uint8_t i = 0, attributes_count = *task_attributes_count; i < attributes_count; ++i)
	{
		struct buffer* attribute = buffer_buffer_data(task_arguments, i);
		SET_NULL_TO_BUFFER(*attribute);
	}

	return 1;
}

uint8_t project_evaluate_task(void* project, const struct buffer* task_arguments)
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
											1, 1, 1, /*verbose*/0))
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

		if (property_get_by_name(project,
								 project_properties[BUILD_FILE_POSITION],
								 project_properties_lengths[BUILD_FILE_POSITION], &build_file) &&
			buffer_size(&build_file))
		{
			struct range base_directory;

			if (!path_get_directory_name(
					buffer_data(&build_file, 0),
					buffer_data(&build_file, 0) + buffer_size(&build_file), &base_directory))
			{
				return 0;
			}

			if (!project_property_set_value(project,
											project_properties[BASE_DIR_POSITION],
											project_properties_lengths[BASE_DIR_POSITION],
											base_directory.start, range_size(&base_directory),
											0, 1, 1, /*verbose*/0))
			{
				return 0;
			}
		}
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

		/*case get_buildfile_uri:
			return project_get_buildfile_uri(project, target, output);*/

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
