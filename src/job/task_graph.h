#pragma once

#include "job/dll.h"
#include "job/types.h"

#include "core/types.h"
#include "core/vector.h"

namespace Job
{
	class TaskGraph;

	class JOB_DLL ITask
	{
	public:
		virtual ~ITask() = default;

		/**
		 * Called to initialize task prior to scheduling.
		 * Can be used to populate dependencies.
		 */
		virtual void OnTaskInit(TaskGraph& taskGraph) = 0;

		/**
		 * Called when task should do work.
		 */
		virtual void OnTaskWork(TaskGraph& taskGraph) = 0;

		/**
		 * Called when all tasks have completed.
		 */
		virtual void OnTasksComplete(TaskGraph& taskGraph) = 0;
	};


	/**
	 * Task Graph.
	 * Used for setting up a graph of dependent tasks on top of
	 * the job system.
	 */
	class JOB_DLL TaskGraph final
	{
	public:
		TaskGraph();
		~TaskGraph();

		/**
		 * Add task to graph.
		 * If a task already exists in the graph, it's not readded.
		 * @param tasks Tasks to add.
		 * @param numTask number of tasks to add.
		 * @pre @a tasks != nullptr.
		 * @pre @a numTasks > 0.
		 */
		void AddTasks(ITask** tasks, i32 numTasks);

		/**
		 * Init tasks.
		 */
		void InitTasks();

		/**
		 * Run tasks.
		 * This will run tasks immediately, and sequentially.
		 */
		void RunTasks();

		/**
		 * Schedule tasks.
		 * This will run them using jobs with appropriate synchronization.
		 */
		void ScheduleTasks();

	private:
		struct TaskGraphImpl* impl_ = nullptr;
	};


} // namespace Job
