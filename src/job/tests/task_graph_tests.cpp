#include "client/window.h"

#include "catch.hpp"

#include "core/concurrency.h"
#include "core/timer.h"
#include "core/vector.h"
#include "job/task_graph.h"

using namespace Core;

namespace
{
	volatile i32 initCounter_ = 0;
	volatile i32 workCounter_ = 0;
	volatile i32 completeCounter_ = 0;

	void ResetCounters()
	{
		initCounter_ = 0;
		workCounter_ = 0;
		completeCounter_ = 0;
	}

	class TestTask : public Job::ITask
	{
	public:
		TestTask(const char* name)
		    : name_(name)
		{
		}

		virtual ~TestTask() {}

		void OnTaskInit(Job::TaskGraph& taskGraph) override
		{
			Core::Log("TestTask::OnTaskInit \"%s\".\n", name_);

			initVal_ = Core::AtomicInc(&initCounter_);

			if(dependencies_.size() > 0)
				taskGraph.AddTasks(dependencies_.data(), dependencies_.size());
		}

		void OnTaskWork(Job::TaskGraph& taskGraph) override
		{
			Core::Log("TestTask::OnTaskWork \"%s\".\n", name_);

			workVal_ = Core::AtomicInc(&workCounter_);
		}

		void OnTasksComplete(Job::TaskGraph& taskGraph) override
		{
			Core::Log("TestTask::OnTasksComplete \"%s\".\n", name_);

			completeVal_ = Core::AtomicInc(&completeCounter_);
		}

		const char* name_ = nullptr;
		Core::Vector<Job::ITask*> dependencies_;

		i32 initVal_ = 0;
		i32 workVal_ = 0;
		i32 completeVal_ = 0;
	};
}

TEST_CASE("taskgraph-tests-basic")
{
	Job::TaskGraph taskGraph;

	// A -> B -> C
	TestTask taskA("Task A");
	TestTask taskB("Task B");
	TestTask taskC("Task C");

	taskA.dependencies_.push_back(&taskB);
	taskB.dependencies_.push_back(&taskC);

	ResetCounters();

	Job::ITask* task = &taskA;
	taskGraph.AddTasks(&task, 1);

	taskGraph.InitTasks();
	REQUIRE(taskA.initVal_ == 1);
	REQUIRE(taskB.initVal_ == 2);
	REQUIRE(taskC.initVal_ == 3);

	taskGraph.RunTasks();
	REQUIRE(taskA.workVal_ == 3);
	REQUIRE(taskB.workVal_ == 2);
	REQUIRE(taskC.workVal_ == 1);

	REQUIRE(taskA.completeVal_ == 3);
	REQUIRE(taskB.completeVal_ == 2);
	REQUIRE(taskC.completeVal_ == 1);
}

TEST_CASE("taskgraph-tests-multiple-0-a")
{
	Job::TaskGraph taskGraph;

	// A -> B -> D
	// |
	// \--> C
	TestTask taskA("Task A");
	TestTask taskB("Task B");
	TestTask taskC("Task C");
	TestTask taskD("Task D");

	taskA.dependencies_.push_back(&taskB);
	taskB.dependencies_.push_back(&taskD);
	taskA.dependencies_.push_back(&taskC);

	ResetCounters();

	Job::ITask* task = &taskA;
	taskGraph.AddTasks(&task, 1);

	taskGraph.InitTasks();
	REQUIRE(taskA.initVal_ == 1);
	REQUIRE(taskB.initVal_ == 2);
	REQUIRE(taskC.initVal_ == 3);
	REQUIRE(taskD.initVal_ == 4);

	taskGraph.RunTasks();
	if(taskC.workVal_ == 1)
	{
		REQUIRE(taskC.workVal_ == 1);
		REQUIRE(taskD.workVal_ == 2);
	}
	else
	{
		REQUIRE(taskD.workVal_ == 1);
		REQUIRE(taskC.workVal_ == 2);
	}
	REQUIRE(taskB.workVal_ == 3);
	REQUIRE(taskA.workVal_ == 4);

	if(taskC.completeVal_ == 1)
	{
		REQUIRE(taskC.completeVal_ == 1);
		REQUIRE(taskD.completeVal_ == 2);
	}
	else
	{
		REQUIRE(taskD.completeVal_ == 1);
		REQUIRE(taskC.completeVal_ == 2);
	}
	REQUIRE(taskC.completeVal_ == 1);
	REQUIRE(taskD.completeVal_ == 2);
	REQUIRE(taskB.completeVal_ == 3);
	REQUIRE(taskA.completeVal_ == 4);
}

TEST_CASE("taskgraph-tests-multiple-0-b")
{
	Job::TaskGraph taskGraph;

	// /--> C
	// |
	// A -> B -> D
	TestTask taskA("Task A");
	TestTask taskB("Task B");
	TestTask taskC("Task C");
	TestTask taskD("Task D");

	taskA.dependencies_.push_back(&taskC);
	taskA.dependencies_.push_back(&taskB);
	taskB.dependencies_.push_back(&taskD);

	ResetCounters();

	Job::ITask* task = &taskA;
	taskGraph.AddTasks(&task, 1);

	taskGraph.InitTasks();
	REQUIRE(taskA.initVal_ == 1);
	REQUIRE(taskC.initVal_ == 2);
	REQUIRE(taskB.initVal_ == 3);
	REQUIRE(taskD.initVal_ == 4);

	taskGraph.RunTasks();
	if(taskC.workVal_ == 1)
	{
		REQUIRE(taskC.workVal_ == 1);
		REQUIRE(taskD.workVal_ == 2);
	}
	else
	{
		REQUIRE(taskD.workVal_ == 1);
		REQUIRE(taskC.workVal_ == 2);
	}
	REQUIRE(taskB.workVal_ == 3);
	REQUIRE(taskA.workVal_ == 4);

	if(taskC.completeVal_ == 1)
	{
		REQUIRE(taskC.completeVal_ == 1);
		REQUIRE(taskD.completeVal_ == 2);
	}
	else
	{
		REQUIRE(taskD.completeVal_ == 1);
		REQUIRE(taskC.completeVal_ == 2);
	}
	REQUIRE(taskC.completeVal_ == 1);
	REQUIRE(taskD.completeVal_ == 2);
	REQUIRE(taskB.completeVal_ == 3);
	REQUIRE(taskA.completeVal_ == 4);
}

TEST_CASE("taskgraph-tests-multiple-1-a")
{
	Job::TaskGraph taskGraph;

	// A -> B -> D
	// |         ^
	// \--> C ---/
	TestTask taskA("Task A");
	TestTask taskB("Task B");
	TestTask taskC("Task C");
	TestTask taskD("Task D");

	taskA.dependencies_.push_back(&taskB);
	taskB.dependencies_.push_back(&taskD);
	taskA.dependencies_.push_back(&taskC);
	taskC.dependencies_.push_back(&taskD);

	ResetCounters();

	Job::ITask* task = &taskA;
	taskGraph.AddTasks(&task, 1);

	taskGraph.InitTasks();
	REQUIRE(taskA.initVal_ == 1);
	REQUIRE(taskB.initVal_ == 2);
	REQUIRE(taskC.initVal_ == 3);
	REQUIRE(taskD.initVal_ == 4);

	taskGraph.RunTasks();
	REQUIRE(taskD.workVal_ == 1);
	if(taskB.workVal_ == 2)
	{
		REQUIRE(taskB.workVal_ == 2);
		REQUIRE(taskC.workVal_ == 3);
	}
	else
	{
		REQUIRE(taskC.workVal_ == 2);
		REQUIRE(taskB.workVal_ == 3);
	}
	REQUIRE(taskA.workVal_ == 4);

	REQUIRE(taskD.completeVal_ == 1);
	if(taskB.completeVal_ == 2)
	{
		REQUIRE(taskB.completeVal_ == 2);
		REQUIRE(taskC.completeVal_ == 3);
	}
	else
	{
		REQUIRE(taskC.completeVal_ == 2);
		REQUIRE(taskB.completeVal_ == 3);
	}
	REQUIRE(taskA.completeVal_ == 4);
}

TEST_CASE("taskgraph-tests-multiple-1-b")
{
	Job::TaskGraph taskGraph;

	// /--> C ---\
	// |         v
	// A -> B -> D
	TestTask taskA("Task A");
	TestTask taskB("Task B");
	TestTask taskC("Task C");
	TestTask taskD("Task D");

	taskA.dependencies_.push_back(&taskC);
	taskA.dependencies_.push_back(&taskB);
	taskB.dependencies_.push_back(&taskD);
	taskC.dependencies_.push_back(&taskD);

	ResetCounters();

	Job::ITask* task = &taskA;
	taskGraph.AddTasks(&task, 1);

	taskGraph.InitTasks();
	REQUIRE(taskA.initVal_ == 1);
	REQUIRE(taskB.initVal_ == 3);
	REQUIRE(taskC.initVal_ == 2);
	REQUIRE(taskD.initVal_ == 4);

	taskGraph.RunTasks();
	REQUIRE(taskD.workVal_ == 1);
	if(taskB.workVal_ == 2)
	{
		REQUIRE(taskB.workVal_ == 2);
		REQUIRE(taskC.workVal_ == 3);
	}
	else
	{
		REQUIRE(taskC.workVal_ == 2);
		REQUIRE(taskB.workVal_ == 3);
	}
	REQUIRE(taskA.workVal_ == 4);

	REQUIRE(taskD.completeVal_ == 1);
	if(taskB.completeVal_ == 2)
	{
		REQUIRE(taskB.completeVal_ == 2);
		REQUIRE(taskC.completeVal_ == 3);
	}
	else
	{
		REQUIRE(taskC.completeVal_ == 2);
		REQUIRE(taskB.completeVal_ == 3);
	}
	REQUIRE(taskA.completeVal_ == 4);
}

TEST_CASE("taskgraph-tests-multiple-2-a")
{
	Job::TaskGraph taskGraph;

	// A -> B -> D
	// |    ^
	// \--> C
	TestTask taskA("Task A");
	TestTask taskB("Task B");
	TestTask taskC("Task C");
	TestTask taskD("Task D");

	taskA.dependencies_.push_back(&taskB);
	taskB.dependencies_.push_back(&taskD);
	taskA.dependencies_.push_back(&taskC);
	taskC.dependencies_.push_back(&taskB);

	ResetCounters();

	Job::ITask* task = &taskA;
	taskGraph.AddTasks(&task, 1);

	taskGraph.InitTasks();
	REQUIRE(taskA.initVal_ == 1);
	REQUIRE(taskB.initVal_ == 2);
	REQUIRE(taskC.initVal_ == 3);
	REQUIRE(taskD.initVal_ == 4);

	taskGraph.RunTasks();
	REQUIRE(taskD.workVal_ == 1);
	REQUIRE(taskB.workVal_ == 2);
	REQUIRE(taskC.workVal_ == 3);
	REQUIRE(taskA.workVal_ == 4);

	REQUIRE(taskD.completeVal_ == 1);
	REQUIRE(taskB.completeVal_ == 2);
	REQUIRE(taskC.completeVal_ == 3);
	REQUIRE(taskA.completeVal_ == 4);
}

TEST_CASE("taskgraph-tests-multiple-2-b")
{
	Job::TaskGraph taskGraph;

	// /--> C
	// |    v
	// A -> B -> D
	TestTask taskA("Task A");
	TestTask taskB("Task B");
	TestTask taskC("Task C");
	TestTask taskD("Task D");

	taskA.dependencies_.push_back(&taskC);
	taskA.dependencies_.push_back(&taskB);
	taskB.dependencies_.push_back(&taskD);
	taskC.dependencies_.push_back(&taskB);

	ResetCounters();

	Job::ITask* task = &taskA;
	taskGraph.AddTasks(&task, 1);

	taskGraph.InitTasks();
	REQUIRE(taskA.initVal_ == 1);
	REQUIRE(taskB.initVal_ == 3);
	REQUIRE(taskC.initVal_ == 2);
	REQUIRE(taskD.initVal_ == 4);

	taskGraph.RunTasks();
	REQUIRE(taskD.workVal_ == 1);
	REQUIRE(taskB.workVal_ == 2);
	REQUIRE(taskC.workVal_ == 3);
	REQUIRE(taskA.workVal_ == 4);

	REQUIRE(taskD.completeVal_ == 1);
	REQUIRE(taskB.completeVal_ == 2);
	REQUIRE(taskC.completeVal_ == 3);
	REQUIRE(taskA.completeVal_ == 4);
}
