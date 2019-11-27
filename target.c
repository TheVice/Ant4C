/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "target.h"
#include "buffer.h"
#include "common.h"
#include "property.h"
#include "range.h"
#include "string_unit.h"
#include "xml.h"

#include <string.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

static const uint8_t depends_delimiter = ',';

struct target
{
	uint8_t name[UINT8_MAX + 1];
	uint8_t name_length;
	/**/
	struct buffer description;
	struct buffer depends;
	struct buffer content;
	/**/
	uint8_t has_executed;
};

struct depend
{
	const struct target* target;
	uint8_t name[UINT8_MAX + 1];
	uint8_t name_length;
};

uint8_t buffer_append_target(struct buffer* targets, const struct target* data, ptrdiff_t data_count)
{
	return buffer_append(targets, (const uint8_t*)data, sizeof(struct target) * data_count);
}

struct target* buffer_target_data(const struct buffer* targets, ptrdiff_t data_position)
{
	return (struct target*)buffer_data(targets, sizeof(struct target) * data_position);
}

uint8_t target_exists(const struct buffer* targets,
					  const uint8_t* name, uint8_t name_length)
{
	void* trg = NULL;
	return target_get(targets, name, name_length, &trg);
}

uint8_t target_get_current_target(const void* target, const uint8_t** name, ptrdiff_t* name_length)
{
	if (NULL == target)
	{
		return 0;
	}

	const struct target* trg = (const struct target*)target;
	*name = trg->name;
	*name_length = trg->name_length;
	return 1;
}

uint8_t target_has_executed(const struct buffer* targets,
							const uint8_t* name, uint8_t name_length)
{
	void* trg = NULL;

	if (!target_get(targets, name, name_length, &trg))
	{
		return 0;
	}

	return 0 < ((struct target*)trg)->has_executed;
}

uint8_t target_get(const struct buffer* targets, const uint8_t* name,
				   uint8_t name_length, void** target)
{
	if (NULL == targets ||  NULL == name || 0 == name_length || NULL == target)
	{
		return 0;
	}

	ptrdiff_t i = 0;
	struct target* target_ = NULL;

	while (NULL != (target_ = buffer_target_data(targets, i++)))
	{
		if (name_length == target_->name_length &&
			0 == memcmp(&target_->name, name, name_length))
		{
			(*target) = target_;
			return 1;
		}
	}

	return 0;
}

uint8_t target_add_depend(const struct range* depend_name, struct buffer* depends)
{
	if (range_is_null_or_empty(depend_name))
	{
		return 0;
	}

	struct depend depend_;

	depend_.target = NULL;

	depend_.name_length = (uint8_t)range_size(depend_name);

#if __STDC_SEC_API__
	if (0 != memcpy_s(depend_.name, UINT8_MAX, depend_name->start, depend_.name_length))
	{
		return 0;
	}

#else
	memcpy(depend_.name, depend_name->start, depend_.name_length);
#endif
	return buffer_append(depends, (const uint8_t*)&depend_, sizeof(struct depend));
}

uint8_t target_add(struct buffer* targets,
				   const struct range* name, const struct range* description,
				   const struct range* depends, const struct range* content)
{
	if (NULL == targets || range_is_null_or_empty(name) || range_is_null_or_empty(content))
	{
		return 0;
	}

	struct target new_target;

	SET_NULL_TO_BUFFER(new_target.description);

	SET_NULL_TO_BUFFER(new_target.depends);

	SET_NULL_TO_BUFFER(new_target.content);

	new_target.name_length = (uint8_t)range_size(name);

	if (target_exists(targets, name->start, new_target.name_length))
	{
		return 0;
	}

#if __STDC_SEC_API__

	if (0 != memcpy_s(new_target.name, UINT8_MAX, name->start, new_target.name_length))
	{
		return 0;
	}

#else
	memcpy(new_target.name, name->start, new_target.name_length);
#endif
	new_target.name[new_target.name_length] = '\0';

	if (!range_is_null_or_empty(description))
	{
		if (!buffer_append(&new_target.description, description->start,
						   description->finish - description->start))
		{
			return 0;
		}
	}

	if (!range_is_null_or_empty(depends))
	{
		struct range depend_;
		depend_.start = depends->start;
		depend_.finish = depends->finish;

		while (depends->finish != (depend_.finish = find_any_symbol_like_or_not_like_that(depend_.finish,
								   depends->finish, &depends_delimiter, 1, 1, 1)))
		{
			const uint8_t* pos = depend_.finish + 1;

			if (!string_trim(&depend_) || !target_add_depend(&depend_, &new_target.depends))
			{
				return 0;
			}

			depend_.start = pos;
			depend_.finish = pos;
		}

		if (!string_trim(&depend_) || !target_add_depend(&depend_, &new_target.depends))
		{
			return 0;
		}
	}

	if (!buffer_append(&new_target.content, content->start, content->finish - content->start))
	{
		return 0;
	}

	new_target.has_executed = 0;
	return buffer_append_target(targets, &new_target, 1);
}

uint8_t target_add_from_xml_tag_record(struct buffer* targets,
									   const uint8_t* record_start, const uint8_t* record_finish)
{
	(void)targets;
	(void)record_start;
	(void)record_finish;
#if 0
	struct range name;
	struct range description;
	struct range depends;
	struct range content;
	/**/
	const uint8_t* target_attributes[] = { (const uint8_t*)"name", (const uint8_t*)"description", (const uint8_t*)"depends" };
	const uint8_t target_attributes_lengths[] = { 4, 11, 7 };
	/**/
	struct range* attribute_values[3];
	attribute_values[0] = &name;
	attribute_values[1] = &description;
	attribute_values[2] = &depends;

	for (uint8_t i = 0; i < 3; ++i)
	{
		if (!xml_get_attribute_value(record_start, record_finish, target_attributes[i],
									 target_attributes_lengths[i], attribute_values[i]))
		{
			if (!i)
			{
				return 0;
			}

			attribute_values[i]->start = attribute_values[i]->finish = NULL;
			continue;
		}
	}

	content.start = 1 + xml_get_tag_finish_pos(record_start, record_finish);
	content.finish = record_finish;
	return target_add(targets, &name, &description, &depends, &content);
#endif
	return 0;
}

void target_clear(struct buffer* targets)
{
	ptrdiff_t i = 0;
	struct target* target = NULL;

	if (NULL == targets)
	{
		return;
	}

	while (NULL != (target = buffer_target_data(targets, i++)))
	{
		buffer_release(&target->description);
		buffer_release(&target->depends);
		buffer_release(&target->content);
	}

	buffer_release(targets);
}
