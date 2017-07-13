#include "job/task_graph.h"
#include "job/private/internal_types.h"

#include "core/map.h"
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
		{}

		i32 idx_ = -1;
		ITask* task_ = nullptr;

		TaskDependencies dependencies_;
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
				[this, task](i32 idx)
				{
					return taskDescs_[idx].task_ == task;
				});

			if(foundIt != dependencies.end())
			{
				return true;
			}

			// Recurse dependencies.
			bool dependencyInGraph = false; 
			for(auto depIdx : dependencies)
			{
				const auto it = std::find_if(taskDescs_.begin(), taskDescs_.end(),
					[depIdx](const TaskDesc& taskDesc)
					{
						return taskDesc.idx_ == depIdx;
					});
				DBG_ASSERT(it != taskDescs_.end());
				dependencyInGraph |= IsTaskInDependencyGraph(task, it->dependencies_);
			}
			return dependencyInGraph;
		}
	};

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
			auto foundTaskDesc = impl_->taskDescs_.end();
			if(foundTask == impl_->tasks_.end())
			{
				foundTask = impl_->tasks_.insert(task, impl_->taskDescs_.size());
				foundTaskDesc = impl_->taskDescs_.emplace_back(impl_->taskDescs_.size(), task);
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
		// 1) Topological sort.
		struct TopologicalSort
		{
			Core::Set<i32> visited_;
			Core::Vector<i32> stack_;

			void Visit(const TaskDescVec& inTaskDescs, i32 idx)
			{
				if(visited_.find(idx) == visited_.end())
					return;

				visited_.insert(idx);

				for(i32 depIdx : inTaskDescs[idx].dependencies_)
					Visit(inTaskDescs, depIdx);

				stack_.push_back(idx);
			}

			void Sort(const TaskDescVec& inTaskDescs, TaskIndices& outIndices)
			{
#error "TODO: Grab tasks free of dependencies."
				for(const auto& taskDesc : inTaskDescs)
				{
					//if(taskDesc.dependencies_.size() == 0)
						//Visit(inTaskDescs, taskDesc.idx_);
				}
			}
		} sorter;

		sorter.Sort(impl_->taskDescs_, impl_->sortedTaskIndices_);
	}

} // namespace Job
