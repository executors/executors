# Introduction

Execution is a fundamental concern of C++ programmers. Every piece of every
program executes somehow and somewhere. For example, the iterations of a `for`
loop execute in sequence on the current thread, while a parallel algorithm may
execute in parallel on a pool of threads. A C++ program's performance depends
critically on the way its execution is mapped onto underlying execution
resources. Naturally, the ability to reason about and control execution is
crucial to the needs of performance-conscious programmers.

In general, there is no standard and ubiquitous way for a C++ programmer to
control execution, but there should be. Instead, programmers control execution
through diverse and non-uniform facilities which are often coupled to low-level
platform details. This lack of common interface is an obstacle to programmers
that wish to target execution-creating facitilies because each must be targeted
idiosyncratically. For example, consider the obstacles a programmer must
overcome when targeting a simple function at one of many facilities for
creating execution:

    void parallel_for(int facility, int n, function<void(int)> f) {
      if(facility == OPENMP) {
        #pragma omp parallel for
        for(int i = 0; i < n; ++i) {
          f(i);
        }
      }
      else if(facility == GPU) {
        parallel_for_gpu_kernel<<<n>>>(f);
      }
      else if(facility == THREAD_POOL) {
        global_thread_pool_variable.submit(n, f);
      }
    }

**Complexity.** The first obstacle highlighted by this example is that each
facility's unique interface necessitates an entirely different implementation.
As the library introduces new facilities, each introduction intrudes upon
`parallel_for`'s implementation. Moreover, the problem worsens as the library
introduces new functions. While the maintainance burden of a single simple
function like `parallel_for` might be manageable, consider the maintainance
complexity of the cross product of a set of parallel algorithms with a set of
facilities.

**Synchronization.** Execution created through different facilities has
different synchronization properties. For example, an OpenMP parallel for loop
is synchronous because the spawning thread blocks until the loop is complete.
In contrast, the execution of GPU kernels is typically asynchronous; kernel
launches return immediately and the launching thread continues its execution
without waiting for the kernel's execution to complete. Work submitted to a
thread pool may or may not block the submitting thread. Correct code must
account for these synchronization differences or suffer data races. To minimize
the possibility of races, these differences should be exposed by library
interfaces.

**Non-Expressivity.** Our `parallel_for` example restricts its client to a few
simple modes of execution through the use of a single integer choosing which
facility to use. These modes are so restrictive that even simple
generalizations are out of reach. For example, suppose the programmer wants to
supply their own thread pool rather than use the global thread pool, or perhaps
the global pool augmented with some notion or priority or affinity? Similarly,
perhaps the programmer may wish to target a specific GPU or collection of
GPUs rather than some GPU implied by the surrounding environment. The
vocabulary of `parallel_for`'s interface is not rich enough to express
these subtleties.

This example illustrates the kinds of problems we propose to solve with
**executors**, which we envision as a standard way to create execution in C++.
There has already been considerable work within the C++ Standards Committee to
standardize a model of executors. Google's proposal interfaced executors to
thread pools and was briefly incorporated into a draft of the Concurrency TS
[@Austern12:N3378; @Austern13:N3562; @Mysen13:N3731; @Mysen13:N3785;
@Mysen15:N4414; @Mysen15:P0008R0]. Next, Chris Kohlhoff's proposal focused on
asynchronous processing central to the requirements of the Networking TS
[@Kohlhoff14:N4046]. Finally, NVIDIA's proposal focused on bulk execution for
the parallel algorithms of the Parallelism TS [@Hoberock15:N4406;
@Hoberock15:P0058R0; @Hoberock16:P0058R1]. A unification of this work
[@Hoberock16:P0443R0; @Hoberock17:P0443R1] specifies the most current version
[-@Hoberock17:P0443R1] of the executor model this paper describes. Our goal in
this document is to outline our vision for programming with executors in C++
and explain how we believe our design achieves this vision.

# Terminology

We envision executors as an abstraction of diverse underlying facilities responsible
for implementing execution. This abstraction will introduce a uniform interface
for creating execution which does not currently exist in C++. Before exploring
this vision, it will be useful to define some terminology for the major
concepts involved in our programming model: execution resources, execution
contexts, execution agents, and executors.

An **execution resource** is an instance of a hardware and/or software facility
capable of executing a callable function object. Different resources may offer
a broad array of functionality and semantics, and may range from SIMD vector
units accessible in a single thread to an entire runtime managing a large
collection of threads. In practice, different resources may also exhibit
different performance characteristics of interest to the performance-conscious
programmer. For example, an implementation might expose different processor
cores, with potentially non-uniform access to memory, as separate resources to
enable programmers to reason about locality.

A program may require creating execution on multiple different kinds of
execution resources, and these resources may have significantly different
capabilities. For example, callable function objects invoked on a thread of
execution have the repertoire of a Standard C++ program, including access to
the facilities of the operating system, file system, network, and similar. By
contrast, GPUs do not create standard threads of execution, and the callable
function objects they execute may have limited access to these facilities.
Moreover, functions executed by GPUs typically require special identification
by the programmer and the addresses of these functions are incompatible with
those of standard functions. Because execution resources impart different
freedoms and restrictions to the execution they create, and these differences
are visible to the programmer, we say that they are **heterogeneous**.

Our proposal does not currently specify a programming model for dealing with
heterogeneous execution resources. Instead, the execution resources
representable by our proposal are implicitly **homogeneous** and execute
Standard C++ functions. We envision that an extension of our basic executors
model will deal with heterogeneity by exposing execution resource
**architecture**. However, the introduction of a notion of concrete
architecture into C++ would be a departure from C++'s abstract machine. Because
such a departure will likely prove controversial, we think a design for
heterogeneous resources is an open question for future work.

An **execution context** is a program object that represents a specific
collection of execution resources and the **execution agents** that exist
within those resources. In our model, execution agents are units of execution,
and a 1-to-1 mapping exists between an execution agent and an 
invocation of a callable function object. An agent is bound[^bound_footnote] to an
execution context, and hence to one or more of the resources that context represents.

[^bound_footnote]: An execution agent is bound to an execution context and thus is restricted to execute only on the associated specific collection of execution resources.  For example, if a context includes multiple threads then the agent may execute on any of those threads, or migrate among those threads.

An **executor** is an object associated with a specific execution context. It
provides a mechanism for creating execution agents from a callable function
object. The primary concern of our design is to define requirements for
executors and specify their interactions with clients.

\textcolor{red}{TODO:} This section should provide a few concrete examples of each kind of thing

* If needed, expand into a more general Background section

# Using Executors

We expect that the vast majority of programmers will interact with executors
indirectly by composing them with functions that create execution on behalf of
a client.

## Using Executors with the Standard Library

Some functions, like `std::async`, will receive executors as parameters directly:

    // get an executor through some means
    my_executor_type my_executor = ...

    // launch an async using my executor
    auto future = std::async(my_executor, [] {
      std::cout << "Hello world, from a new execution agent!" << std::endl;
    });

This use of `std::async` has semantics similar to legacy uses of `std::async`,
but there are at least two important differences. First, instead of
creating a new thread of execution, this overload of `std::async` uses
`my_executor` to create an **execution agent** to execute the lambda
function. In this programming model, execution agents act as units of
execution, and every use of an executor to create execution creates one or
more execution agents. Secondly, the type of future object returned by
this overload of `std::async` depends on the type of `my_executor`. We
will discuss executor-defined future types in a later section.

Other functions will receive executors indirectly. For example, algorithms will
receive executors via execution policies:

    // get an executor through some means
    my_executor_type my_executor = ...

    // execute a parallel for_each "on" my executor
    std::for_each(std::execution::par.on(my_executor), data.begin(), data.end(), func);

In this example, the expression `par.on(my_executor)` creates a parallel
execution policy whose associated executor is `my_executor`. When `std::for_each`
creates execution it will use the executor associated with this execution policy
to create multiple execution agents to invoke `func` in parallel.

\textcolor{red}{TODO:} Demonstrate use cases central to the Networking TS

## Using Executors with Application-Level Libraries

When composing executors with functions which use them to create execution
agents, we use the following convention. When a function uses an executor to
create a single agent, the first parameter is an executor. When a function uses
an executor to create multiple agents at once, the first parameter is an
execution policy whose associated executor is the implied executor to use. The
rationale is that the requirements imposed by execution policies include
ordering among agents organized into a group executing as a unit. Logically, requirements that only
apply to agents executing as a group are nonsensical to functions which only
create a single agent.

### Executors Associated with Execution Policies

For example, the library interface for a numerical solver of systems of
linear equations might parameterize a `solve` function like this:

    template<class ExecutionPolicy>
    void solve(ExecutionPolicy policy, const matrix& A, vector& x, const vector& b) {
      // invert the matrix using the policy
      matrix A_inverse = invert(policy, A);

      // multiply b by A's inverse to solve for x
      x = multiply(A_inverse, b);
    }

By organizing `solve`'s implementation around an execution policy, it is
insulated from the details of creating execution. This frees the implementer to
apply their expertise to the application domain -- namely, numerical linear
algebra -- rather than orthogonal problems introduced by execution.
Simultaneously, this organization decouples `solve` from any particular kind
of execution. Because an execution policy is exposed in `solve`'s interface
and forwarded along through its calls to lower-level functions, `solve`'s
client is in complete control over its execution.

This is a powerful way to compose libraries. For example, a client of `solve`
may initially choose to execute `solve` sequentially:

    solve(std::execution::seq, A, x, b);

Later, the client may introduce parallelism as an optimization:

    solve(std::execution::par, A, x, b);

A further optimization might locate `solve`'s execution nearer to the memory
it accesses in order to avoid the performance hazards of non-uniform access.
Associating an executor with affinity for particular processor cores could
constrain `solve` to execute on those cores:

    executor_with_affinity exec = ...

    solve(std::execution::par.on(exec), A, s, b);

In the meantime, the efficiency of `solve` or the quality of its output may
have been improved through the use of a more sophisticated algorithm. Composing
libraries around execution policies and executors allows these two programming
activities to proceed independently.

### Executors for Coarse-Grained Tasks

A similar argument holds for application library interfaces that consume
executors directly in order to create execution agents one by one. For example,
consider an library function that executes some long-running task. To
avoid requiring clients to wait for its completion, this function immediately
returns a future object corresponding to its completion:

    template<class Executor>
    std::execution::executor_future_t<Executor,void>
    long_running_task(const Executor& exec) {
      // first, start two subtasks asynchronously
      auto future1 = subtask1(exec);
      auto future2 = subtask2(exec);

      // finally, start subtask3 when the first two are complete
      return subtask3(exec, future1, future2);
    }

\textcolor{red}{TODO:} should we use an executor category (e.g. `TwoWayExecutor`) instead of `Executor` here, or should we defer introduction of executor categories until a later section?

Consider `long_running_task`'s interface. Because the ordering requirements
imposed by an execution policy are irrelevant to `long_running_task`'s
semantics, it is parameterized by an executor instead of an execution policy.
The type of future object returned by `long_running_task` is given by the type
trait `std::execution::executor_future_t`, which names the type of future
returned when asynchronous execution is created by the type of executor used as
its template parameter. The implementation forwards along the executor
similarly to our previous example. First, the executor is passed to calls to
two independent, asynchronous subtasks. Then, the two futures corresponding to
these subtasks along with the executor are used to call the third subtask. Its
asynchronous result becomes the overall resulting future object.

## Obtaining Executors

So far, we have not addressed the issue of actually obtaining an executor to
use. We believe that there will be many different sources of executors.

**Executors from contexts.** Though our proposal makes no such requirement, we
believe many execution contexts will provide methods to create executors bound
to them. For example, our proposal defines `static_thread_pool`, which is an
execution context representing a simple, manually-sized thread pool. Clients
may receive an executor which creates work on a `static_thread_pool` by calling
its `.executor` method:  

    // create a thread pool with 4 threads
    static_thread_pool pool(4);

    // get an executor from the thread pool
    auto exec = pool.executor();

    // use the executor on some long-running task
    auto task1 = long_running_task(exec);

**Executors from policies.** Another standard source of executors will be the
standard execution policies, which will each have a similar `.executor`
method:

    // get par's associated executor
    auto par_exec = std::execution::par.executor();

    // use the executor on some long-running task
    auto task2 = long_running_task(par_exec);

**System executors.** We may also decide to provide access to implied "system"
executors used by various Standard Library functions. For example, the legacy
overload `std::async(func)` could be redefined in terms of executors in a way
that also preserves its semantics. If the implied executor used by the legacy
overload `std::async(func)` were made available, programmers porting their
existing codes to our proposed new overload `std::async(exec, func)` could
target executors in a way that preserved the original program's behavior.

**Executor adaptors.** Still other executors may be "fancy" and adapt some
other type of base executor. For example, consider a hypothetical logging
executor which prints output to a log when the base executor is used to create
execution:

    // get an executor from the thread pool
    auto exec = pool.executor();

    // wrap the thread pool's executor in a logging_executor
    logging_executor<decltype(exec)> logging_exec(exec);

    // use the logging executor in a parallel sort
    std::sort(std::execution::par.on(logging_exec), my_data.begin(), my_data.end());

We do not believe this is an exhaustive list of executor sources. Like other
adaptable, user-defined types ubiquitous to C++, sources of executors will be
diverse.

# Building Control Structures

The previous section's examples illustrate that for the vast majority of
programmers, executors will be opaque objects that merely act as abstract
representations of places where execution happens. The mechanics of direct
interaction with executors to create execution are irrelevant to this audience.
However, these mechanics are relevant to the small audience of programmers
implementing **control structures**. By control structure, we mean any function
which uses an executor, directly or indirectly, to create execution. For
example, `std::async`, the parallel algorithms library, `solve`, and
`long_running_task` are all examples of control structures because they use a
client's executor to create execution agents. In particular, our proposal adds
executor support to the following control structures from the Standard Library
and technical specifications.

| Standard Library    | Concurrency TS        | Parallelism TS                     | Networking TS |
|---------------------|-----------------------|------------------------------------|---------------|
| `invoke`            | `future::then`        | `define_task_block`                | \textcolor{red}{TODO} |
| `async`             | `shared_future::then` | `define_task_block_restore_thread` |               |
| parallel algorithms |                       | `task_block::run`                  |               |

Table: The control structures we propose to introduce.

## Fundamental Interactions with Executors via Customization Points

Some control structures (e.g., `solve`) will simply forward the executor to
other lower-level control structures (`invert` and `multiply`) to create
execution. However, at some low level of the call stack, one or more control
structures must actually interact with the executor at a fundamental level.
`std::async` is an illustrative example. Consider a possible implementation:

``` {#example:async_implementation}
template<class Executor, class Future, class... Args>
execution::executor_future_t<Executor,auto>
async(const Executor& exec, Function&& f, Args&&... args) {
  // bind together f with its arguments
  auto g = bind(forward<Function>(f), forward<Args>(args)...);

  // implement with executor customization point async_execute
  return execution::async_execute(exec, g);
}
```

In this implementation, `std::async`'s only job is to package `f` with its
arguments and forward this package along with the executor to an **executor
customization point**. Executor customization points are functions for
performing fundamental interactions with all types of
executors[^customization_point_note]. In this case, that customization point is
`execution::async_execute`, which uses the executor to create a single
execution agent to invoke a function. The agent's execution is asynchronous,
          and `execution::async_execute` returns a future corresponding to its
          completion.

[^customization_point_note]: The model for our design the one suggested by Niebler[-@Niebler15:N4381].

## Properties of Customization Points

The properties of execution created by fundamental executor interactions vary
along three dimensions we have identified as critical to an interaction's
correctness and efficiency. The combination of these properties determines the
customization point's semantics and name, which is assembled by concatenating a
prefix, infix, and suffix.

**Cardinality.** Cardinality describes how many execution agents the use of a
customization point creates, whether it be a single agent or multiple agents.
We include bulk agent creation in our design to enable executors to amortize
the cost of execution agent creation over multiple agents. By the same token,
support for single-agent creation enables executors to apply optimizations
to the important special case of a single agent. Customization points which
create multiple agents in bulk have names prefixed with `bulk_`;
single-agent customization points have no prefix. 

**Directionality.** Some executor customization points return a channel back to
the client for synchronizing with the result of execution. For asynchronous
customization points, this channel is a future object corresponding to an
eventual result. For synchronous customization points, this channel is simply
the result itself. Other customization points allow clients to
"fire-and-forget" their execution and return no such channel. We refer to
fire-and-forgetful customization points as "one-way" while those that provide a
synchronization channel are "two-way"[^directionality_caveat]. Two-way
customization points allow executors to participate directly in synchronization rather
than require inefficient synchronization out-of-band. On the other hand, when
synchronization is not required, one-way customization points avoid the cost of
a synchronization channel. The names of two-way customization points names are
infixed with `sync_`, `async_`, or `then_`.  One-way customization points have
no infix.

[^directionality_caveat]: We think that the names "one-way" and "two-way" should be improved.

**Blocking semantics.** Executor customization points may or may not block
their client's execution pending the completion of the execution they create.
Depending on the relationship between client and executed task, blocking
guarantees may be critical to either program correctness or performance. A
customization point may guarantee to always block, possibly block, or never
block its client. Customization points which may possibly block its client are
suffixed with `execute`. The exception to this rule are customization points
infixed with `sync_`. Because they always return the result of execution to
their client as a value, there is no way for `sync_` customization points to
execute without blocking their client. Customization points which will never
block its client are suffixed with `post` or `defer`. `defer` indicates a
preference to treat the created execution as a continuation of the client while
`post` indicates no such preference.

**Understanding blocking guarantees.** \textcolor{red}{Maybe find a different
  place in the paper for this lengthy explanation.} The blocking property is not
  applied uniformly. It is both a holistic property of the executor type and
  also a property of individual customization points in a few exceptional
  cases. This design is currently controversial. The reason that we chose for
  blocking to be a property of the executor type was to avoid the combinatorial
  explosion of three versions of each customization point: blocking,
  non-blocking, and possibly blocking. An alternative design could avoid
  explosively versioning customization points but would also require a way to
  introspect the blocking guarantee of individual customization points. A
  design which discarded the ability to introspect blocking guarantees is
  undesirable. It seemed simpler to the designers to understand and introspect
  a blocking guarantee as a holistic property of the executor type, rather than
  at the granularity of individual customization points.
  
There are exceptions to this rule where blocking guarantees are provided at the
granularity of individual customization points. The two-way `sync_` functions
are exceptions because it is impossible to return the result of execution in a
way that does not block the client which created that execution. The `post` and
`defer` functions are exceptions to the executor's blocking trait because we
desire the ability for a single executor to provide a blocking or
possibly-blocking `execute` as well as the unconditionally non-blocking
customization points `post` and `defer`. In such a situation, all three of
these customization points have different semantics.

\textcolor{red}{TODO:} Perhaps speculate about alternative approaches to blocking guarantees which would not suffer from the above problems

## The Customization Points Provided by Our Design

Combining these properties results in customization points with names like
`execute` and `bulk_async_execute`. One goal of the naming scheme is to allow
readers to understand, at a glance, the gross properties of the execution
created by a call to a customization point. For example, `execute` is the
customization point which creates a single execution agent and provides the
least guarantees about the agent it creates. `execute` provides no way to
synchronize with the created execution, and this execution may or may not block
the caller. On the other hand, `bulk_sync_execute` creates a group of execution
agents in bulk, and these execution agents clearly synchronize with the caller.

Combining these three sets of properties yields the following table of customization points.

| Name           | Cardinality | Directionality | Blocking |
|----------------|-------------|----------------|----------|
| `execute`      | single      | one-way        | possibly |
| `post`         | single      | one-way        | no       |
| `defer`        | single      | one-way        | no       |
| `async_execute`| single      | two-way        | possibly |
| `then_execute` | single      | two-way        | possibly |
| `sync_execute` | single      | two-way        | yes      |
| `async_post`   | single      | two-way        | no       |
| `async_defer`  | single      | two-way        | no       |
| `bulk_execute` | bulk        | one-way        | possibly |
| `bulk_post`    | bulk        | one-way        | no       |
| `bulk_defer`   | bulk        | one-way        | no       |
| `bulk_async_execute` | bulk   | two-way        | possibly |
| `bulk_then_execute`  | bulk   | two-way        | possibly |
| `bulk_sync_execute`  | bulk   | two-way        | yes      |
| `bulk_async_post`    | bulk   | two-way        | no       |
| `bulk_async_defer`   | bulk   | two-way        | no       |

Note that the entire cross product of the three sets of properties is not
represented in this table. For example, `then_post` does not exist. When
selecting customization points, we have made a trade-off between expressivity
and minimalism guided by their usefulness to the Standard Library and the
technical specifications we initially wish to target.

## Customization Point Interfaces

Customization points are customization point objects as described by the
Ranges TS [@Niebler17:RangesTS] and are provided in the `execution` namespace.

### Single-Agent Interfaces

First we describe single-agent customization points. For example, `execution::async_execute`: 

    namespace execution {

    template<class Executor, class Function>
    executor_future_t<Executor,std::invoke_result_t<std::decay_t<Function>>
    async_execute(const Executor& exec, Function&& f);

    }

The first parameter of each customization point is the executor object used to
create execution, and the second parameter is a callable object encapsulating
the task of the created execution. The executor is received by `const`
reference because executors act as shallow-`const` "views" of execution
contexts. Creating execution does not mutate the view. Single-agent
customization points receive the callable as a forwarding reference.
Single-agent, two-way customization points return the result of the callable
object either directly, or through a future as shown above. One-way
customization points always return `void`.

For `then_execute`, the third parameter is a future which is the predecessor dependency for the execution:

    template<class Executor, class Function, class Future>
    executor_future_t<Executor,std::invoke_result_t<std::decay_t<Function>,U&>>
    then_execute(const Executor& exec, Function&& f, Future& predecessor_future);

Let `U` by the type of `Future`'s result object. The callable object `f` is
invoked with a reference to the result object of the predecessor future (or
    without a parameter if a `void` future). By design, this is inconsistent
with the interface of the Concurrency TS's `future::then` which invokes its
continuation with a copy of the predecessor future. Our design avoids the
composability issues of `future::then` [@Executors16:Issue96] and is consistent
with `bulk_then_execute`, discussed below. Note that the type of `Future` is
allowed to differ from `executor_future_t<Executor,U>`, enabling
interoperability between executors and foreign future types.

Note that customization points do not receive a parameter pack of arguments for
`f`. This is a deliberate design to embue all customization point parameters
with a semantic meaning which may be exploited by the executor. Generic
parameters for `f` would have no special meaning. We expect most clients to
manipulate executors through higher-level control structures which are better
positioned to provide conveniences like variadic parameter packing. Otherwise,
a client may use `std::bind` if an appropriate control structure is
unavailable.

### Bulk Interfaces

Bulk customization points create a group of execution agents as a unit, and
each of these execution agents calls an individual invocation of the given
callable function object. The ordering guarantees of these invocations are
given by `std::execution::executor_execution_category_t`. Because they create
multiple agents, bulk customization points introduce ownership and lifetime
issues avoided by single-agent customization points and they include additional
parameters to address these issues. For example, consider
`execution::bulk_async_execute`:

    template<class Executor, class Function, class Factory1, class Factory2>
    executor_future_t<Executor,std::invoke_result_t<Factory1>>
    bulk_async_execute(const Executor& exec, Function f, executor_shape_t<Executor> shape,
                       Factory1 result_factory, Factory2 shared_parameter_factory);

**Bulk results.** The first difference is that `bulk_async_execute` returns the
result of a **factory** rather than the result of `f`. Because bulk
customization points create a group of execution agents which invoke multiple
invocations of `f`, the result of execution is ambiguous. For example, all
results of `f` could be collected into a container and returned, or a single
individual result could be selected and returned. Our design requires the
client to explicitly disambiguate the result via a factory. The
`result_factory` is simply a callable object that is invoked before the group
of execution agents begins invoking `f`, and the result of this factory is
passed as a parameter to the invocations of `f`, which may arbitrarily mutate
the result as a side effect. Any result of `f` itself is discarded.

**Pass-by-value.** Next, note that `f` is passed by value, rather than via
forwarding reference. In general, it is impossible to elect a single agent to
own `f` during execution because the group of agents may not be executing
concurrently with each other or with the client. Instead, each agent owns a
copy of `f`. One consequence of this policy is that move-only callables must be
passed by a proxy such as `std::reference_wrapper`.

**Shape.** The first new parameter is `shape`, which describes the index space
of the group of created execution agents. Each agent in the group is assigned a
unique point in this index space and the agent receives it as a parameter to
`f`. The type of this index is `executor_index_t<Executor>`. Currently, our
proposal requires `executor_shape_t` (and hence `executor_index_t`) to be an
integral type, but we envision generalizing this to support higher-dimensional
index spaces.

**Factories.** The next two parameters are factories. The first is the
`result_factory`, which we have already discussed. The second factory creates a
shared parameter for `f`. Like the result, the shared parameter is constructed
before the group of agents begins execution and it is passed as a parameter to
`f`. Unlike the result, the shared parameter is discarded. Its purpose is to
act as a temporary data structure shared by all execution agents during the
computation. Examples are `std::barrier` or atomic objects. If the client
desires to retain the shared parameter, it may be incorporated into the result
during the execution of `f`.

The result and shared parameter are passed indirectly via factories instead of
directly as objects because we believe this is the most general-purpose and
efficient scheme to pass parameters to newly-created groups of execution agents
[@Executors16:Issue9]. First, factories allow non-movable types to be
parameters, including concurrency primitives like `std::barrier` and
`std::atomic`. Next, some important types are not efficient to copy, especially
containers used as scratchpads. Finally, the location of results and shared
parameters will important to a parallel algorithm's efficiency. We envision
associating allocators with individual factories to provide
control[^factory_footnote].

The user function receives the shared state objects via bare references rather
than an alternative channel such as `std::shared_ptr` because the lifetime of these shared objects is
bound to the entire group of agents which share them. Because the sharing
relationship is structured and identified beforehand, this enables
optimizations that would be impossible for `shared_ptr`. For example, the way
`shared_ptr` allows sharers to join and leave its group of sharers in an
unstructured fashion necessitates dynamic storage and reference counting. By
contrast, the structure enforced by bulk customization permits more efficient
storage and sharing schemes.

[^factory_footnote]: This envisioned allocator support is why we refer to these callable objects as "factories" rather than simply "functions" or "callable objects".

**Bulk continuations.** Like `then_execute`, `bulk_then_execute` introduces a
predecessor future upon which the bulk continuation depends:

    template<class Executor, class Function, class Future, class Factory1, class Factory2>
    executor_future_t<Executor,std::invoke_result_t<Factory1>>
    bulk_then_execute(const Executor& exec, Function f, executor_shape_t<Executor> shape,
                      Future& predecessor_future,
                      Factory1 result_factory, Factory2 shared_factory);

If the predecessor future's result object is not `void`, a reference to the
predecessor object is passed to `f`'s invocation. Like the result and shared
parameter, we pass the predecessor object by reference because no single agent
in the group is its owner. The predecessor is collectively owned by the entire
group of agents. As a consequence, `f` must carefully synchronize access to the
predecessor object to avoid creating data races.

**Parameter order.** In any case, `f` is invoked with parameters provided in
the same order as the corresponding parameters of the customization point. The
agent index is always the first parameter, followed by the parameters emanating
from `predecessor_future`, `result_factory`, and `shared_factory`.

\textcolor{red}{TODO:} probably need to insert a discussion of allocator parameters into this

## Customization Points Adapt An Executor's Native Functionality

Our [`std::async` implementation example](#example:async_implementation)
illustrated that the control structure does not interact with the executor
directly through a member function. Our design interposes customization
points between control structures and executors to create a uniform interface
for control structures to target. Recall that we have identified a set of
possible fundamental executor interactions and we expect that this set may
grow in the future. Since it would be too burdensome for a single executor to
natively support this entire growing set of possible interactions, our design
allows executors to select a subset for native support. At the same time, for
any given executor, control structures need access to the largest possible
set of fundamental interactions. Control structures gain access to the entire
set[^adaptation_caveat] of fundamental interactions via adaptation. When an
executor natively supports the requested fundamental interaction, the
customization point simply calls that native function. When native support is
unavailable, the executor's native interactions are adapted to fulfill the
requested interaction's requirements.

[^adaptation_caveat]: In certain cases, some interactions are impossible
because their requirements are inherently incompatible with a particular
executor's provided functionality. For example, a requirement for non-blocking
execution from an executor which always executes "inline".

As a simple example, consider the adaptation performed by
`execution::sync_execute` when a given executor provides no native support:

    template<class Executor, class Function>
    std::invoke_result_t<std::decay_t<Function>>
    sync_execute(const Executor& exec, Function&& f)
    {
      auto future = execution::async_execute(exec, std::forward<Function>(f));
      return future.get();
    }

In this case, `execution::sync_execute` creates asynchronous execution via
`execution::async_execute` and receives a future object, which is immediately
waited on. Even though `exec` does not provide a native version of
`sync_execute`, its client may use it as if it does.

**Predicting adaptations.** Other customization points may apply more complex
adaptations than the simple one applied by `sync_execute`. Indeed, there may be
a variety of ways to adapt a particular executor given the native interactions
it offers. Some of these adaptations imply greater costs than others. Our
proposal specifies these adaptations in detail and provides compile-time tools
to allow clients who wish to do so the ability to predict what adaptations will
be applied, if any, to an executor when used with a specific customization
point.

**Invariant preservation.** There are limits to the kinds of adaptations that
customization points may apply, and these limits preserve executor invariants.
The rationale is that a customization point should not introduce surprising
behavior when adapting an executor's native functionality. During adaptation,
         the basic invariant-preserving rule we apply is that only the
         executor's native functionality creates execution agents. As a
         corollary, an adaptation never introduces new threads into a program
         unless the executor itself introduces those threads as an effect of
         creating execution agents. Additionally, this rule implies that a
         customization point never weakens guarantees made by an executor. For
         example, given a non-blocking executor, a customization point
         never[^invariant_caveat] blocks its client's execution.  Moreover, a
         customization point never weakens the group execution ordering
         guarantee provided by a bulk executor.

[^invariant_caveat]: Except in the case of `sync_execute`, as previously mentioned. 

# Implementing Executors

A programmer implements an executor by defining a type which satisfies the
requirements of the executor interface. The simplest possible example may be an
executor which always creates execution "inline":

    struct inline_executor {
      bool operator==(const inline_executor&) const {
        return true;
      }

      bool operator!=(const inline_executor&) const {
        return false;
      }

      const inline_executor& context() const {
        return *this;
      }

      template<class Function>
      auto sync_execute(Function&& f) const {
        return std::forward<Function>(f)();
      }
    };

First, all executor types must be `CopyConstructible`, which our
`inline_executor` implicitly satisfies. Other requirements are satisfied by
explicitly defining various member types and functions for introspection and
execution agent creation.

## Introspection

Clients introspect executors at runtime through functions and at compile time
through executor-specific type traits.

### Functions

**Executor identity.** All executors are required to be `EqualityComparable` in
order for clients to reason about their identity. Equivalence between two
executors implies that the same execution function invoked on either executors
produces the same side effects. \textcolor{red}{Is there a more precise way to
  say this?} `inline_executor` satisfies `EqualityComparable` by defining
  `operator==` and `operator!=`. Because `inline_executor::sync_execute` simply
  invokes its function inline, all instances of `inline_executor` produce the
  same side effects and are therefore equivalent.

**Execution context access.** Next, all executors are required to provide
access to their associated execution context via a member function named
`.context`. In non-generic contexts where the concrete types of executors and
their associated contexts are known in advance, clients may use contexts to
reason about underlying execution resources in order to make choices about
where to create execution agents. \textcolor{red}{The rationale for context
  access needs a better explanation, especially about how generic functions
    could make use of context introspection.} Because `inline_executor` is such
    a simple example, it serves as its own execution context and simply returns
    a reference to itself. In general, the result of `.context` must be an
    `EqualityComparable` type, and more sophisticated executors will return
    some other object: 

    class thread_pool_executor {
      private:
        mutable thread_pool& pool_;

      public:
        thread_pool_executor(thread_pool& pool) : pool_(pool) {}

        bool operator==(const thread_pool& other) const {
          return pool_ == other.pool_;
        }

        bool operator!=(const thread_pool& other) const {
          return pool_ != other.pool_;
        }

        const thread_pool& context() const {
          return pool_;
        }

        template<class Function>
        void execute(Function&& f) const {
          pool_.submit(std::forward<Function>(f));
        }
    };

In this example, an executor which creates execution agents by submitting to a
thread pool returns a reference to that thread pool from `.context`.

### Type Traits

Executor-specific type traits advertise semantics of cross-cutting guarantees
and also identify associated types. Executor type traits are provided in the
`execution` namespace and are prefixed with `executor_`. Unless otherwise
indicated, when an executor type does not proactively define a member type with
the corresponding name (sans prefix), the value of these traits have a default.
This default conveys semantics that make the fewest assumptions about the
executor's behavior.

**Execution mapping.** When executors create execution agents, they are
*mapped* onto execution resources. The properties of this mapping may be of
interest to some clients. For example, the relationship between an execution
agent and the lifetime of `thread_local` variables may be inferred by
inspecting the mapping of the agent onto its thread of execution (if any). In
our model, such mappings are represented as empty tag types and they are
introspected through the `executor_execution_mapping_category` type trait.
Currently, this trait returns one of three values:

  1. `other_execution_mapping_tag`: The executor maps agents onto non-standard execution resources.
  2. `thread_execution_mapping_tag`: The executor maps agents onto threads of execution.
  3. `unique_thread_execution_mapping_tag`: The executor maps each agent onto a new thread of execution, and that thread of execution does not change over the agent's lifetime.[^unique_thread_footnote]

[^unique_thread_footnote]: `new_thread_execution_mapping_tag` might be a better name for this. `unique_thread_execution_mapping_tag` wouldn't necessarily suggest each agent gets a newly-created thread, just its *own* thread.

The first mapping category is intended to represent mappings onto resources
which are not standard threads of execution. The abilities of such agents may
be subject to executor-defined limitations. The next two categories indicate
that agents execute on standard threads of execution as normal.
`thread_execution_mapping_tag` guarantees that agents execute on threads, but
makes no additional guarantee about the identification between agent and
thread. `unique_thread_execution_mapping_tag` does make an additional
guarantee; each agent executes on a newly-created thread. We envision that
this set of mapping categories may grow in the future.

The default value of `executor_execution_mapping_category` is `thread_execution_mapping_tag`.

**Blocking guarantee.** When a client uses an executor to create execution
agents, the execution of that client may be blocked pending the completion of
those execution agents. The `executor_execute_blocking_category` trait
describes the way in which these agents are guaranteed to block their client.

When executors create execution agents, those agents
possibly block the client's execution pending the completion of those agents.
This guarantee is given by `executor_execute_blocking_category`, which
returns one of three values:

  1. `blocking_execution_tag`: The agents' execution blocks the client.
  2. `possibly_blocking_execution_tag`: The agent's execution possibly block the client.
  3. `non_blocking_execution_tag`: The agent's execution does not block the client.

The guarantee provided by `executor_execute_blocking_category` only applies to
those customization points whose name is suffixed with `execute`. Exceptions
are the `sync_` customization points, which must always block their client by
definition. When the agents created by an executor possibly block its client,
  it's conceivable that the executor could provide dynamic runtime facilities
  for querying its actual blocking behavior. However, our design prescribes no
  interface for doing so.

The default value of `executor_execute_blocking_category` is `possibly_blocking_execution_tag`.

**Bulk forward progress guarantee.** When an executor creates a group of execution
agents, their forward progress obeys certain semantics. For example, a group of
agents may invoke the user-provided function sequentially, or they may be
invoked in parallel. Any guarantee the executor makes of these semantics is
conveyed by the `executor_execution_category` trait, which takes one one of
three values:

  1. `sequenced_execution_tag`: The invocations of the user-provided callable function object are sequenced in lexicographical order of their indices.
  2. `parallel_execution_tag`: The invocations of the user-provided callable function object are unsequenced when invoked on separate threads, and indeterminately sequenced when invoked on the same thread.
  3. `unsequenced_execution_tag`: The invocations of the user-provided callable function object are not guaranteed to be sequenced, even when those invocations are executed within the same thread.

These guarantees agree with those made by the corresponding standard execution
policies, and indeed these guarantees are intended to be used by execution
policies to describe the invocations of element access functions during
parallel algorithm execution. One difference between these guarantees and the
standard execution policies is that, unlike `std::execution::sequenced_policy`,
         `sequenced_execution_tag` does not imply that execution happens on the
         client's thread[^seq_footnote]. Instead, `executor_execution_mapping`
         captures such guarantees.

We expect this list to grow in the future. For example, guarantees of
concurrent or vectorized execution among a group of agents would be obvious
additions.

The default value of `executor_execution_category` is `unsequenced_execution_tag`.

[^seq_footnote]: We might want to introduce something like `this_thread_execution_mapping_tag` to capture the needs of `std::execution::seq`, which requires algorithms to execute on the current thread.

These describe the types of parameters involved in bulk customization points

**Executor shape type.** When an executor creates a group of execution agents
in bulk, the index space of those agents is described by a *shape*. Our current
proposal is limited to one-dimensional shapes representable by an integral
type, but we envision generalization to multiple dimensions. The type of an
executor's shape is given by `executor_shape`, and its default value is
`std::size_t`.

**Executor index type.** Execution agents within a group are uniquely
identified within their group's index space by an *index*. In addition to
sharing the dimensionality of the shape, these indices have a lexicographic
ordering. Like `executor_shape`, the type of an executor's index is given by
`executor_index`, and its default value is `std::size_t`.

**Execution context type.** `executor_context` simply names the type of an
executor's execution context by decaying the result of its member function
`.context`. This default cannot be overriden by a member type because
`.context`'s result is authoritative.

**Associated `Future` type.** `executor_future` names the type of an executor's
associated future type, which is the type of object returned by asynchronous,
           two-way customization points. The type is determined by the result
           of `execution::async_execute`, which must satisfy the requirements
           of the `Future` concept[^future_footnote]. Otherwise, the type is
           `std::future`. All of an executor's two-way asynchronous
           customization points must return the same type of future.
           \textcolor{red}{Do we allow users to specialize this type trait? P0443R1
             suggests yes, but if so, why do we allow this for the future
               type trait but not for the context type trait?}

[^future_footnote]: For now, the only type which satisfies `Future` is
`std::experimental::future`, specified by the Concurrency TS. We expect the
requirements of `Future` to be elaborated by a separate proposal.

## Execution Agent Creation via Execution Functions

Executors expose their native support for execution agent creation through
**execution functions** which are either member or free functions whose first
parameter is an executor. Member functions are preferred, but free functions
allow programmers to retrofit non-modifiable legacy types into executors. When
it exists, an executor customization point simply calls the execution function
sharing its same name without adaptation. When both a member and free function
with the same name exist, customization points prefer the member function. This
scheme is consistent with the precedent set by the Ranges TS
[@Niebler17:RangesTS] which ensures that a third party cannot "hijack" an
executor's native behavior by introducing a rogue free function.

In this section, we describe the suite of execution functions we have
identified as key to the needs of the Standard Library and TSes we have chosen
to target. Without loss of generality, we discuss the free function form of
each execution function in order of decreasing support each had within SG1 at
the 2017 Kona standardization committee meeting.

## Bulk Functions

We begin by discussing the execution functions which create groups of execution
agents in bulk, because the corresponding single-agent functions are each a
functionally special case.

### `bulk_then_execute`

    template<class Executor, class Function, class Future, class Factory1, class Factory2>
    executor_future_t<Executor, std::invoke_result_t<Factory1>>
    bulk_then_execute(const Executor& exec, Function f, executor_shape_t<Executor> shape,
                      Future& predecessor_future,
                      Factory1 result_factory, Factory2 shared_factory);

`bulk_then_execute` creates an asynchronous bulk continuation. Given some
future object referring to some predecessor task, `bulk_then_execute` creates a
group of agents whose execution is contingent on the completion of that
predecessor task. Because it is asynchronous, `bulk_then_execute` returns a
future which corresponds to the result of the bulk continuation.

`bulk_then_execute` is the most general execution function we have identified
because it may be used to implement any other execution function without having
to go out-of-band through channels not made explicit through the execution
function's interface. Explicitly elaborating this information through the
interface is critical because it enables the executor author to participate in
optimizations which would not be possible had that information been discarded
through backchannels.

For example, suppose the only available execution function was
`bulk_sync_execute`. It would be possible to implement `bulk_then_execute`'s
functionality by making a call to `bulk_sync_execute` inside a continuation
created by `predecessor_future.then`:

    predecessor_future.then([=] {
      return exec.bulk_sync_execute(exec, f, shape, result_factory, shared_factory);
    });

Note that this implementation creates `1 + shape` execution agents: one agent
created by `then` along with `shape` agents created by `bulk_sync_execute`.
Depending on the relative cost of agents created by `then` and
`bulk_sync_execute`, the overhead of introducing that extra agent may be
significant. Moreover, because the `then` operation occurs separately from
`bulk_sync_execute`, the continuation is invisible to `exec` and this precludes
`exec`'s participation in scheduling. Because we wish to allow executors to
abstract sophisticated task-scheduling runtimes, this shortcoming is
unacceptable.

### `bulk_async_execute`

    template<class Executor, class Function, class Factory1, class Factory2>
    executor_future_t<Executor, std::invoke_result_t<Factory1>>
    bulk_async_execute(const Executor& exec, Function f, executor_shape_t<Executor> shape,
                       Factory1 result_factory, Factory2 shared_factory);

`bulk_async_execute` asynchonously creates a group of execution agents whose
execution may begin immediately. Like `bulk_then_execute`, `bulk_async_execute`
returns a future corresponding to the result of the asynchronous group of
execution agents it creates. Due to these similarities, `bulk_async_execute` is
functionally equivalent to calling `bulk_then_execute` with a ready `void`
future:

    future<void> no_predecessor = make_ready_future();
    exec.bulk_then_execute(f, shape, no_predecessor, result_factory, shared_factory);

We include `bulk_async_execute` because the equivalent path through
`bulk_then_execute` via a ready future may incur overhead. The cost of the
future itself may be significant, especially if any sort of
dynamically-allocated asynchronous state is associated with that future.
Alternatively, the act of scheduling itself may be a source of overhead,
especially if it requires any sort of graph analysis performed by a dynamic
runtime. Providing executors the opportunity to specialize for cases where
it is known at compile time that no dependency exists avoids both hazards.

Moreover, common types of executor may not naturally create execution in terms
of continutions on futures as expected by `bulk_then_execute`.
`bulk_async_execute` is a better match for these cases because it does not
require accomodating a predecessor dependency.

### `bulk_async_post`

    template<class Executor, class Function, class Factory1, class Factory2>
    executor_future_t<Executor, std::invoke_result_t<Factory1>>
    bulk_async_post(const Executor& exec, Function f, executor_shape_t<Executor> shape,
                    Factory1 result_factory, Factory2 shared_factory);

`bulk_async_post` is equivalent to `bulk_async_execute` except that it makes an
additional guarantee not to block the client's execution. Some executors will
not be able to provide such a guarantee and could not be adapted to this
functionality when `bulk_async_post` is absent. For example, an implementation
of `bulk_async_post` which simply forwards its arguments directly to
`bulk_async_execute` is possible only if
`executor_execute_blocking_category_t<Executor>` is
`nonblocking_execution_tag`.

### `bulk_sync_execute`

    template<class Executor, class Function, class Factory1, class Factory2>
    std::invoke_result_t<Factory1>
    bulk_sync_execute(const Executor& exec, Function f, executor_shape_t<Executor> shape,
                      Factory1 result_factory, Factory2 shared_factory);

`bulk_sync_execute` is equivalent to `bulk_async_execute` except that it blocks
its client until the result of execution is complete. It returns this result
directly as a value, rather than indirectly via a future object.
Correspondingly, it may be implemented by calling `bulk_async_execute` and
getting the future's result:

    auto future = exec.bulk_async_execute(f, shape, result_factory, shared_factory);
    return future.get();

Like `bulk_async_execute`, we include `bulk_sync_execute` to avoid overhead
incurred by introducing a temporary future object. This overhead is likely to
be significant for executors whose cost of execution agent creation is very
small. A hypothetical `simd_executor` or `inline_executor` are examples.

## Single-Agent Functions

Single-agent execution functions are special cases of their bulk counterparts.
Because we expect single-agent creation to be an important special case, we
include these to allow for specialization.

### `then_execute`

    template<class Executor, class Function, class Future>
    executor_future_t<
      Executor,
      std::invoke_result_t<std::decay_t<Function>, decltype(std::declval<Future>().get())&>
    >
    then_execute(const Executor& exec, Function&& f, Future& predecessor_future);

`then_execute` creates an asynchronous execution agent. It may be implemented
by using `bulk_then_execute` to create a group with a single agent:

    using result_t = std::invoke_result_t<Function>;
    using predecessor_t = decltype(predecessor_future.get());

    // create a factory to return an object of the appropriate type
    // XXX instead of default construction, this really needs to return some sort
    //     of storage for an unintialized result and then the lambda below would
    //     placement new it
    auto result_factory = []{ return result_t(); };

    // pass f as a shared parameter to account for move-only functions
    auto shared_factory = [f = std::forward<Function>(f)]{ return f; };

    // create a lambda for the "group" of agents to invoke
    auto g = [](executor_index_t<Executor> ignored_index,
                predecessor_t& predecessor,
                result_t& result,
                Function& f) {
      // invoke f with the predecessor as an argument and assign the result
      result = f(predecessor);
    };

    return exec.bulk_then_execute(g,
      executor_shape_t<Executor>{1}, // create a single agent group
      predecessor_future,
      result_factory,
      shared_factory
    );

The sample implementation passes both the function to invoke and its result
indirectly via factories. The result of these factories are shared across the
group of agents created by `bulk_then_execute`. However, this group has only
one agent and no sharing actually occurs. The cost of this unnecessary sharing
may be significant and can be avoided if an executor specializes `then_execute`.

### `async_execute`

    template<class Executor, class Function>
    executor_future_t<Executor, std::invoke_result_t<std::decay_t<Function>>>
    async_execute(const Executor& exec, Function&& f);

`async_execute` creates an asynchronous execution agent. It may be implemented
by using `then_execute` with a ready `void` future:

    std::experimental::future<void> ready_future = std::experimental::make_ready_future();
    return exec.then_execute(std::forward<Function>(f), ready_future);

Alternatively, `bulk_async_execute` could be used, analogously to the use of
`bulk_then_execute` in `then_execute`'s implementation.

The cost of a superfluous immediately-ready future object could be significant
compared to the cost of agent creation. For example, the future object's
implementation could contain data structures required for inter-thread
synchronization. In this case, these data structures are wasteful and never
need to be used because the future is ready-made.

On the other hand, once a suitable `Future` concept allows for user-definable
futures, the initial future need not be `std::experimental::future`. Instead, a
hypothetical `always_ready_future` could be an efficient substitute as it would
not require addressing the problem of synchronization:

    always_ready_future<void> ready_future;
    return exec.then_execute(std::forward<Function>(f), ready_future);

However, to fully exploit such efficiency, `then_execute` may need to
recognize this case and take special action for `always_ready_future`.

Because of the opportunity for efficient specialization of a common use case, and to avoid
requiring executors to explicitly support continuations with `then_execute`, including
`async_execute` as an execution function is worthwhile.

### `async_post`

    template<class Executor, class Function>
    executor_future_t<Executor, std::invoke_result_t<std::decay_t<Function>>>
    async_post(const Executor& exec, Function&& f);

`async_post` is equivalent to `async_execute` except that it makes an
additional guarantee not to block the client's execution. Some executors will
not be able to provide such a guarantee and could not be adapted to this
functionality when `async_post` is absent. For example, an implementation of
`async_post` which simply forwards its arguments directly to `async_post` is
possible only if `executor_execute_blocking_category_t<Executor>` is
`nonblocking_execution_tag`.

### `sync_execute`

    template<class Executor, class Function>
    std::invoke_result_t<std::decay_t<Function>>
    sync_execute(const Executor& exec, Function&& f);

`sync_execute` is equivalent to `async_execute` except that it blocks its
client until the result of execution is complete. It returns this result
directly as a value, rather than indirectly via a future object.
Correspondingly, it may be implemented by calling `async_execute` and getting the future's result:

    auto future = exec.async_execute(std::forward<Function>(f));
    return future.get();

Like `async_execute`, we include `sync_execute` to avoid overhead incurred by
introducing a temporary future object. This overhead is likely to be
significant for executors whose cost of execution agent creation is very small.
A hypothetical `simd_executor` or `inline_executor` are examples.

We believe it is possible to make the cost of the future arbitrarily small in
very special cases. For example, if the executor's associated future is a
hypothetical `always_ready_future`, then a path through `async_execute` is
unlikely to incur any penalty:

    template<class T>
    class always_ready_future {
      private:
        T value_;

      public:
        T get() { return value_; }

      ...
    };

    ...

    using result_type = std::invoke_result_t<std::decay_t<Function>>;
    always_ready_future<result_type> future = exec.async_execute(std::forward<Function>(f));

However, this efficient implementation depends on `always_ready_future` being
the executor's associated future type. Moreover, it requires the executor to
treat `async_execute` as `sync_execute`'s moral equivalent by returning an
immediately ready result which always blocks the client.

Because most executors will not return ready future objects from their two-way
asynchronous execution functions, they will incur the overhead of this
`async_execute`-based implementation. Therefore, `sync_execute` is worth
including.

## One-Way Functions

### `bulk_post`
\textcolor{red}{TODO:} Show how the semantics of the corresponding customization point can be implemented by calling one of the previously defined methods

\textcolor{red}{TODO:} Provide an argument, preferably with empirical evidence, why doing so is insufficient and demonstrating how providing the more specialized method is a superior choice

### `bulk_execute`
\textcolor{red}{TODO:} Show how the semantics of the corresponding customization point can be implemented by calling one of the previously defined methods

\textcolor{red}{TODO:} Provide an argument, preferably with empirical evidence, why doing so is insufficient and demonstrating how providing the more specialized method is a superior choice

### `post`
\textcolor{red}{TODO:} Show how the semantics of the corresponding customization point can be implemented by calling one of the previously defined methods

\textcolor{red}{TODO:} Provide an argument, preferably with empirical evidence, why doing so is insufficient and demonstrating how providing the more specialized method is a superior choice

\textcolor{red}{TODO:} mention that it seems possible to implement any single-agent one-way function with `post`

### `execute`
\textcolor{red}{TODO:} Show how the semantics of the corresponding customization point can be implemented by calling one of the previously defined methods

\textcolor{red}{TODO:} Provide an argument, preferably with empirical evidence, why doing so is insufficient and demonstrating how providing the more specialized method is a superior choice

### `defer`
\textcolor{red}{TODO:} Show how the semantics of the corresponding customization point can be implemented by calling one of the previously defined methods

\textcolor{red}{TODO:} Provide an argument, preferably with empirical evidence, why doing so is insufficient and demonstrating how providing the more specialized method is a superior choice


* Describe the adaptations performed by executor customization points
  * Highlight the costs implied by specific adaptations, e.g. temporary intermediate future
    creation or dynamic memory allocation
  * Discuss how the adaptations performed by customization points are chosen and in what order
    they are preferred
  * Discuss how blocking behavior interacts with customization points

# Future Work

We conclude with a brief survey of future work extending our proposal. Some of
this work has already begun and there are others which we believe ought to be
investigated.

## `Future` Concept

Our proposal depends upon the ability of executors to create future objects
whose types differ from `std::future`. Such user-defined `std::future`-like
objects will allow interoperation with resources whose asynchronous execution
is undesirable or impossible to track through standard means. For example,
scheduling runtimes maintain internal data structures to track the
dependency relationships between different tasks. The reification of these
data structures can achieved much more efficiently than by pairing a
`std::promise` with a `std::future`. As another example, some "inline"
executors will create execution immediately in their calling thread. Because
no interthread communication is necessary, inline executors' asynchronous
results do not require expensive dynamic allocation or synchronization
primitives of full-fledged `std::future` objects. We envision that a
separate effort will propose a `Future` concept which would introduce
requirements for these user-defined `std::future`-like types.

## Error Handling

Our proposal prescribes no mechanism for execution functions to communicate
exceptional behavior back to their clients. For our purposes, exceptional
behavior includes exceptions thrown by the callable functions invoked by
execution agents and failures to create those execution agents due to resource
exhaustion. In most cases, resource exhaustion can be reported immediately
similar to `std::async`'s behavior. Reporting exceptions encountered by
execution agents can also be generalized from `std::async`. We envision that
asynchronous two-way functions will report errors through an exceptional future
object, and synchronous two-way functions will simply throw any exceptions they
encounter as normal. However, it is not clear what mechanism, if any, one-way
execution functions should use for error reporting.

## Additional Thread Pool Types

Our proposal specifies a single thread pool type, `static_thread_pool`, which
represents a simple thread pool which does not automatically resize itself. We
recognize that alternative approaches serving other use cases exist and
anticipate additional thread pool proposals. In particular, we are aware of a
separate effort which will propose an additional thread pool type,
         `dynamic_thread_pool`, and we expect this type of thread pool to be
         both dynamically and automatically resizable.

## Heterogeneity

Contemporary execution resources are heterogeneous. CPU cores, lightweight CPU
cores, SIMD units, GPU cores, operating system runtimes, embedded runtimes, and
database runtimes are examples. Heterogeneity of resources often manifests in
non-standard C++ programming models as programmer-visible versioned functions
and disjoint memory spaces. Therefore, the ability for standard executors to
target heterogeneous execution resources depends on a standard treatment of
heterogeneity in general.

The issues raised by heterogeneity impact the entire scope of a heterogeneous
C++ program, not just the space spanned by executors. Therefore, a
comprehensive solution to these issues requires a holistic approach. Moreover,
the relationship between heterogeneous execution and executors
may require technology that is out of scope of a library-only
solution such as our executors model. This technology might
include heterogeneous compilation and linking, just-in-time
compilation, reflection, serialization, and others. A separate
effort should characterize the programming problems posed by
heterogeneity and suggest solutions.

## Bulk Execution Extensions

Our current proposal's model of bulk execution is flat and one-dimensional.
Each bulk execution function creates a single group of execution agents, and
the indices of those agents are integers. We envision extending this simple
model to allow executors to organize agents into hierarchical groups and assign
then multidimensional indices. Because multidimensional indices are relevant to
many high-performance computing domains, some types of execution resources
natively generate them. Moreover, hierarchical organizations of agents
naturally model the kinds of execution created by multicore CPUs, GPUs, and
collections of these.

The organization of such a hierarchy would induce groups of groups (of groups..., etc.) of
execution agents and would introduce a different piece of shared state for each non-terminal node of this
hierarchy. The interface to such an execution function would look like:

    template<class Executor, class Function, class ResultFactory, class... SharedFactories>
    std::invoke_result_t<ResultFactory()>
    bulk_sync_execute(const Executor& exec, Function f, executor_shape_t<Executor> shape,
                      ResultFactory result_factory, SharedFactories... shared_factories);

In this interface, the `shape` parameter simultaneously describes the hierarchy
of groups created by this execution function as well as the multidimensional
shape of each of these groups. Instead of receiving a single factory to create
the shared state for a single group, the interface receives a different factory
for each level of the hierarchy. Each group's shared parameter originates from
the corresponding factory in this variadic list.

# References

