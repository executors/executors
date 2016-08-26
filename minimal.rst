===================================================================
DXXXXX : Executor
===================================================================

:Project: ISO JTC1/SC22/WG21: Programming Language C++
:Number: DXXXXX
:Date: 2016-09-XX
:Reply-to:
:Author: tbd
:Contact: tbd
:Author: tbd
:Contact: tbd
:Audience: SG1 Concurrency
:URL:

.. sectnum::

------------------------------------------------------------------------------
Conceptual Elements
------------------------------------------------------------------------------

**Instruction Stream**
  Code to be run in a form appropriate for the target execution architecture.

**Execution Architecture**
  Denotes the target architecture for an instruction stream.
  The instruction stream defined by the *main* entry point
  and associated global object initialization instruction streams
  is the *host process* execution architecture.
  Other possible target execution architectures include attached
  accelerators such as GPU, remote procedure call (RPC), and
  database management system (DBMS) servers.
  The execution architecture may impose architecture-specific constraints
  and provides architecture-specific facilities for an instruction stream.

**Execution Resource**
  An instance of an execution architecture that is capable of running
  an instruction stream targeting that architecture.
  Examples include a collection of ``std::thread`` within the host process
  that are bound to particular cores, GPU CUDA stream, an RPC server,
  a DBMS server.
  Execution resources typically have weak *locality* properties both with
  respect to one another and with respect to memory resources.
  For example, cores within a non-uniform memory access (NUMA) region
  are *more local* to each other than cores in different NUMA regions
  and hyperthreads within a single core are more local to each other than
  hyperthreads in different cores.

*Lightweight* **Execution Agent**
  An instruction stream is run by an execution agent on an execution resource.
  An execution agent may be *lightweight* in that its existance is only
  observable while the instruction stream is running.
  As such a lightweight execution agent may come into existence when
  the instruction stream starts running and cease to exist when the
  instruction stream ends.

**Execution Context**
  The mapping of execution agents to execution resources.

**Execution Function**
  The binding of an instruction stream to one or more execution agents.
  The instruction stream of a parallel algorithm may be bound to multiple
  execution agents that can run concurrently on an execution resource.
  An instruction stream's entry and return interface conforms to a
  specification defined by an execution function.
  An execution function targets a specific execution architecture.

**Executor**
  Provides execution functions for running instruction streams on
  an particular, observeable execution resource.
  A particular executor targets a particular execution architecture.


------------------------------------------------------------------------------
Extensibility
------------------------------------------------------------------------------

An essential design consideration is the extensibility of executors
and supporting conceptual elements.
When new execution architectures are developed and made available
through C++ the associated extension of executor facilities
should be minimally disruptive to code that is already *executor aware*
and compatible with constraints imposed by the execution architecture.


For extensibility *executor* cannot be a specific, standardized class.
Instead an executor is an class that conforms to a standardized concepts,
semantics, and interface patterns.
This is similar to interators where a specific class is not standardized
but instead semantic / behavioral properties are standardized with
standardized mechanisms to observe those semantics.


An instruction stream is supplied to an execution function as
an object of a type that satisfies ``std::is_callable``.
The callable interface of this object has leading arguments
defined by the execution agent that will run the object
and trailing arguments that are passed through the execution function.
Either of these argument lists may be empty.




Semantic properties can include

  * execution architecture and its associated restrictions and facilities
    for the instruction streams

  * execution resource

  * execution function

  * blocking

  * one-way / two-way

  * execution agent(s) forward progress guarantees

  * exception handling semantics

  * ability to synchronize with asynchronous execution

  * instruction stream return value if a synchronizing execution

  * execution agent property type for leading instruction stream entry arguments




------------------------------------------------------------------------------
Proposal: Executor Concept
------------------------------------------------------------------------------

  | struct executor_properties {
  |   enum : class blocking { always , never , potentially };
  |   enum : class progress { };
  |   enum : class way { one , two };
  | };

  | class *executor-concept* {
  | public:
  |   constexpr executor_properties::blocking  blocking = //
  |   constexpr executor_properties::way       way      = //
  |   constexpr executor_properties::progress  progress = //
  |   using architecture  = // implementation defined
  |   using agent_spec    = // implementation defined
  |   using agent_id      = // implementation defined
  |
  |   constexpr architecture resource() const ;
  |
  |   template< typename F , typename ... Args >
  |   using return_t =  //
  |
  |   template <typename F, typename...Args>
  |   return_t< T , Args ... >
  |   execute( agent_spec , F&& , Args ... );
  | };


An instruction stream defined by a callable object is
input to an executor's ``execute`` function.
The trailing argument pack is passed through the
``execute`` function to the trailing arguments
of the instruction stream entry.


Requires:

  | if ( is_same_v< agent_id , void > )
  |   is_callable_v< decay_t<F>( decay_t<Args>... ) >
  | else
  |   is_callable_v< decay_t<F>( agent_id , decay_t<Args>... ) >


------------------------------------------------------------------------------
Proposal: Executor for ``std::async``
------------------------------------------------------------------------------

The ``std::async`` capability implies the existence of a hidden executor
defined on the host process architecture.
Exposing this executor enables observation of executor properties
associated with ``std::async`` and explicit use of this executor;
as opposed to the implied use of this hidden executor.

  | // Execution architecture is the host process
  | // with access to all host process facilities.
  | class host_process {
  |   // traits and properties to be defined
  |   int concurrency() const ;
  | };
  |
  | class async_host_executor {
  | public:
  |   constexpr executor_properties::blocking  blocking = executor_properties::blocking::possibly ;
  |   constexpr executor_properties::way       way      = executor_properties::way::two ;
  |   constexpr executor_properties::progress  progress = executor_properties::progress::  ;
  |   using architecture  = host_process ;
  |   using agent_spec    = launch ;
  |   using agent_id      = void ;
  |
  |   constexpr architecture resource() const ;
  |
  |   template <typename F, typename...Args>
  |   using return_t = future< result_of_t< decay_t<F>(decay_t<Args>...) > > ;
  |
  |   template <typename F, typename...Args>
  |   future< result_of_t< decay_t<F>(decay_t<Args>...) > >
  |   execute( exec_policy , F&& , Args&& ... );
  | };


