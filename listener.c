/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 - 2021 TheVice
 *
 */

#include "listener.h"
#include "interpreter.h"
#include "project.h"
#include "range.h"
#include "shared_object.h"
#include "target.h"

static void (*on_project_started)(const uint8_t* source, const uint8_t* the_project) = NULL;
static void (*on_project_finished)(const uint8_t* source, const uint8_t* the_project, uint8_t result) = NULL;

static void (*on_target_started)(const uint8_t* source, ptrdiff_t offset,
								 const uint8_t* the_project, const uint8_t* the_target) = NULL;
static void (*on_target_finished)(const uint8_t* source, ptrdiff_t offset,
								  const uint8_t* the_project, const uint8_t* the_target, uint8_t result) = NULL;

static void (*on_task_started)(const uint8_t* source, ptrdiff_t offset,
							   const uint8_t* the_project, const uint8_t* the_target, const uint8_t* task_name) = NULL;
static void (*on_task_finished)(const uint8_t* source, ptrdiff_t offset,
								const uint8_t* the_project, const uint8_t* the_target, const uint8_t* task_name, uint8_t result) = NULL;

void listener_get_data(const uint8_t* the_project, const uint8_t* the_target,
					   const struct range* task_name, ptrdiff_t task_id, const uint8_t* the_module,
					   ptrdiff_t* output_offset, const uint8_t** output_project_name,
					   const uint8_t** output_target_name, const uint8_t** output_task_name, uint8_t verbose)
{
	if (NULL != output_offset && NULL != task_name && interpreter_get_unknown_task_id() != task_id)
	{
		*output_offset = project_get_source_offset(the_project, task_name->finish);
	}

	if (NULL != output_project_name && project_set_listener_project_name(the_project, verbose))
	{
		*output_project_name = project_get_listener_project_name(the_project);
	}

	if (NULL != output_target_name && !target_get_current_target(the_target, output_target_name, NULL))
	{
		*output_target_name = NULL;
	}

	if (NULL != output_task_name && project_set_listener_task(the_project, task_name, task_id, the_module))
	{
		*output_task_name = project_get_listener_task_name(the_project);
	}
}

void listener_project_started(const uint8_t* source, const uint8_t* the_project, uint8_t verbose)
{
	if (NULL != on_project_started)
	{
		const uint8_t* project_name = NULL;
		listener_get_data(the_project, NULL, NULL, 0, NULL, NULL, &project_name, NULL, NULL, verbose);
		/**/
		on_project_started(source, project_name);
	}
}

void listener_project_finished(const uint8_t* source, const uint8_t* the_project, uint8_t result,
							   uint8_t verbose)
{
	if (NULL != on_project_finished)
	{
		const uint8_t* project_name = NULL;
		listener_get_data(the_project, NULL, NULL, 0, NULL, NULL, &project_name, NULL, NULL, verbose);
		/**/
		on_project_finished(source, project_name, result);
	}
}

void listener_target_started(const uint8_t* source, ptrdiff_t offset,
							 const uint8_t* the_project, const uint8_t* the_target, uint8_t verbose)
{
	if (NULL != on_target_started)
	{
		const uint8_t* project_name = NULL;
		const uint8_t* target_name = NULL;
		listener_get_data(the_project, the_target, NULL, 0, NULL, NULL, &project_name, &target_name, NULL, verbose);
		/**/
		on_target_started(source, offset, project_name, target_name);
	}
}

void listener_target_finished(const uint8_t* source, ptrdiff_t offset,
							  const uint8_t* the_project, const uint8_t* the_target, uint8_t result, uint8_t verbose)
{
	if (NULL != on_target_finished)
	{
		const uint8_t* project_name = NULL;
		const uint8_t* target_name = NULL;
		listener_get_data(the_project, the_target, NULL, 0, NULL, NULL, &project_name, &target_name, NULL, verbose);
		/**/
		on_target_finished(source, offset, project_name, target_name, result);
	}
}

void listener_task_started(const uint8_t* source, ptrdiff_t offset,
						   const uint8_t* the_project, const uint8_t* the_target,
						   const struct range* task_name, ptrdiff_t task_id,
						   const uint8_t* the_module, uint8_t verbose)
{
	if (NULL != on_task_started)
	{
		const uint8_t* project_name = NULL;
		const uint8_t* target_name = NULL;
		const uint8_t* output_task_name = NULL;
		listener_get_data(
			the_project, the_target, task_name, task_id, the_module, &offset,
			&project_name, &target_name, &output_task_name, verbose);
		/**/
		on_task_started(source, offset, project_name, target_name, output_task_name);
	}
}

void listener_task_finished(const uint8_t* source, ptrdiff_t offset,
							const uint8_t* the_project, const uint8_t* the_target,
							const struct range* task_name, ptrdiff_t task_id,
							const uint8_t* the_module, uint8_t result, uint8_t verbose)
{
	if (NULL != on_task_finished)
	{
		const uint8_t* project_name = NULL;
		const uint8_t* target_name = NULL;
		const uint8_t* output_task_name = NULL;
		listener_get_data(
			the_project, the_target, task_name, task_id, the_module, &offset,
			&project_name, &target_name, &output_task_name, verbose);
		/**/
		on_task_finished(source, offset, project_name, target_name, output_task_name, result);
	}
}

void listener_set_on_project_started(
	void (*listener_on_project_started)(const uint8_t* source, const uint8_t* the_project))
{
	on_project_started = listener_on_project_started;
}

void listener_set_on_project_finished(
	void (*listener_on_project_finished)(const uint8_t* source, const uint8_t* the_project, uint8_t result))
{
	on_project_finished = listener_on_project_finished;
}

void listener_set_on_target_started(
	void (*listener_on_target_started)(const uint8_t* source, ptrdiff_t offset,
									   const uint8_t* the_project, const uint8_t* the_target))
{
	on_target_started = listener_on_target_started;
}

void listener_set_on_target_finished(
	void (*listener_on_target_finished)(const uint8_t* source, ptrdiff_t offset,
										const uint8_t* the_project, const uint8_t* the_target, uint8_t result))
{
	on_target_finished = listener_on_target_finished;
}

void listener_set_on_task_started(
	void (*listener_on_task_started)(const uint8_t* source, ptrdiff_t offset,
									 const uint8_t* the_project, const uint8_t* the_target, const uint8_t* the_task))
{
	on_task_started = listener_on_task_started;
}

void listener_set_on_task_finished(
	void (*listener_on_task_finished)(const uint8_t* source, ptrdiff_t offset,
									  const uint8_t* the_project, const uint8_t* the_target,
									  const uint8_t* the_task, uint8_t result))
{
	on_task_finished = listener_on_task_finished;
}

uint8_t load_listener(const uint8_t* listener, void** object)
{
	if (!listener || !object)
	{
		return 0;
	}

	*object = shared_object_load(listener);

	if (NULL == (*object))
	{
		return 0;
	}

	static const uint8_t* procedures_names[] =
	{
		(const uint8_t*)"listener_project_started",
		(const uint8_t*)"listener_project_finished",
		(const uint8_t*)"listener_target_started",
		(const uint8_t*)"listener_target_finished",
		(const uint8_t*)"listener_task_started",
		(const uint8_t*)"listener_task_finished"
	};

	for (uint8_t i = 0, count = 6; i < count; ++i)
	{
		void* address = shared_object_get_procedure_address(*object, procedures_names[i]);

		if (!address)
		{
			continue;
		}

		switch (i)
		{
#if defined(_MSC_VER) && (_MSC_VER < 1910)
#pragma warning(disable: 4055)
#endif

			case 0:
				listener_set_on_project_started(
					(void (*)(const uint8_t* source, const uint8_t* the_project))address);
				break;

			case 1:
				listener_set_on_project_finished(
					(void (*)(const uint8_t* source, const uint8_t* the_project, uint8_t result))address);
				break;

			case 2:
				listener_set_on_target_started(
					(void (*)(const uint8_t* source, ptrdiff_t offset,
							  const uint8_t* the_project, const uint8_t* the_target))address);
				break;

			case 3:
				listener_set_on_target_finished(
					(void (*)(const uint8_t* source, ptrdiff_t offset,
							  const uint8_t* the_project, const uint8_t* the_target, uint8_t result))address);
				break;

			case 4:
				listener_set_on_task_started(
					(void (*)(const uint8_t* source, ptrdiff_t offset,
							  const uint8_t* the_project, const uint8_t* the_target, const uint8_t* the_task))address);
				break;

			case 5:
				listener_set_on_task_finished(
					(void (*)(const uint8_t* source, ptrdiff_t offset,
							  const uint8_t* the_project, const uint8_t* the_target,
							  const uint8_t* the_task, uint8_t result))address);
				break;
#if defined(_MSC_VER) && (_MSC_VER < 1910)
#pragma warning(default: 4055)
#endif

			default:
				break;
		}
	}

	return 1;
}
