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

static const uint8_t project_str[] = { 'p', 'r', 'o', 'j', 'e', 'c', 't' };
#define PROJECT_LENGTH COUNT_OF(project_str)

static const uint8_t description_str[] = { 'd', 'e', 's', 'c', 'r', 'i', 'p', 't', 'i', 'o', 'n' };
static const uint8_t property_str[] = { 'p', 'r', 'o', 'p', 'e', 'r', 't', 'y' };
static const uint8_t target_str[] = { 't', 'a', 'r', 'g', 'e', 't' };

#define DESCRIPTION_LENGTH COUNT_OF(description_str)
#define PROPERTY_LENGTH COUNT_OF(property_str)
#define TARGET_LENGTH COUNT_OF(target_str)

static const uint8_t project_name_property[] = { 'p', 'r', 'o', 'j', 'e', 'c', 't', '.', 'n', 'a', 'm', 'e' };
static const uint8_t project_default_property[] = { 'p', 'r', 'o', 'j', 'e', 'c', 't', '.', 'd', 'e', 'f', 'a', 'u', 'l', 't' };
static const uint8_t project_base_dir_property[] = { 'p', 'r', 'o', 'j', 'e', 'c', 't', '.', 'b', 'a', 's', 'e', 'd', 'i', 'r' };
static const uint8_t project_build_file_property[] = { 'p', 'r', 'o', 'j', 'e', 'c', 't', '.', 'b', 'u', 'i', 'l', 'd', 'f', 'i', 'l', 'e' };

#define PROJECT_NAME_LENGTH				COUNT_OF(project_name_property)
#define PROJECT_DEFAULT_LENGTH			COUNT_OF(project_default_property)
#define PROJECT_BASE_DIR_LENGTH			COUNT_OF(project_base_dir_property)
#define PROJECT_BUILD_FILE_LENGTH		COUNT_OF(project_build_file_property)

static const uint8_t project_name_tag[] = { 'n', 'a', 'm', 'e' };
static const uint8_t project_default_tag[] = { 'd', 'e', 'f', 'a', 'u', 'l', 't' };
static const uint8_t project_base_dir_tag[] = { 'b', 'a', 's', 'e', 'd', 'i', 'r' };

#define PROJECT_NAME_TAG_LENGTH			COUNT_OF(project_name_tag)
#define PROJECT_DEFAULT_TAG_LENGTH		COUNT_OF(project_default_tag)
#define PROJECT_BASE_DIR_TAG_LENGTH		COUNT_OF(project_base_dir_tag)

struct project
{
	struct buffer description;
	struct buffer properties;
	struct buffer targets;
};

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
	return property_get_pointer(&pro->properties, property_name, property_name_length, the_property);
}

uint8_t project_property_set_value(void* project, const void* target,
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

	return property_set_by_name(project, target, &pro->properties, property_name, property_name_length,
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

uint8_t project_get_base_directory(const void* project, const void* target, struct buffer* base_directory)
{
	return property_get_by_name(project, target, project_base_dir_property, PROJECT_BASE_DIR_LENGTH,
								base_directory);
}

uint8_t project_get_buildfile_path(const void* project, const void* target, struct buffer* build_file)
{
	return property_get_by_name(project, target, project_build_file_property, PROJECT_BUILD_FILE_LENGTH,
								build_file);
}

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

uint8_t project_get_default_target(const void* project, const void* target, struct buffer* default_target)
{
	return property_get_by_name(project, target, project_default_property, PROJECT_DEFAULT_LENGTH,
								default_target);
}

uint8_t project_get_name(const void* project, const void* target, struct buffer* project_name)
{
	return property_get_by_name(project, target, project_name_property, PROJECT_NAME_LENGTH,
								project_name);
}

static struct project gProject;

uint8_t project_new(void** project)
{
	/*NOTE: provide multi project support on a future release.*/
	if (NULL == project || NULL != (*project))
	{
		return 0;
	}

	SET_NULL_TO_BUFFER(gProject.description);
	SET_NULL_TO_BUFFER(gProject.properties);
	SET_NULL_TO_BUFFER(gProject.targets);
	(*project) = &gProject;
	return 1;
}

uint8_t project_add_properties(void* project, const void* target, const void* properties,
							   uint8_t verbose)
{
	if (NULL == project || NULL == properties)
	{
		return 0;
	}

	struct project* pro = (struct project*)project;

	return property_append(project, target, &pro->properties, properties, verbose);
}

uint8_t project_set_property_from_attribute_if_such_exists(
	struct project* project, const void* target,
	const uint8_t* attributes_start, const uint8_t* attributes_finish,
	const uint8_t* property_name, uint8_t property_name_length,
	const uint8_t* attribute_name, uint8_t attribute_name_length,
	uint8_t verbose)
{
	if (NULL == project ||
		NULL == attributes_start ||
		NULL == attributes_finish ||
		attributes_finish < attributes_start ||
		NULL == property_name ||
		0 == property_name_length ||
		NULL == attribute_name ||
		0 == attribute_name_length)
	{
		return 0;
	}

	struct range attribute_value;

	if (xml_get_attribute_value(attributes_start, attributes_finish,
								attribute_name, attribute_name_length,
								&attribute_value))
	{
		if (!property_set_by_name(
				project, target,
				&project->properties,
				property_name, property_name_length,
				attribute_value.start, range_size(&attribute_value),
				property_value_is_byte_array,
				0, 1, 1, verbose))
		{
			return 0;
		}
	}

	return 1;
}

const uint8_t* project_is_in_element(const struct range* element)
{
	struct range tag_name;
	tag_name.start = tag_name.finish = NULL;

	if (!xml_get_tag_name(element->start, element->finish, &tag_name) ||
		!string_equal(tag_name.start, tag_name.finish, project_str, project_str + PROJECT_LENGTH))
	{
		return NULL;
	}

	return tag_name.finish;
}

uint8_t project_set_base_directory(struct project* project, void* tatget,
								   const uint8_t* attributes_start,
								   const uint8_t* attributes_finish, uint8_t verbose)
{
	return project_set_property_from_attribute_if_such_exists(
			   project, tatget,
			   attributes_start, attributes_finish,
			   project_base_dir_property, PROJECT_BASE_DIR_LENGTH,
			   project_base_dir_tag, PROJECT_BASE_DIR_TAG_LENGTH, verbose);
}

uint8_t project_set_buildfile_path(void* project, const void* target,
								   const uint8_t* build_file, ptrdiff_t build_file_length, uint8_t verbose)
{
	return project_property_set_value(project, target, project_build_file_property, PROJECT_BUILD_FILE_LENGTH,
									  build_file, build_file_length, 0, 1, 1, verbose);
}

uint8_t project_set_default_target(struct project* project, void* tatget,
								   const uint8_t* attributes_start,
								   const uint8_t* attributes_finish, uint8_t verbose)
{
	return project_set_property_from_attribute_if_such_exists(
			   project, tatget,
			   attributes_start, attributes_finish,
			   project_default_property, PROJECT_DEFAULT_LENGTH,
			   project_default_tag, PROJECT_DEFAULT_TAG_LENGTH, verbose);
}

uint8_t project_set_name(struct project* project, void* tatget,
						 const uint8_t* attributes_start,
						 const uint8_t* attributes_finish, uint8_t verbose)
{
	return project_set_property_from_attribute_if_such_exists(
			   project, tatget,
			   attributes_start, attributes_finish,
			   project_name_property, PROJECT_NAME_LENGTH,
			   project_name_tag, PROJECT_NAME_TAG_LENGTH, verbose);
}

uint8_t project_load_properties_and_targets(struct project* project, struct buffer* elements, uint8_t verbose)
{
	ptrdiff_t i = 0;
	struct range* element = NULL;

	while (NULL != (element = buffer_range_data(elements, i++)))
	{
		struct range tag_name_or_content;

		if (!xml_get_tag_name(element->start, element->finish, &tag_name_or_content))
		{
			i = 0;
			break;
		}

		if (string_equal(tag_name_or_content.start, tag_name_or_content.finish,
						 description_str, description_str + DESCRIPTION_LENGTH))
		{
			if (xml_get_element_value(element, &tag_name_or_content) &&
				!buffer_append(&project->description,
							   tag_name_or_content.start,
							   range_size(&tag_name_or_content)))
			{
				i = 0;
				break;
			}

			continue;
		}

		uint8_t skip = 0;

		if (!interpreter_xml_tag_should_be_skip_by_if_or_unless(project, NULL, tag_name_or_content.finish,
				element->finish, &skip))
		{
			i = 0;
			break;
		}

		/*TODO: name of target to skip may be used for depends.*/
		if (skip)
		{
			continue;
		}

		if (string_equal(tag_name_or_content.start, tag_name_or_content.finish,
						 property_str, property_str + PROPERTY_LENGTH))
		{
			if (!property_set_from_xml_tag_record(project, NULL,
												  &project->properties,
												  tag_name_or_content.finish, element->finish, verbose))
			{
				i = 0;
				break;
			}

			continue;
		}
		else if (string_equal(tag_name_or_content.start, tag_name_or_content.finish, target_str,
							  target_str + TARGET_LENGTH))
		{
			if (!target_add_from_xml_tag_record(&project->targets,
												tag_name_or_content.finish, element->finish))
			{
				i = 0;
				break;
			}

			continue;
		}

		if (!interpreter_evaluate_task(project, NULL,
									   interpreter_get_task(tag_name_or_content.start, tag_name_or_content.finish),
									   tag_name_or_content.finish,
									   element->finish))
		{
			i = 0;
			break;
		}
	}

	return 0 < i;
}

uint8_t project_load_from_content(const uint8_t* content_start, const uint8_t* content_finish,
								  const uint8_t* base_dir, ptrdiff_t base_dir_length,
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

	struct project* pro = (struct project*)project;

	const uint8_t* element_finish = element->finish;

	const uint8_t* attributes_finish = xml_get_tag_finish_pos(tag_name_finish, element_finish);

	if (!project_set_name(pro, NULL, tag_name_finish, attributes_finish, verbose))
	{
		buffer_release(&elements);
		return 0;
	}

	if (!property_exists(&pro->properties, project_default_property, PROJECT_DEFAULT_LENGTH))
	{
		if (!project_set_default_target(pro, NULL, tag_name_finish, attributes_finish, verbose))
		{
			buffer_release(&elements);
			return 0;
		}
	}

	/*NOTE: path in tag may be not full - '.' for example.*/
	if (!project_set_base_directory(pro, NULL, tag_name_finish, attributes_finish, verbose))
	{
		buffer_release(&elements);
		return 0;
	}

	if (!property_exists(&pro->properties, project_base_dir_property, PROJECT_BASE_DIR_LENGTH))
	{
		if (!project_property_set_value(project, NULL,
										project_base_dir_property, PROJECT_BASE_DIR_LENGTH,
										base_dir, base_dir_length,
										0, 1, 1, verbose))
		{
			buffer_release(&elements);
			return 0;
		}
	}

	if (!buffer_resize(&elements, 0))
	{
		buffer_release(&elements);
		return 0;
	}

	if (0 < xml_get_sub_nodes_elements(tag_name_finish, element_finish, &elements) &&
		!project_load_properties_and_targets(pro, &elements, verbose))
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
	/**/
	uint8_t* dst = buffer_data(&content, 0);
	uint8_t* src = buffer_data(&content, addition_path_length);

	for (addition_path_length = 0; addition_path_length < path_length; ++addition_path_length)
	{
		(*dst) = (*src);
		++dst;
		++src;
	}

	if (!buffer_resize(&content, path_length) ||
		!buffer_push_back(&content, '\0'))
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

	if (!project_set_buildfile_path(project, NULL,
									buffer_data(&content, 0),
									path_length, verbose))
	{
		buffer_release(&content);
		return 0;
	}

	if (!read_file(buffer_data(&content, 0), &content))
	{
		buffer_release(&content);
		return 0;
	}

	/*TODO:if (!project_set_default_target(project, NULL, "default=", "<target>", verbose))
	{
		buffer_release(&content);
		return 0;
	}*/
	struct range directory;

	if (!path_get_directory_name(buffer_data(&content, 0),
								 buffer_data(&content, 0) + path_length, &directory))
	{
		buffer_release(&content);
		return 0;
	}

	if (!project_load_from_content(buffer_data(&content, 0) + path_length,
								   buffer_data(&content, 0) + buffer_size(&content),
								   directory.start, range_size(&directory), project, verbose))
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

	buffer_release(&pro->description);

	property_clear(&pro->properties);

	target_clear(&pro->targets);
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

uint8_t project_exec_function(const void* project, const void* target,
							  uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
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
		case get_base_directory:
			return project_get_base_directory(project, target, output);

		case get_buildfile_path:
			return project_get_buildfile_path(project, target, output);

		case get_buildfile_uri:
			return project_get_buildfile_uri(project, target, output);

		case get_default_target:
			return project_get_default_target(project, target, output);

		case get_name:
			return project_get_name(project, target, output);

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
