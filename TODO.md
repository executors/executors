# Executors TODO

## Define `ChrisKExecutor` requirements
  * leave a placeholder for ChrisK TODO item

## Define minimal ParallelExecutor/BulkExecutor requirements
  * XXX Choose a name
    * Jared prefers `BulkExecutor` because the required operations are prefixed with `bulk_`
  * required member functions
    * single-agent operation
      * `async_execute()`
    * multi-agent operations
      * naming scheme
        * Think it's important to distinguish the name of the multi-agent operations from the single-agent operations instead of introducing an overload
        * bulk-synchronous / bulk-asynchronous is a term of art
        * Jared prefers `bulk_` prefix
      * `bulk_async_execute()`
      * `bulk_execute()`
      * `bulk_then_execute()`
  * optional member types
    * `shape_type` -- type must be an integral type
      * if not given, assumed to be `size_t`
      * we need this to know what sort of type to use as a parameter to the `bulk_` operations
      * the reason it is named `shape_type` instead of something like `size_type` is to allow relaxing the requirements on integral types to more general "shape" types in the future
    * `index_type` -- type must be constructible from `shape_type`
      * need this type to know what sort of type the user function should receive as a parameter
  * the caller/agents forward progress guarantee applies uniformly to all three asynchronous operations
    * `async_execute()`
    * `bulk_async_execute()`
    * `bulk_then_execute()`
    * it's much simpler than defining different guarantees for each operation
      * if a programmer wants different guarantees, they should define a different executor type
      * if in practice the different operations behave differently, then the executor can make the weakest guarantee

## Define free function customization points
  * the signatures of these free functions are identical to the member functions, except the first parameter takes a mutable reference to an executor object
    * requirements for the type of executor is just `Executor`
  * customization points
    * `execution::bikeshed_execute()`
      * one-way, single-agent execution
    * `execution::async_execute()`
      * two-way, single-agent execution
    * `execution::bulk_execute()`
      * two-way, bulk synchronous execution
    * `execution::bulk_async_execute()`
      * two-way, bulk asynchronous execution
    * `execution::bulk_then_execute()`
      * two-way, bulk asynchronous continuation
  * figure out what rules these customization points follow when adapting an `Executor` which does not provide native functionality 
    * we want to avoid spurious introduction of threads
    * could defer to the executor rules we defined in [the NVIDIA proposal](wg21.link/p0058)
  * placeholder for customization points ChrisK's executor need

## ExecutionPolicy/Executor interoperation
  * Member type of policy `::executor_type` names the type of the executor
  * this type must be `ParallelExecutor`/`BulkExecutor`
  * `.on()`
    * `.on()` takes `Executor`
    * adapts the given `Executor` into a `ParallelExecutor`/`BulkExecutor`
      * `Executor`s which already satisfy bulk parallelism requirements pass through unadapted
    * return an implementation-defined execution policy identical to the original, except the associated executor is different
      * when the associated executor has been adapted, its type is implementation-defined
      * when the associated executor does not require adaptation, its type is what was provided to `.on()`
    * we can imagine enhancing `.on()` in the future to receive other types of objects which may not be `Executor`s
      * the idea is that `.on()` just performs an adaptation and returns a new policy -- it's a factory for execution policies

