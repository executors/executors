# Executors TODO

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

