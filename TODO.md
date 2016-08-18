# Executors TODO

## Define minimal `Executor` requirements
  * required member functions is one of
    1. one-way function
      * XXX needs a name -- would be nice if it followed the `_execute()` naming scheme
        * ChrisM's proposal used `spawn()` -- `spawn_execute()` ?
        * could also draw from ChrisK's names
          * which one of these matches this one-way operation semantic?
            * `dispatch_execute()`
            * `post_execute()`
            * `defer_execute()`
      * needs weakest guarantees we can make
    2. two-way function 
      * name: `async_execute()`
      * needs a signature
      * needs weakest guarantees we can make
      * modulo `Future` return type, the semantic guarantees of `async_execute()` should match the one-way function
  * Must be `CopyConstructible`
    * the idea is that an `Executor` is a view of some other long-lived resource
    * making copies of `Executor`s should be cheap, like `Iterator`s
      * not sure this applies in practice
      * what about `executor_array` -- it is like a `std::vector`

## Advertising forward progress
  * optional executor member type for inter agent forward progress
    * XXX typedef needs a name
      * Agency uses `::execution_category_tag`
      * since there are different guarantees we care about, this name may not be specific enough
    * `sequenced_execution_tag`
    * `parallel_execution_tag`
    * `unsequenced_execution_tag`
    * `concurrent_execution_tag` is suggested but not required for minimal proposal
    * if no member type exists, the assumed advertisement is `unsequenced_execution_tag`
    * XXX what should happen if this member type is defined but the executor is not a `BulkExecutor`?
      * is the type not considered an `Executor`?
      * is the advertisement ignored?
        * could be useful for debugging
  * optional executor member type for caller/agents forward progress
    * typedef needs a name
    * "may block" type needs a name
    * "does block" type needs a name
    * "does not block" type needs a name
    * if no member type exists, the assumed advertisement is "may block"
  * do we need traits for introspecting an executor's two advertised guarantees?
    * i.e. something like `executor_bikeshed_trait_name_here_t<Executor>`

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

