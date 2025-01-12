// Copyright 2020-2021 The Ray Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <ray/api/function_manager.h>
#include <ray/api/serializer.h>
#include <boost/dll.hpp>
#include <memory>
#include "absl/synchronization/mutex.h"
#include "invocation_spec.h"
#include "ray/common/id.h"
#include "ray/common/task/task_spec.h"
#include "ray/core_worker/common.h"

namespace ray {

namespace internal {
/// Execute remote functions by networking stream.
msgpack::sbuffer TaskExecutionHandler(const std::string &func_name,
                                      const std::vector<msgpack::sbuffer> &args_buffer,
                                      msgpack::sbuffer *actor_ptr);

BOOST_DLL_ALIAS(internal::TaskExecutionHandler, TaskExecutionHandler);

FunctionManager &GetFunctionManager();
BOOST_DLL_ALIAS(internal::GetFunctionManager, GetFunctionManager);

std::pair<const RemoteFunctionMap_t &, const RemoteMemberFunctionMap_t &>
GetRemoteFunctions();
BOOST_DLL_ALIAS(internal::GetRemoteFunctions, GetRemoteFunctions);
}  // namespace internal

namespace api {

class AbstractRayRuntime;

class ActorContext {
 public:
  std::shared_ptr<msgpack::sbuffer> current_actor = nullptr;

  std::shared_ptr<absl::Mutex> actor_mutex;

  ActorContext() { actor_mutex = std::shared_ptr<absl::Mutex>(new absl::Mutex); }
};

class TaskExecutor {
 public:
  TaskExecutor(AbstractRayRuntime &abstract_ray_tuntime_);

  /// TODO(Guyang Song): support multiple tasks execution
  std::unique_ptr<ObjectID> Execute(InvocationSpec &invocation);

  static void Invoke(
      const TaskSpecification &task_spec, std::shared_ptr<msgpack::sbuffer> actor,
      AbstractRayRuntime *runtime,
      std::unordered_map<ActorID, std::unique_ptr<ActorContext>> &actor_contexts,
      absl::Mutex &actor_contexts_mutex);

  static Status ExecuteTask(
      ray::TaskType task_type, const std::string task_name,
      const RayFunction &ray_function,
      const std::unordered_map<std::string, double> &required_resources,
      const std::vector<std::shared_ptr<ray::RayObject>> &args,
      const std::vector<ObjectID> &arg_reference_ids,
      const std::vector<ObjectID> &return_ids, const std::string &debugger_breakpoint,
      std::vector<std::shared_ptr<ray::RayObject>> *results,
      std::shared_ptr<ray::LocalMemoryBuffer> &creation_task_exception_pb_bytes);

  virtual ~TaskExecutor(){};

 private:
  AbstractRayRuntime &abstract_ray_tuntime_;
  static std::shared_ptr<msgpack::sbuffer> current_actor_;
};
}  // namespace api
}  // namespace ray