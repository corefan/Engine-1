#include "job/task_graph.h"
#include "job/private/internal_types.h"

#include "core/map.h"
#include "core/misc.h"
#include "core/set.h"

#include <algorithm>

namespace Core
{
	u32 Hash(u32 input, Job::ITask* data) { return HashCRC32(input, &data, sizeof(data)); }
} // namespace Core

namespace Job
{
	using TaskMap = Core::Map<ITask*, i32>;
	using TaskDependencies = Core::Set<i32>;
	using TaskIndices = Core::Vector<i32>;

	struct TaskDesc
	{
		TaskDesc(i32 idx, ITask* task)
		    : idx_(idx)
		    , task_(task)
		{
		}

		i32 idx_ = -1;
		i32 group_ = 0;
		ITask* task_ = nullptr;

		TaskDependencies dependencies_;
		i32 numDependents_ = 0;
	};

	using TaskDescVec = Core::Vector<TaskDesc>;

	struct TaskGraphImpl
	{
		TaskDescVec taskDescs_;
		TaskMap tasks_;

		TaskIndices sortedTaskIndices_;

		/// Used when initializaing the tasks.
		i32 currInitTaskIdx_ = -1;

		bool IsTaskInDependencyGraph(ITask* task, const TaskDependencies& dependencies)
		{
			auto foundIt = std::find_if(dependencies.begin(), dependencies.end(),
			    [this, task](i32 idx) { return taskDescs_[idx].task_ == task; });

			if(foundIt != dependencies.end())
			{
				return true;
			}

			// Recurse dependencies.
			bool dependencyInGraph = false;
			for(auto depIdx : dependencies)
			{
				const auto it = std::find_if(taskDescs_.begin(), taskDescs_.end(),
				    [depIdx](const TaskDesc& taskDesc) { return taskDesc.idx_ == depIdx; });
				DBG_ASSERT(it != taskDescs_.end());
				dependencyInGraph |= IsTaskInDependencyGraph(task, it->dependencies_);
			}
			return dependencyInGraph;
		}
	};

	TaskGraph::TaskGraph()
	    : impl_(new TaskGraphImpl())
	{
	}

	TaskGraph::~TaskGraph() { delete impl_; }

	void TaskGraph::AddTasks(ITask** tasks, i32 numTasks)
	{
		DBG_ASSERT(impl_);
		DBG_ASSERT(tasks != nullptr);
		DBG_ASSERT(numTasks > 0);

		for(i32 idx = 0; idx < numTasks; ++idx)
		{
			auto* task = tasks[idx];
			DBG_ASSERT(task);

			auto foundTask = impl_->tasks_.find(task);
			if(foundTask == impl_->tasks_.end())
			{
				foundTask = impl_->tasks_.insert(task, impl_->taskDescs_.size());
				impl_->taskDescs_.emplace_back(impl_->taskDescs_.size(), task);
			}

			// If we're current initializing a task, then we should add all these as dependencies.
			if(impl_->currInitTaskIdx_ != -1)
			{
				auto& taskDesc = impl_->taskDescs_[impl_->currInitTaskIdx_];
				taskDesc.dependencies_.insert(foundTask->second);
			}
		}
	}

	void TaskGraph::InitTasks()
	{
		DBG_ASSERT(impl_);
		DBG_ASSERT(impl_->currInitTaskIdx_ == -1);

		// taskDescs_ vector can increase in size whilst looping through.
		for(i32 idx = 0; idx < impl_->taskDescs_.size(); ++idx)
		{
			auto taskDesc = impl_->taskDescs_[idx];

			impl_->currInitTaskIdx_ = idx;
			taskDesc.task_->OnTaskInit(*this);
		}

		impl_->currInitTaskIdx_ = -1;

#if defined(DEBUG)
		// 0) Validate dependencies.
		for(i32 idx = 0; idx < impl_->taskDescs_.size(); ++idx)
		{
			auto taskDesc = impl_->taskDescs_[idx];
			DBG_ASSERT(!impl_->IsTaskInDependencyGraph(taskDesc.task_, taskDesc.dependencies_));
		}
#endif

		// 1) Setup groups for processing.
		struct GroupingHelper
		{
			Core::Set<i32> visited_;

			i32 Visit(TaskDescVec& inTaskDescs, i32 idx, i32 leafDist)
			{
				visited_.insert(idx);

				i32 maxLeafDist = leafDist;
				for(i32 depIdx : inTaskDescs[idx].dependencies_)
				{
					maxLeafDist = Core::Max(maxLeafDist, Visit(inTaskDescs, depIdx, leafDist));
				}

				leafDist = maxLeafDist;

				auto& taskDesc = inTaskDescs[idx];
				taskDesc.group_ = Core::Max(taskDesc.group_, leafDist);
				return ++leafDist;
			}

			void Group(TaskDescVec& inTaskDescs)
			{
				for(const auto& taskDesc : inTaskDescs)
				{
					i32 leafDist = 0;
					if(visited_.find(taskDesc.idx_) == visited_.end())
						Visit(inTaskDescs, taskDesc.idx_, leafDist);
				}
			}
		};

		GroupingHelper grouper;
		grouper.Group(impl_->taskDescs_);

		// 2) Sort by group.
		impl_->sortedTaskIndices_.reserve(impl_->taskDescs_.size());
		for(i32 idx = 0; idx < impl_->taskDescs_.size(); ++idx)
			impl_->sortedTaskIndices_.emplace_back(idx);

		std::sort(impl_->sortedTaskIndices_.begin(), impl_->sortedTaskIndices_.end(),
		    [this](i32 idxA, i32 idxB) { return impl_->taskDescs_[idxA].group_ < impl_->taskDescs_[idxB].group_; });
	}

	void TaskGraph::RunTasks()
	{
		// Run tasks in their sorted order.
		for(i32 idx : impl_->sortedTaskIndices_)
		{
			const auto& taskDesc = impl_->taskDescs_[idx];
			taskDesc.task_->OnTaskWork(*this);
		}

		// Mark complete.
		for(i32 idx : impl_->sortedTaskIndices_)
		{
			const auto& taskDesc = impl_->taskDescs_[idx];
			taskDesc.task_->OnTasksComplete(*this);
		}
	}

	void TaskGraph::ScheduleTasks() { DBG_ASSERT_MSG(false, "Unimplementd.") }

} // namespace Job
