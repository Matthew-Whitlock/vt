
#if !defined INCLUDED_RUNTIME_H
#define INCLUDED_RUNTIME_H

#include "config.h"
#include "runtime_component_fwd.h"
#include "runtime_get.h"

#include <memory>
#include <mpi.h>

namespace vt { namespace runtime {

struct Runtime {
  template <typename ComponentT>
  using ComponentPtr = std::unique_ptr<ComponentT>;

  Runtime(
    int argc = 0, char** argv = nullptr,
    WorkerCountType in_num_workers = no_workers, bool const interop_mode = false,
    MPI_Comm* in_comm = nullptr
  );

  Runtime(Runtime const&) = delete;
  Runtime(Runtime&&) = delete;
  Runtime& operator=(Runtime const&) = delete;

  virtual ~Runtime();

  bool isTerminated();
  void initialize();
  void sync();
  void finalize();
  void terminationHandler();
  void setup();

protected:
  void initializeContext(int argc, char** argv, MPI_Comm* comm);
  void initializeComponents();
  void initializeOptionalComponents();
  void initializeWorkers(WorkerCountType const num_workers);

  void finalizeContext();
  void finalizeComponents();
  void finalizeOptionalComponents();

public:
  ComponentPtr<Registry> theRegistry;
  ComponentPtr<ActiveMessenger> theMsg;
  ComponentPtr<ctx::Context> theContext;
  ComponentPtr<event::AsyncEvent> theEvent;
  ComponentPtr<term::TerminationDetector> theTerm;
  ComponentPtr<barrier::Barrier> theBarrier;
  ComponentPtr<pool::Pool> thePool;
  ComponentPtr<rdma::RDMAManager> theRDMA;
  ComponentPtr<param::Param> theParam;
  ComponentPtr<seq::Sequencer> theSeq;
  ComponentPtr<seq::SequencerVirtual> theVirtualSeq;
  ComponentPtr<sched::Scheduler> theSched;
  ComponentPtr<location::LocationManager> theLocMan;
  ComponentPtr<vrt::VirtualContextManager> theVirtualManager;

  // Node-level worker-based components for vt (these are optional)
  ComponentPtr<worker::WorkerGroup> theWorkerGrp;

  // Optional components
  #if backend_check_enabled(trace_enabled)
    ComponentPtr<trace::Trace> theTrace = nullptr;
  #endif

protected:
  bool runtime_active_ = false;
  bool is_interop_ = false;
  WorkerCountType num_workers_ = no_workers;
  MPI_Comm* communicator_ = nullptr;
  int user_argc_ = 0;
  char** user_argv_ = nullptr;
};

}} /* end namespace vt::runtime */

#include "runtime.impl.h"

#endif /*INCLUDED_RUNTIME_H*/
