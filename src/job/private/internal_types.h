#pragma once

#include "job/dll.h"
#include "core/types.h"

namespace Job
{
	/**
	 * Counter internal details.
	 */
	struct Counter final
	{
		/// Counter value. Decreases as job completes.
		volatile i32 value_ = 0;
		/// Should counter be freed by the last that's using it?
		bool free_ = false;

		Counter() = default;
		Counter(const Counter&) = delete;
	};
}
